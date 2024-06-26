#ifndef __NET__
#define __NET__

#include "utils.h"
#include <memory.h>

#define MAX_VLAN_MEMBERSHIP 10

/*Forward declarations*/
typedef struct graph_ graph_t;
typedef struct interface_ interface_t;
typedef struct node_ node_t;
typedef struct arp_table_ arp_table_t;
typedef struct mac_table_ mac_table_t;
typedef struct rt_table_ rt_table_t;

extern void init_arp_table(arp_table_t **arp_table);
extern void init_mac_table(mac_table_t **mac_table);
extern void init_rt_table(rt_table_t **rt_table);

typedef enum {
    ACCESS,/*Atmost 1 VLAN Membership*/
    TRUNK, /*Atleast 1 VLAN Membership*/
    L2_MODE_UNKNOWN
}intf_l2_mode_t;

typedef struct ip_add_{
    unsigned char ip_addr[16];
}ip_add_t;

typedef struct mac_add_{
    unsigned char mac[6];
}mac_add_t;

typedef struct node_nw_prop_{
    /*To find various node device capabilities*/
    unsigned int flags;
    arp_table_t *arp_table;      
    mac_table_t *mac_table;
    rt_table_t *rt_table;

    /*L3 properties*/
    bool_t is_lb_addr_config;
    ip_add_t lb_addr; /*loopback address of node*/
    
}node_nw_prop_t;

static inline char * 
intf_l2_mode_str(intf_l2_mode_t intf_l2_mode){
    switch (intf_l2_mode) {
        case ACCESS:
            return "access";
        case TRUNK:
            return "trunk";
        default:
            return "L2_MODE_UNKNOWN";
    }
}



static inline void
init_node_nw_prop(node_nw_prop_t *node_nw_prop){

    node_nw_prop->flags = 0;
    node_nw_prop->is_lb_addr_config = FALSE;
    memset(node_nw_prop->lb_addr.ip_addr,0, 16);
    init_arp_table(&(node_nw_prop->arp_table));
    init_mac_table(&(node_nw_prop->mac_table));
    init_rt_table(&(node_nw_prop->rt_table));
}

typedef struct intf_nw_props_{
    /*L2 properties*/
    mac_add_t mac_add;

    /*L3 properties*/
    bool_t is_ipadd_config; /*Set to TRUE if ip add is configured
                            intf operates in L3 mode if ip addr is configured on the intf*/
    ip_add_t ip_add;
    char mask;
    intf_l2_mode_t intf_l2_mode; /*if IP Address is configured 
                                    then this is set to L2_MODE_UNKNOWN*/
    unsigned int vlans[MAX_VLAN_MEMBERSHIP]; /*0 is the default value of 
                                             each array element indicating no vlan*/
}intf_nw_props_t;

static inline void
init_intf_nw_prop(intf_nw_props_t *intf_nw_props){

    memset(intf_nw_props->mac_add.mac, 0, sizeof(intf_nw_props->mac_add.mac));
    intf_nw_props->is_ipadd_config = FALSE;
    memset(intf_nw_props->ip_add.ip_addr,0,16);
    intf_nw_props->mask = 0;
    intf_nw_props->intf_l2_mode = L2_MODE_UNKNOWN;
    memset(intf_nw_props->vlans, 0, sizeof(intf_nw_props->vlans));
}

void
interface_assign_mac_address(interface_t *interface);

/* Shortcuts for easy access*/
#define IF_MAC(intf_ptr) (intf_ptr->intf_nw_props.mac_add.mac)
#define IF_IP(intf_ptr) (intf_ptr->intf_nw_props.ip_add.ip_addr)

#define NODE_LO_ADDR(node_ptr) (node_ptr->node_nw_prop.lb_addr.ip_addr)

#define IS_INTF_L3_MODE(intf_ptr)((intf_ptr->intf_nw_props.is_ipadd_config==TRUE)&&(IF_IP(intf_ptr)!=NULL))
#define NODE_ARP_TABLE(node) (node->node_nw_prop.arp_table)
#define NODE_MAC_TABLE(node) (node->node_nw_prop.mac_table)
#define IF_L2_MODE(intf_ptr) (intf_ptr->intf_nw_props.intf_l2_mode)
#define NODE_RT_TABLE(node) (node->node_nw_prop.rt_table)

bool_t node_set_loopback_address(node_t *node, char *ip_addr);
bool_t node_set_intf_ip_address(node_t *node, char *local_if, char *ip_addr, char mask);
bool_t node_unset_intf_ip_address(node_t *node, char *local_if);

void dump_nw_graph(graph_t *graph);
void dump_node_nw_props(node_t *node);
void dump_intf_props(interface_t *interface);

unsigned int 
ip_addr_p_to_n(char *ip_addr);

void
ip_addr_n_to_p(unsigned int ip_addr, char *ip_addr_str);

unsigned int
convert_ip_from_str_to_int(char *ip_addr);

void
convert_ip_fron_int_to_str(unsigned int ip_addr, char *output_buffer);

char *
pkt_buffer_shift_right(char *pkt, unsigned int pkt_size,
    unsigned int total_buffer_size);

interface_t *
node_get_matching_subnet_interface(node_t *node, char *ip_addr);

unsigned int 
get_access_intf_operating_vlan_id(interface_t *interface);

bool_t 
is_trunk_interface_vlan_enabled(interface_t *interface, unsigned int vlan_id);

#endif
