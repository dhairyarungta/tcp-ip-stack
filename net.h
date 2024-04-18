#ifndef __NET__
#define __NET__

#include "utils.h"
#include <memory.h>


typedef struct graph_ graph_t;
typedef struct interface_ interface_t;
typedef struct node_ node_t;

typedef struct ip_add_{
    unsigned char ip_addr[16];
}ip_add_t;

typedef struct mac_add_{
    unsigned char mac[6];
}mac_add_t;

typedef struct node_nw_prop_{
    /*To find various node device capabilities*/
    unsigned int flags;

    /*L3 properties*/
    bool_t is_lb_addr_config;
    ip_add_t lb_addr; /*loopback address of node*/
}node_nw_prop_t;

static inline void
init_node_nw_prop(node_nw_prop_t *node_nw_prop){
    node_nw_prop->flags = 0;
    node_nw_prop->is_lb_addr_config = FALSE;
    memset(node_nw_prop->lb_addr.ip_addr,0, 16);
}

typedef struct intf_nw_props_{
    /*L2 properties*/
    mac_add_t mac_add;

    /*L3 properties*/
    bool_t is_ipadd_config; /*Set to TRUE if ip add is configured
                            intf operates in L3 mode if ip add is configured on the intf*/
    ip_add_t ip_add;
    char mask;
}intf_nw_props_t;

static inline void
init_intf_nw_prop(intf_nw_props_t *intf_nw_props){
    memset(intf_nw_props->mac_add.mac, 0, sizeof(intf_nw_props->mac_add.mac));
    intf_nw_props->is_ipadd_config = FALSE;
    memset(intf_nw_props->ip_add.ip_addr,0,16);
    intf_nw_props->mask = 0;
}

void
interface_assign_mac_address(interface_t *interface);

/* Shortcuts for easy access*/
#define IF_MAC(intf_ptr) (intf_ptr->intf_nw_props.mac_add.mac)
#define IF_IP(intf_ptr) (intf_ptr->intf_nw_props.ip_add.ip_addr)

#define NODE_LO_ADDR(node_ptr) (node_ptr->node_nw_prop.lb_addr.ip_addr)

#define IS_INTF_L3_MODE(intf_ptr)((intf_ptr->intf_nw_props.is_ipadd_config==TRUE)&&(IF_IP(intf_ptr)!=NULL))

bool_t node_set_loopback_address(node_t *node, char *ip_addr);
bool_t node_set_intf_ip_address(node_t *node, char *local_if, char *ip_addr, char mask);
bool_t node_unset_intf_ip_address(node_t *node, char *local_if);

void dump_nw_graph(graph_t *graph);
void dump_node_nw_props(node_t *node);
void dump_intf_props(interface_t *interface);

unsigned int
convert_ip_from_str_to_int(char *ip_addr);

void
convert_ip_fron_int_to_str(unsigned int ip_addr, char *output_buffer);

#endif
