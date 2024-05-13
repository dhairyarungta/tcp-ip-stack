#include <stdlib.h>
#include <stdio.h>
#include "../net.h"
#include "../comm.h"
#include "../graph.h"
#include "layer2.h"

typedef struct mac_table_entry_ {
    mac_add_t mac; /*KEY*/
    char oif_name[IF_NAME_SIZE];
    glthread_t mac_entry_glue;
} mac_table_entry_t;

typedef struct mac_table_{
    glthread_t mac_entries;
}mac_table_t;

GLTHREAD_TO_STRUCT(mac_entry_glue_to_mac_entry ,mac_table_entry_t, mac_entry_glue);

#define IS_MAC_TABLE_ENTRY_EQUAL(mac_table_entry_1, mac_table_entry_2)\
    (strncmp(mac_table_entry_1->mac.mac, mac_table_entry_2->mac.mac, 6)==0 && \
    strncmp(mac_table_entry_1->oif_name, mac_table_entry_2->oif_name, IF_NAME_SIZE)==0)


mac_table_entry_t *
mac_table_lookup(mac_table_t *mac_table, char *mac){

    glthread_t *glthreadptr = NULL;
    mac_table_entry_t *mac_table_entry = NULL;
    ITERATE_GLTHREAD_BEGIN(&mac_table->mac_entries, glthreadptr){
        mac_table_entry = mac_entry_glue_to_mac_entry(glthreadptr);
        if(strncmp(mac, mac_table_entry->mac.mac,sizeof(mac_add_t))==0){
            return mac_table_entry;
        }
    }
    ITERATE_GLTHREAD_END(&mac_table->mac_entries, glthreadptr);
    return NULL;
}

void
delete_mac_table_entry (mac_table_t *mac_table, char *mac){

    mac_table_entry_t *mac_table_entry = mac_table_lookup(mac_table, mac);
    if(!mac_table_entry){
        return ;
    }
    remove_glthread(&mac_table_entry->mac_entry_glue);
    free(mac_table_entry);
}

void
init_mac_table(mac_table_t **mac_table){

    *mac_table = (mac_table_t *) calloc(1, sizeof(mac_table_t));
    init_glthread(&((*mac_table)->mac_entries));
}

bool_t 
mac_table_entry_add(mac_table_t *mac_table, mac_table_entry_t *mac_table_entry){

    mac_table_entry_t *mac_table_entry_old = mac_table_lookup(mac_table, mac_table_entry->mac.mac);
    if(mac_table_entry_old && IS_MAC_TABLE_ENTRY_EQUAL(mac_table_entry_old, mac_table_entry)){
        return FALSE;
    }

    if(mac_table_entry_old){
        delete_mac_table_entry(mac_table, mac_table_entry_old->mac.mac);
    }
    
    init_glthread(&mac_table_entry->mac_entry_glue);
    glthread_add_next(&mac_table->mac_entries, &mac_table_entry->mac_entry_glue);
    return TRUE;
}

static void
l2_switch_perform_mac_learning(node_t *node, char *src_mac, char *intf_name){

    mac_table_t *mac_table = node->node_nw_prop.mac_table;
    mac_table_entry_t *mac_table_entry_new = (mac_table_entry_t *) calloc(1, sizeof(mac_table_entry_t));
    strncpy(mac_table_entry_new->oif_name, intf_name, IF_NAME_SIZE);
    mac_table_entry_new->oif_name[IF_NAME_SIZE-1] = '\0';
    strncpy(mac_table_entry_new->mac.mac, src_mac, sizeof(mac_add_t));

    if(mac_table_entry_add(mac_table, mac_table_entry_new) == FALSE){
        free(mac_table_entry_new);
    }
}

static bool_t
l2_switch_send_pkt_out(char *pkt , unsigned int pkt_size,
    interface_t *oif){
    
    if(IS_INTF_L3_MODE(oif)){
        assert(0);
    }

    intf_l2_mode_t intf_l2_mode = IF_L2_MODE(oif);
    if(intf_l2_mode == L2_MODE_UNKNOWN){
        return FALSE;
    }

    ethernet_hdr_t *ethernet_hdr = (ethernet_hdr_t *)pkt;
    vlan_8021q_hdr_t *vlan_8021q_hdr = is_pkt_vlan_tagged(ethernet_hdr);

    switch(intf_l2_mode){
        case ACCESS:
        {
            unsigned int intf_vland_id = get_access_intf_operating_vlan_id(oif);
            if(vlan_8021q_hdr==NULL && intf_vland_id==0){
                assert(0); /*A Non Configured Access VLAN shouldn't be allowed to send 
                            to any MAC of its choice*/
            }

            if(vlan_8021q_hdr==NULL && intf_vland_id!=0){
                return FALSE;
            }

            if(vlan_8021q_hdr!=NULL && intf_vland_id!=0){
                if(vlan_8021q_hdr->tci_vid==intf_vland_id){
                    send_pkt_out(pkt, pkt_size, oif);
                    return TRUE;
                }
                else {
                    return FALSE;
                }
            }

            if(vlan_8021q_hdr!=NULL && intf_vland_id==0){
                return FALSE;
            }

            return FALSE;
        }

        case TRUNK:
        {
            unsigned pkt_vlan_id = 0;
            if(vlan_8021q_hdr!=NULL){
                pkt_vlan_id = GET_802_1Q_VLAN_ID(vlan_8021q_hdr);
            }

            if(pkt_vlan_id && is_trunk_interface_vlan_enabled(oif, pkt_vlan_id)){
                send_pkt_out(pkt, pkt_size, oif);
                return TRUE;
            }

            return FALSE;
        }

        default:
            return FALSE;
    }
}

static bool_t
l2_switch_flood_pkt_out(node_t * node, interface_t *expempted_intf,
    char *pkt, unsigned int pkt_size){

    interface_t *oif = NULL;

    char *pkt_copy = NULL;
    char *temp_pkt = (char *) calloc(1, MAX_PACKET_BUFFER_SIZE);
    pkt_copy = temp_pkt+MAX_PACKET_BUFFER_SIZE-pkt_size;
    
    for(unsigned int i = 0; i < MAX_INTF_PER_NODE; i++){
        oif = node->intf[i];
        if(!oif){
            break;
        }

        if(IS_INTF_L3_MODE(oif) || oif==expempted_intf){
            continue;
        }

        memcpy(pkt_copy, pkt, pkt_size);
        l2_switch_send_pkt_out(pkt_copy, pkt_size, oif);
    }
    free(temp_pkt);
    return TRUE;
}

static void
l2_switch_forward_frame(node_t *node, interface_t *recv_intf, char *pkt, unsigned int pkt_size){

    ethernet_hdr_t *ethernet_hdr = (ethernet_hdr_t *)pkt;
    if(IS_MAC_BROADCAST_ADDR(ethernet_hdr->dst_mac.mac)){
        l2_switch_flood_pkt_out(node, recv_intf, (char *)ethernet_hdr, pkt_size);
        return;
    }

    mac_table_entry_t *mac_table_entry = mac_table_lookup(NODE_MAC_TABLE(node), ethernet_hdr->dst_mac.mac);
    if(!mac_table_entry){
        l2_switch_flood_pkt_out(node, recv_intf, (char *)ethernet_hdr, pkt_size);
        return;
    }

    char *oif_name = mac_table_entry->oif_name;
    interface_t *oif = get_node_if_by_name(node, oif_name); 

    if(!oif){
        return;
    }

    l2_switch_send_pkt_out((char *)ethernet_hdr, pkt_size, oif);
}

void
l2_switch_recv_frame(interface_t *interface, 
    char *pkt, unsigned int pkt_size){
    
    node_t *node = interface->att_node;
    unsigned char *dst_mac = ((ethernet_hdr_t *)pkt)->dst_mac.mac;
    unsigned char *src_mac = ((ethernet_hdr_t *)pkt)->src_mac.mac;
    
   l2_switch_perform_mac_learning(node, src_mac, interface->if_name);
   l2_switch_forward_frame(node, interface, pkt, pkt_size);
}

void
dump_mac_table(mac_table_t *mac_table){

    printf("MAC Table :\n\t MAC Address, Outgoing Intf Name\n");
    
    mac_table_entry_t *mac_table_entry = NULL;
    glthread_t *glthreadptr = NULL;

    ITERATE_GLTHREAD_BEGIN(&mac_table->mac_entries, glthreadptr){
        mac_table_entry = mac_entry_glue_to_mac_entry(glthreadptr);
        printf("\t %u:%u:%u:%u:%u:%u, %s\n", 
                mac_table_entry->mac.mac[0],
                mac_table_entry->mac.mac[1],
                mac_table_entry->mac.mac[2],
                mac_table_entry->mac.mac[3],
                mac_table_entry->mac.mac[4],
                mac_table_entry->mac.mac[5],
                mac_table_entry->oif_name);
    }ITERATE_GLTHREAD_END(&mac_table->mac_entries, glthreadptr);
}