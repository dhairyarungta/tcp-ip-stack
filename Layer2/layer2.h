#ifndef __LAYER2__
#define __LAYER2__

#include <string.h>
#include "../net.h"
#include "../gluethread/glthread.h"
#include "../graph.h"

#pragma pack(push,1)
typedef struct ethernet_hdr_ {
    mac_add_t dst_mac;
    mac_add_t src_mac;
    unsigned short type;
    char payload[248];
    unsigned int FCS; //frame check sequence
} ethernet_hdr_t;
#pragma pack(pop)


static inline bool_t 
l2_frame_recv_qualify_interface(interface_t *interface,ethernet_hdr_t *ethernet_hdr){
    if(! IS_INTF_L3_MODE(interface)){
        return FALSE;
    }
    else if(IS_INTF_L3_MODE(interface) && memcmp(ethernet_hdr->dst_mac.mac,IF_MAC(interface),sizeof(mac_add_t))==0)        
    {
        return TRUE;
    }
    else if(IS_INTF_L3_MODE(interface) && IS_MAC_BROADCAST_ADDR(ethernet_hdr->dst_mac.mac)){
        return TRUE;
    }
    else {
        return FALSE;
    }
    
}

//#define ETH_HDR_SIZE_EXCL_PAYLOAD1 (sizeof(ethernet_hdr_t)-248*sizeof(char))

#define ETH_HDR_SIZE_EXCL_PAYLOAD \
    (sizeof(ethernet_hdr_t)-sizeof(((ethernet_hdr_t*)0)->payload))

//#define ETH_FCS1(eth_hdr_ptr, payload_size) \
    (*(unsigned int*)((char*)(eth_hdr_ptr)+2*sizeof(mac_add_t)\
    +sizeof(unsigned short)+payload_size))

#define ETH_FCS(eth_hdr_ptr, payload_size) \
    (*(unsigned int *)(((char*)(((ethernet_hdr_t *)eth_hdr_ptr)->payload))+payload_size))

static inline ethernet_hdr_t *
ALLOC_ETH_HDR_WITH_PAYLOAD(char *pkt,unsigned int pkt_size){
    //another possible implementation
    //memmove supports overlapping memory
    //memmove(pkt-sizeof(((ethernet_hdr_t*)0)->FCS),pkt,pkt_size );

    char *temp = (char *)calloc(1,pkt_size);
    memcpy(temp,pkt,pkt_size);
    ethernet_hdr_t *eth_hdr = (ethernet_hdr_t *)(pkt-ETH_HDR_SIZE_EXCL_PAYLOAD);
    memset(eth_hdr, 0,ETH_HDR_SIZE_EXCL_PAYLOAD);
    
    memcpy(eth_hdr->payload, temp,pkt_size);
    free(temp);
    return eth_hdr;
    
}


#endif