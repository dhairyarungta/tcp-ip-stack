#include "layer3.h"
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

void 
init_rt_table(rt_table_t **rt_table){

    *rt_table = (rt_table_t *)calloc(1, sizeof(rt_table_t));
    init_glthread(&((*rt_table)->route_list));
}

static bool_t
_rt_table_entry_add(rt_table_t *rt_table, l3_route_t *l3_route){

    l3_route_t *l3_route_old = rt_table_lookup(rt_table, l3_route->dest, l3_route->mask);

    if(l3_route_old != NULL && IS_L3_ROUTES_EQUAL(l3_route, l3_route_old)){
        return FALSE;
    }

    if(l3_route_old != NULL){
        delete_rt_table_entry(rt_table, l3_route_old->dest, l3_route_old->mask);
    }
    init_glthread(&l3_route->rt_glue);
    glthread_add_next(&rt_table->route_list, &(l3_route->rt_glue));
    return TRUE;
}

void
delete_rt_table_entry(rt_table_t *rt_table, char *ip_addr, char mask){

    char dst_str_with_mask[16] ;
    memset(dst_str_with_mask, 0, 16);
    apply_mask(ip_addr, mask, dst_str_with_mask);
    l3_route_t *l3_route = rt_table_lookup(rt_table, dst_str_with_mask, mask);

    if(l3_route != NULL){
        remove_glthread(&l3_route->rt_glue);
        free(l3_route);
    }
}

void
rt_table_add_direct_route(rt_table_t *rt_table, char *dst, char mask){

    rt_table_add_route(rt_table, dst, mask, 0, 0);
}

void
dump_rt_table(rt_table_t *rt_table){

    glthread_t *curr = NULL;
    l3_route_t *l3_route =  NULL;   
    printf("L3 Routing Table:\n");
    ITERATE_GLTHREAD_BEGIN(&rt_table->route_list, curr){
        l3_route = rt_glue_to_l3_route(curr);
        printf("\t%-18s %-4d %-18s %s\n",
            l3_route->dest, l3_route->mask, 
            l3_route->is_direct ? "NA" : l3_route->gw_ip,
            l3_route->is_direct ? "NA" : l3_route->oif);
    }
    ITERATE_GLTHREAD_END(&rt_table->route_list, curr);
}

void 
rt_table_add_route(rt_table_t *rt_table, char *dst, char mask, 
    char *gw, char *oif){

    unsigned int dst_int;
    char dst_str_with_mask[16];
    memset(dst_str_with_mask, 0, 16);
    apply_mask(dst, mask, dst_str_with_mask);
    inet_pton(AF_INET, dst_str_with_mask, &dst_int);
    dst_int = htonl(dst_int);

    l3_route_t *l3_route = l3rib_lookup_lpm(rt_table, dst_int);

    /*Route range exists*/
    assert(!l3_route);

    l3_route = (l3_route_t *)calloc(1, sizeof(l3_route_t));
    strncpy(l3_route->dest, dst_str_with_mask, 16);
    l3_route->dest[15]= '\0';
    l3_route->mask = mask;

    if(!gw && !oif){
        l3_route->is_direct = TRUE;
    }
    else{
        l3_route->is_direct = FALSE;
    }
    
    if(gw && oif){
        strncpy(l3_route->gw_ip, gw, 16);
        l3_route->gw_ip[15] = 0;
        strncpy(l3_route->oif, oif, IF_NAME_SIZE);
        l3_route->oif[IF_NAME_SIZE-1] = '\0';
    }

    if(!_rt_table_entry_add(rt_table, l3_route)){
        printf("Error : Route %s/%d Installation Failed\n",
            dst_str_with_mask, mask);
        free(l3_route);
    }
}

/*L3 routing using lpm : longest prefix match*/
l3_route_t *
l3rib_lookup_lpm(rt_table_t *rt_table, uint32_t dst_ip){
    
    char dst_ip_str[16];
    char subnet[16]; /*Dst ip subnet for each mask for the L3 Route entry*/

    dst_ip = htonl(dst_ip);
    inet_ntop(AF_INET, &dst_ip, dst_ip_str, 16);
    dst_ip_str[15] = '\0'; 

    unsigned int longest_mask = 0;
    l3_route_t* l3_route = NULL;
    l3_route_t* l3_route_lpm = NULL;
    l3_route_t* l3_route_default = NULL;

    glthread_t *curr = NULL;
    ITERATE_GLTHREAD_BEGIN(&rt_table->route_list, curr){

        l3_route = rt_glue_to_l3_route(curr);
        memset(subnet, 0 , 16);
        apply_mask(dst_ip_str, l3_route->mask, subnet);
        
        if(strncmp("0.0.0.0", l3_route->dest, 16) == 0 && 
            l3_route->mask == 0){
            l3_route_default = l3_route;
        }
        else if(strncmp(subnet, l3_route->dest, 16) == 0){
            if(l3_route->mask > longest_mask){
                longest_mask = l3_route->mask;
                l3_route_lpm = l3_route;
            }
        }
    }
    ITERATE_GLTHREAD_END(&rt_table->route_list, curr);
    
    return (l3_route_lpm ? l3_route_lpm : l3_route_default);
}

l3_route_t *
rt_table_lookup(rt_table_t *rt_table, char *ip_addr, char mask){

    glthread_t *glthreadptr = NULL;
    l3_route_t *l3_route = NULL;

    ITERATE_GLTHREAD_BEGIN(&rt_table->route_list, glthreadptr){
        l3_route = rt_glue_to_l3_route(glthreadptr);
        if(strncmp(ip_addr, l3_route->dest, 16) == 0 && 
            mask == l3_route->mask){
            return l3_route;
        }
    }
    ITERATE_GLTHREAD_END(&rt_table->route_list, glthreadptr);
    return NULL;
}

void 
promote_pkt_to_layer3(node_t *node, interface_t *interface, 
    char *pkt, unsigned int pkt_size){
        ;
}