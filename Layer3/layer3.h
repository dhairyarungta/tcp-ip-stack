#ifndef __LAYER3__
#define __LAYER3__

#include "../net.h"
#include "../graph.h"
#include "../gluethread/glthread.h"

typedef struct rt_table_ {
    glthread_t route_list;
}rt_table_t;

/*l3_route_t represents a single entry in the rt_table_t*/
typedef struct l3_route_ {
    char dest[16] ; /*key, using longest prefix match*/
    char mask ; /*key, using longest prefix match*/
    bool_t is_direct ; /*if set to TRUE, then gw_ip and oif have no meaning*/
    char gw_ip[16] ;
    char oif[IF_NAME_SIZE] ; /*OIF*/
    glthread_t rt_glue;
}l3_route_t ;

GLTHREAD_TO_STRUCT(rt_glue_to_l3_route, l3_route_t, rt_glue);

void 
init_rt_table(rt_table_t **rt_table);

void
rt_table_add_direct_route(rt_table_t *rt_table, char *dst, char mask);

void
dump_rt_table(rt_table_t *rt_table);

void 
rt_table_add_route(rt_table_t *rt_table, char *dst, char mask, 
    char *gw, char *oif);

/*L3 routing using lpm : longest prefix match*/
l3_route_t *
l3rib_lookup_lpm(rt_table_t *rt_table, uint32_t dest_ip);

l3_route_t *
rt_table_lookup(rt_table_t *rt_table, char *ip_addr, char mask);

void
delete_rt_table_entry(rt_table_t *rt_table, char *ip_addr, char mask);

void 
promote_pkt_to_layer3(node_t *node, interface_t *interface, 
    char *pkt, unsigned int pkt_size);

#define IS_L3_ROUTES_EQUAL(rt_ptr_1, rt_ptr_2)\
    (strncmp(rt_ptr_1->dest, rt_ptr_2->dest, 16)==0) &&\
    (rt_ptr_1->mask == rt_ptr_2->mask) &&\
    (rt_ptr_1->is_direct == rt_ptr_2->is_direct) &&\
    (strncmp(rt_ptr_1->gw_ip, rt_ptr_2->gw_ip,16)==0) &&\
    (strncmp(rt_ptr_1->oif, rt_ptr_2->oif, IF_NAME_SIZE))

#endif