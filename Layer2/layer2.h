#ifndef __LAYER2__
#define __LAYER2__

#include <string.h>
#include "../net.h"
#include "../gluethread/glthread.h"
#include "../graph.h"

#pragma pack(push,1)
typedef struct apr_hdr_{
    short hw_type; //hardware type, 1 for eth cable
    short proto_type ; //0x0800 for IPv4
    char hw_addr_len;
    char proto_addr_len;
    short op_code;
    mac_add_t src_mac;
    unsigned int src_ip;
    mac_add_t dst_mac;
    unsigned int dst_ip;
}arp_hdr_t;

typedef struct ethernet_hdr_ {
    mac_add_t dst_mac;
    mac_add_t src_mac;
    unsigned short type;
    char payload[248];
    unsigned int FCS; //frame check sequence
} ethernet_hdr_t;
#pragma pack(pop)


typedef struct arp_entry_{
    ip_add_t ip_addr;
    mac_add_t mac_addr;
    char oif_name [IF_NAME_SIZE];
    glthread_t arp_glue;
}arp_entry_t;

typedef struct arp_table_{
    glthread_t arp_entries;
}arp_table_t;

GLTHREAD_TO_STRUCT(arp_glue_to_arp, arp_entry_t, arp_glue);


static inline bool_t 
l2_frame_recv_qualify_interface(interface_t *interface,ethernet_hdr_t *ethernet_hdr){
    if(! IS_INTF_L3_MODE(interface)){
        return FALSE;
    }
    else if(memcmp(ethernet_hdr->dst_mac.mac,IF_MAC(interface),sizeof(mac_add_t))==0)        
    {
        return TRUE;
    }
    else if(IS_MAC_BROADCAST_ADDR(ethernet_hdr->dst_mac.mac)){
        return TRUE;
    }

    return FALSE;
    
}

//#define ETH_HDR_SIZE_EXCL_PAYLOAD1 (sizeof(ethernet_hdr_t)-248*sizeof(char))

#define ETH_HDR_SIZE_EXCL_PAYLOAD \
    (sizeof(ethernet_hdr_t)-sizeof(((ethernet_hdr_t*)0)->payload))

//#define ETH_FCS1(eth_hdr_ptr, payload_size) \
    (*(unsigned int*)((char*)(eth_hdr_ptr)+2*sizeof(mac_add_t)\
    +sizeof(unsigned short)+payload_size))

/*Never access FCS field directly because for allocated memory which is casted to 
ethernet_hdr_t * size of payload is varying*/
#define ETH_FCS(eth_hdr_ptr, payload_size) \
    (*(unsigned int *)(((char*)(((ethernet_hdr_t *)eth_hdr_ptr)->payload))+payload_size))

static inline ethernet_hdr_t *
ALLOC_ETH_HDR_WITH_PAYLOAD(char *pkt,unsigned int pkt_size){

    char *temp = (char *)calloc(1,pkt_size);
    memcpy(temp,pkt,pkt_size);
    ethernet_hdr_t *eth_hdr = (ethernet_hdr_t *)(pkt-ETH_HDR_SIZE_EXCL_PAYLOAD);
    memset(eth_hdr, 0,ETH_HDR_SIZE_EXCL_PAYLOAD);
    
    memcpy(eth_hdr->payload, temp,pkt_size);
    free(temp);
    return eth_hdr;
    
}

static void
process_arp_reply_msg(node_t *node, interface_t *iif, ethernet_hdr_t *ethernet_hdr);

static void
send_arp_reply_msg(ethernet_hdr_t *ethernet_hdr_in, interface_t *oif);

static void
process_arp_broadcast_request(node_t *node, interface_t *iif, ethernet_hdr_t *ethernet_hdr);

void 
init_arp_table(arp_table_t **arp_table);

bool_t 
arp_table_entry_add(arp_table_t *arp_table, arp_entry_t *arp_entry);

void
arp_table_update_from_arp_reply(arp_table_t *arp_table, arp_hdr_t *arp_hdr, interface_t *iif);//incoming intf

void 
delete_arp_table_entry(arp_table_t *arp_table, char *ip_addr);

void
dump_arp_table(arp_table_t *arp_table);

void
send_arp_broadcast_request(node_t *node, interface_t *oif, char *ip_addr);

arp_entry_t *
arp_table_lookup(arp_table_t *arp_table, char *ip_addr);

void 
layer2_frame_recv(node_t *node, interface_t *interface, 
    char *pkt, unsigned int pkt_size);

static inline bool_t
l2_frame_recv_qualify_on_interface(interface_t *interface, 
    ethernet_hdr_t *ethernet_hdr){
    
    if(!interface->intf_nw_props.is_ipadd_config){
        return FALSE;
    }

    if(memcmp(IF_MAC(interface),ethernet_hdr->dst_mac.mac,sizeof(mac_add_t)) == 0){
        return TRUE;
    }

    if(IS_MAC_BROADCAST_ADDR(ethernet_hdr->dst_mac.mac)){
        return TRUE;
    }

    return FALSE;
}

static inline char *
GET_ETHERNET_HDR_PAYLOAD(ethernet_hdr_t *ethernet_hdr){
    return ethernet_hdr->payload;
}

void
node_set_intf_l2_mode(node_t *node, char *intf_name,
    intf_l2_mode_t intf_l2_mode);

#endif