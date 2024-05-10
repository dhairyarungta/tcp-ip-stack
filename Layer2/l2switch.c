#include <stdlib.h>
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
    (strncmp()==0 && strncmp()==0)


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

bool_t 
mac_table_entry_add(mac_table_t *mac_table, mac_table_entry_t *mac_table_entry){

    mac_table_entry_t *mac_table_entry_old = mac_table_lookup(mac_table, mac_table_entry->mac.mac);
    
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

static void
l2_switch_perform_mac_learning(node_t *node, char *src_mac, char *intf_name){
    
}

static void
l2_switch_forward_frame(node_t *node, interface_t *recv_intf, char *pkt, unsigned int pkt_size){

    ethernet_hdr_t *ethernet_hdr = (ethernet_hdr_t *)pkt;
    if(IS_MAC_BROADCAST_ADDR(ethernet_hdr->dst_mac.mac)){
        send_pkt_flood_l2_intf_only(node, recv_intf, pkt, pkt_size);
        return;
    }

    mac_table_entry_t *mac_table_entry = mac_table_lookup(NODE_MAC_TABLE(node), ethernet_hdr->dst_mac.mac);
    if(!mac_table_entry){
        send_pkt_flood_l2_intf_only(node, recv_intf, pkt, pkt_size);
        return;
    }

    char *oif_name = mac_table_entry->oif_name;
    interface_t *oif = get_node_if_by_name(node, oif_name); 

    if(!oif){
        return;
    }

    send_pkt_out(pkt, pkt_size, oif);
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