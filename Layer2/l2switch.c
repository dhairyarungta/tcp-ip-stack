#include <stdlib.h>
#include "../net.h"
#include "../comm.h"
#include "../graph.h"
#include "layer2.h"

typedef struct mac_table_entry_ {
    mac_add_t mac; /*KEY*/
    char oif[IF_NAME_SIZE];
    glthread_t mac_entry_glue;
} mac_table_entry_t;

typedef struct mac_table_{
    glthread_t mac_entries;
}mac_table_t;


bool_t 
mac_table_entry_add(mac_table_t *mac_table, mac_table_entry_t *mac_table_entry){

}

mac_table_entry_t *
mac_table_lookup(mac_table_t *mac_table, char *mac){

}

void
delete_mac_table_entry (mac_table_t *mac_table, char *mac){

}

void
init_mac_table(mac_table_t **mac_table){

}

static void
l2_switch_perform_mac_learning(node_t *node, char *src_mac, char *intf_name){
    
}

static void
l2_switch_forward_frame(node_t *node, interface_t *interface, char *pkt, unsigned int pkt_size){

    ethernet_hdr_t *ethernet_hdr = (ethernet_hdr_t *)pkt;
    if(IS_MAC_BROADCAST_ADDR(ethernet_hdr->dst_mac.mac)){
        send_pkt_flood_l2_intf_only(node, interface, pkt, pkt_size);
        return;
    }

    mac_table_entry_t *mac_table_entry = mac_table_lookup(NODE_MAC_TABLE(node), ethernet_hdr->dst_mac.mac);
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