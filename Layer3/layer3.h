#ifndef __LAYER3__
#define __LAYER3__

#include "../net.h"
#include "../graph.h"
#include "../gluethread/glthread.h"

#pragma pack(push,1)
/*IPV4 header format as per standard specification*/
typedef struct ip_hdr_ {
    unsigned int version : 4; /*version number, 4 for IPV4*/
    unsigned int ihl : 4;
    char tos; 
    unsigned short total_length;

    short identification;
    unsigned int unused_flag : 1;
    unsigned int DF_flag : 1;
    unsigned int MORE_flag : 1;
    unsigned int frag_offset :1;

    char ttl;
    char protocol;
    unsigned short checksum;
    unsigned int src_ip;
    unsigned int dst_ip;
}ip_hdr_t;
#pragma pack(pop)

static inline void
initialize_ip_hdr(ip_hdr_t *ip_hdr){
    ip_hdr->version = 4;
    ip_hdr->ihl = 5;
    ip_hdr->tos = 0;
    ip_hdr->total_length = 0;

    ip_hdr->identification = 0;
    ip_hdr->unused_flag = 0;
    ip_hdr->DF_flag = 1;
    ip_hdr->MORE_flag = 0;
    ip_hdr->frag_offset = 0;

    ip_hdr->ttl = 64;
    ip_hdr->protocol = 0;
    ip_hdr->checksum = 0;
    ip_hdr->src_ip = 0;
    ip_hdr->dst_ip = 0;
}

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

#define IP_HDR_LEN_IN_BYTES(ip_hdr_ptr) (4*ip_hdr_ptr->ihl)
#define IP_HDR_TOTAL_LEN_IN_BYTES(ip_hdr_ptr) (4*ip_hdr_ptr->total_length)
#define INCREMENT_IPHDR(ip_hdr_ptr) (((char *)ip_hdr_ptr)+(4*ip_hdr_ptr->ihl))
#define IP_HDR_PAYLAOD_SIZE(ip_hdr_ptr) (4*((ip_hdr_ptr->total_length)-(ip_hdr_ptr->ihl)))

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