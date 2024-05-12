#include "layer2.h"
#include "../graph.h"
#include <stdlib.h>
#include "../tcpconst.h"
#include <arpa/inet.h>
#include <stdio.h>
#include "../comm.h"

extern void promote_pkt_to_layer3(node_t *node, interface_t *interface, 
    char *pkt, unsigned int pkt_size);

extern void l2_switch_recv_frame(interface_t *interface, 
    char *pkt, unsigned int pkt_size);

void 
send_arp_broadcast_request(node_t *node, interface_t *oif, char *ip_addr){

    unsigned int payload_size = sizeof(arp_hdr_t);
    ethernet_hdr_t *ethernet_hdr = (ethernet_hdr_t *) calloc(1,payload_size + ETH_HDR_SIZE_EXCL_PAYLOAD);
    if(!oif){

        /*oif is allowed to be NULL when the function is called*/
        oif = node_get_matching_subnet_interface(node, ip_addr);
        if(!oif){
            printf("Error Node : %s , No eligible subnet for ARP resolution for IP Address : %s",
                node->node_name, ip_addr);
            return ;
        }
    }

    layer2_fill_with_broadcast_mac(ethernet_hdr->dst_mac.mac);
    memcpy(ethernet_hdr->src_mac.mac, IF_MAC(oif),sizeof(mac_add_t));
    ethernet_hdr->type = ARP_MSG;

    arp_hdr_t *arp_hdr = (arp_hdr_t *) ethernet_hdr->payload;
    arp_hdr->hw_type = 1;
    arp_hdr->proto_type = 0x0800;
    arp_hdr->hw_addr_len = sizeof(mac_add_t);
    arp_hdr->proto_addr_len = 4;
    arp_hdr->op_code = ARP_BROAD_REQ;


    memcpy(arp_hdr->src_mac.mac, IF_MAC(oif), sizeof(mac_add_t));
    inet_pton(AF_INET, IF_IP(oif), &arp_hdr->src_ip);
    arp_hdr->src_ip = htonl(arp_hdr->src_ip);

    memset(arp_hdr->dst_mac.mac, 0, sizeof(mac_add_t));
    inet_pton(AF_INET, ip_addr, &arp_hdr->dst_ip);
    arp_hdr->dst_ip = htonl(arp_hdr->dst_ip);

    ETH_FCS(ethernet_hdr, payload_size) = 0;

    send_pkt_out((char *)ethernet_hdr, ETH_HDR_SIZE_EXCL_PAYLOAD+payload_size, oif);
    free(ethernet_hdr);
}

static void 
send_arp_reply_msg(ethernet_hdr_t *ethernet_hdr_in, interface_t *oif){

    arp_hdr_t *arp_hdr_in = (arp_hdr_t *) GET_ETHERNET_HDR_PAYLOAD(ethernet_hdr_in);
    ethernet_hdr_t *ethernet_hdr_reply = (ethernet_hdr_t *) calloc(1, MAX_PACKET_BUFFER_SIZE);
    memcpy(ethernet_hdr_reply->dst_mac.mac, arp_hdr_in->src_mac.mac, sizeof(mac_add_t)); 
    memcpy(ethernet_hdr_reply->src_mac.mac, IF_MAC(oif),sizeof(mac_add_t));

    ethernet_hdr_reply->type = ARP_MSG; 
    arp_hdr_t *arp_hdr_reply = (arp_hdr_t *)GET_ETHERNET_HDR_PAYLOAD(ethernet_hdr_reply);

    arp_hdr_reply->hw_type = 1;
    arp_hdr_reply->proto_type = 0x0800;
    arp_hdr_reply->proto_addr_len = 4;
    arp_hdr_reply->hw_addr_len = sizeof(mac_add_t);
    arp_hdr_reply->op_code = ARP_REPLY;

    memcpy(arp_hdr_reply->src_mac.mac, IF_MAC(oif), sizeof(mac_add_t));

    inet_pton(AF_INET, IF_IP(oif), &arp_hdr_reply->src_ip);
    arp_hdr_reply->src_ip = htonl(arp_hdr_reply->src_ip);

    memcpy(arp_hdr_reply->dst_mac.mac, arp_hdr_in->src_mac.mac, sizeof(mac_add_t));

    arp_hdr_reply->dst_ip = arp_hdr_in->src_ip;

    ETH_FCS(ethernet_hdr_reply, sizeof(arp_hdr_t));

    unsigned int total_pkt_size = ETH_HDR_SIZE_EXCL_PAYLOAD + sizeof(arp_hdr_t);
    char *shifted_pkt_buffer = pkt_buffer_shift_right((char *)ethernet_hdr_reply, total_pkt_size, MAX_PACKET_BUFFER_SIZE);

    send_pkt_out(shifted_pkt_buffer, total_pkt_size, oif);
    free(ethernet_hdr_reply);
}

static void
process_arp_broadcast_request(node_t *node, interface_t *iif, 
    ethernet_hdr_t *ethernet_hdr){
    
    printf("%s : ARP Broadcast msg received on interface %s of node %s\n"
        ,__FUNCTION__,iif->if_name,iif->att_node->node_name);
    char ip_addr[16];
    arp_hdr_t *arp_hdr = (arp_hdr_t *)(GET_ETHERNET_HDR_PAYLOAD(ethernet_hdr));
    /* ARP broadcast request msg has passed MAC Address check
     * Now, this node need to reply to this ARP Broadcast req
     * msg if Dst ip address in ARP req msg matches iif's ip address*/

    unsigned int arp_dst_ip = htonl(arp_hdr->dst_ip);
    inet_ntop(AF_INET, &arp_dst_ip, ip_addr, 16);
    ip_addr[15] = '\0';

    if(strncmp(IF_IP(iif), ip_addr, 16)!=0){
        printf("%s : ARP Broadcast req msg dropped , Dst IP Address : %s did not match with interface ip : %s\n",
            node->node_name, ip_addr, IF_IP(iif));
        return;
    }
    send_arp_reply_msg(ethernet_hdr, iif);

}

static void 
process_arp_reply_msg(node_t *node, interface_t *iif, 
    ethernet_hdr_t *ethernet_hdr){

        printf("%s : ARP Reply msg received on interface %s of node %s\n",
            __FUNCTION__, iif->if_name, iif->att_node->node_name);
        
        arp_table_update_from_arp_reply(node->node_nw_prop.arp_table, (arp_hdr_t *)GET_ETHERNET_HDR_PAYLOAD(ethernet_hdr), iif);

}

void 
layer2_frame_recv(node_t *node, interface_t *interface, 
    char *pkt, unsigned int pkt_size){

    /*Entry point into the TCP/IP stack after the physical layer*/

    ethernet_hdr_t *ethernet_hdr = (ethernet_hdr_t *)pkt;
    if(l2_frame_recv_qualify_on_interface(interface, ethernet_hdr) == FALSE){
        printf("L2 Frame Rejected\n");
        return;
    }
    
    printf("L2 Frame Accepted\n");

    if(IS_INTF_L3_MODE(interface)){ 
        switch(ethernet_hdr->type){
            case ARP_MSG :
            {
                arp_hdr_t *arp_hdr = (arp_hdr_t *)(ethernet_hdr->payload);
                switch (arp_hdr->op_code) {
                    case ARP_BROAD_REQ :
                        process_arp_broadcast_request(node, interface, ethernet_hdr); 
                        break;
                    
                    case ARP_REPLY : 
                        process_arp_reply_msg(node, interface, ethernet_hdr);
                        break;
                    
                    default:
                        break;
                }
                break;
            }
            default:
                promote_pkt_to_layer3(node, interface, pkt, pkt_size);
                break;
        }
    }
    else if(IF_L2_MODE(interface)==TRUNK || IF_L2_MODE(interface)==ACCESS){
        l2_switch_recv_frame(interface, pkt, pkt_size);
    }
    else{
        return;
    }
}

void 
init_arp_table(arp_table_t** arp_table){
    *arp_table = calloc(1, sizeof(arp_table_t));
    init_glthread(&((*arp_table)->arp_entries));
}

arp_entry_t *
arp_table_lookup(arp_table_t *arp_table, char *ip_addr){
    glthread_t *glthreadptr = NULL;
    arp_entry_t *arp_entry = NULL;
    ITERATE_GLTHREAD_BEGIN(&(arp_table->arp_entries), glthreadptr){
        arp_entry = arp_glue_to_arp(glthreadptr);
        if(strncmp(arp_entry->ip_addr.ip_addr,ip_addr,16)==0){
           return arp_entry; 
        }
    }ITERATE_GLTHREAD_END(&(arp_table->arp_entries), glthreadptr);
    return NULL;
}

bool_t 
arp_table_entry_add(arp_table_t *arp_table, arp_entry_t *arp_entry){

    arp_entry_t * arp_entry_old = arp_table_lookup(arp_table, arp_entry->ip_addr.ip_addr);
    
    if(arp_entry_old && IS_ARP_ENTRIES_EQUAL(arp_entry, arp_entry_old)){
        return FALSE;
    }
    
    if(arp_entry_old){
        delete_arp_table_entry(arp_table, arp_entry->ip_addr.ip_addr);
    }

    init_glthread(&arp_entry->arp_glue);
    glthread_add_next(&arp_table->arp_entries, &arp_entry->arp_glue);
    return TRUE;
}

void
arp_table_update_from_arp_reply(arp_table_t *arp_table, 
    arp_hdr_t *arp_hdr, interface_t *iif){

    unsigned int src_ip = 0;
    assert(arp_hdr->op_code == ARP_REPLY);
    arp_entry_t *arp_entry = calloc(1, sizeof(arp_entry_t));
    src_ip = htonl(arp_hdr->src_ip);
    inet_ntop(AF_INET, &src_ip, arp_entry->ip_addr.ip_addr, 16);
    arp_entry->ip_addr.ip_addr[15] = '\0';
    memcpy(arp_entry->mac_addr.mac,arp_hdr->src_mac.mac,sizeof(mac_add_t));
    strncpy(arp_entry->oif_name, iif->if_name, IF_NAME_SIZE);

    bool_t rc = arp_table_entry_add(arp_table, arp_entry);
    if(rc == FALSE){
        free(arp_entry);
    }

}

void 
delete_arp_table_entry(arp_table_t *arp_table, char *ip_addr){

    arp_entry_t *arp_entry = arp_table_lookup(arp_table,ip_addr);
    if(arp_entry==NULL)
        return;

    remove_glthread(&(arp_entry->arp_glue));
    free(arp_entry);
}

void
dump_arp_table(arp_table_t *arp_table){
    glthread_t *glthreadptr = NULL;
    arp_entry_t *arp_entry = NULL;

    ITERATE_GLTHREAD_BEGIN(&arp_table->arp_entries, glthreadptr){
        arp_entry = arp_glue_to_arp(glthreadptr);
        printf("IP : %s, MAC : %u:%u:%u:%u:%u:%u, OIF : %s\n",
        arp_entry->ip_addr.ip_addr,
        arp_entry->mac_addr.mac[0],
        arp_entry->mac_addr.mac[1],
        arp_entry->mac_addr.mac[2],
        arp_entry->mac_addr.mac[3],
        arp_entry->mac_addr.mac[4],
        arp_entry->mac_addr.mac[5],
        arp_entry->oif_name);
    }ITERATE_GLTHREAD_END(&arp_table->arp_entries, glthreadptr);
}

//TODO add vlan support
void
node_set_intf_l2_mode(node_t *node, char *intf_name,
    intf_l2_mode_t intf_l2_mode){  
    
    interface_t *interface = get_node_if_by_name(node, intf_name);
    assert(interface);
    interface_set_l2_mode(node, interface, intf_l2_mode_str(intf_l2_mode));
}

void
interface_set_l2_mode(node_t *node, 
    interface_t *interface, char *l2_mode_option){
        
    intf_l2_mode_t intf_l2_mode;
    if(strncmp(l2_mode_option, "trunk", strlen("trunk"))==0){
        intf_l2_mode = TRUNK;
    }
    else if(strncmp(l2_mode_option, "access", strlen("access"))==0){
        intf_l2_mode = ACCESS;
    }
    else{
        assert(0);
    }

    if(IS_INTF_L3_MODE(interface)){
        interface->intf_nw_props.is_ipadd_config = FALSE;
        IF_L2_MODE(interface) = intf_l2_mode;
        return;
    }

    if(IF_L2_MODE(interface)==L2_MODE_UNKNOWN){
        IF_L2_MODE(interface) = intf_l2_mode;
        return;
    }

    if(IF_L2_MODE(interface)==intf_l2_mode){
        return;
    }

    if(intf_l2_mode==ACCESS && IF_L2_MODE(interface)==TRUNK){

        IF_L2_MODE(interface) = intf_l2_mode;
        for(unsigned int i = 0; i < MAX_VLAN_MEMBERSHIP; i++){
            interface->intf_nw_props.vlans[i] = 0;
        }   
        return;
    }

    if(intf_l2_mode==TRUNK && IF_L2_MODE(interface)==ACCESS){

        IF_L2_MODE(interface) = intf_l2_mode;
        return ;
    }
    assert(0);
}

ethernet_hdr_t *
tag_pkt_with_vlan_id(ethernet_hdr_t *ethernet_hdr, unsigned int total_pkt_size,
    int vlan_id, unsigned int *new_pkt_size){

    //pkt to argument are right shifted, implies enough space on left to add vlan tag fields
    vlan_8021q_hdr_t *vlan_8021q_hdr = is_pkt_vlan_tagged(ethernet_hdr);
    if(vlan_8021q_hdr!=NULL){
        vlan_8021q_hdr->tci_vid = vlan_id;
        return ethernet_hdr;
    }

    ethernet_hdr_t *ethernet_hdr_old = (ethernet_hdr_t *)calloc(1, total_pkt_size);
    memcpy(ethernet_hdr_old, ethernet_hdr, total_pkt_size);
    vlan_ethernet_hdr_t *vlan_ethernet_hdr_new = (vlan_ethernet_hdr_t *)(((char *)ethernet_hdr)- sizeof(vlan_8021q_hdr_t));
    unsigned int payload_size = total_pkt_size - ETH_HDR_SIZE_EXCL_PAYLOAD;

    memset(vlan_ethernet_hdr_new, 0, VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD+payload_size);

    strncpy(vlan_ethernet_hdr_new->src_mac.mac,ethernet_hdr_old->src_mac.mac,sizeof(mac_add_t));
    strncpy(vlan_ethernet_hdr_new->dst_mac.mac,ethernet_hdr_old->dst_mac.mac, sizeof(mac_add_t));
    memcpy(vlan_ethernet_hdr_new->payload, ethernet_hdr_old->payload, payload_size);
    vlan_ethernet_hdr_new->type = ethernet_hdr_old->type;
    VLAN_ETH_FCS(vlan_ethernet_hdr_new, payload_size) = ETH_FCS(ethernet_hdr_old, payload_size);
    vlan_8021q_hdr_t *vlan_8021q_hdr_new = (vlan_8021q_hdr_t *)(vlan_ethernet_hdr_new->payload);
    vlan_8021q_hdr_new->tci_vid = vlan_id;
    SET_COMMON_ETH_FCS((ethernet_hdr_t *)vlan_ethernet_hdr_new, payload_size, 0);

    *new_pkt_size = VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD + payload_size;
    free(ethernet_hdr_old);
    return (ethernet_hdr_t *)vlan_ethernet_hdr_new;
}

ethernet_hdr_t *
untag_pkt_with_vlan_id(ethernet_hdr_t *ethernet_hdr, unsigned int total_pkt_size,
    unsigned int *new_pkt_size){
        
    vlan_8021q_hdr_t *vlan_8021q_hdr = is_pkt_vlan_tagged(ethernet_hdr);
    if(vlan_8021q_hdr == NULL){
        return ethernet_hdr;
    }

    vlan_ethernet_hdr_t *vlan_ethernet_hdr_old = (vlan_ethernet_hdr_t *)calloc(1, total_pkt_size);
    unsigned int payload_size = total_pkt_size - VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD;
    memcpy(vlan_ethernet_hdr_old, ethernet_hdr, total_pkt_size);

    memset(ethernet_hdr, 0, total_pkt_size);    
    
    ethernet_hdr_t *ethernet_hdr_new = (ethernet_hdr_t *)((char *)ethernet_hdr) + sizeof(vlan_8021q_hdr_t);
    strncpy(ethernet_hdr_new->dst_mac.mac, vlan_ethernet_hdr_old->dst_mac.mac, sizeof(mac_add_t));
    strncpy(ethernet_hdr_new->src_mac.mac, vlan_ethernet_hdr_old->src_mac.mac, sizeof(mac_add_t));
    memcpy(ethernet_hdr_new->payload, vlan_ethernet_hdr_old->payload, payload_size);
    ethernet_hdr_new->type = vlan_ethernet_hdr_old->type;
    SET_COMMON_ETH_FCS(ethernet_hdr_new, payload_size, 0);

    *new_pkt_size = ETH_HDR_SIZE_EXCL_PAYLOAD + payload_size;
    free(vlan_ethernet_hdr_old);
    return ethernet_hdr_new;
}

void
node_set_intf_vlan_membership(node_t *node, char *intf_name, unsigned int vlan_id){

    interface_t *interface = get_node_if_by_name(node, intf_name);
    assert(interface);

    interface_set_vlan(node, interface, vlan_id);

}

void
interface_set_vlan(node_t *node, interface_t *interface, unsigned int vlan_id){
    ;
}