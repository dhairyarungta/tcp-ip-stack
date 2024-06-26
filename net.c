#include "net.h"
#include "graph.h"
#include "utils.h"
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

extern void rt_table_add_direct_route(rt_table_t *rt_table, char *dst, char mask);

static unsigned int
hash_code(void *ptr, unsigned int size){

    unsigned int value = 0;
    unsigned int i = 0;
    char *str = (char*)ptr;
    while(i<size){
        value +=*str;
        value*=97;
        str++;
        i++;
    }
    return value;
}

void 
interface_assign_mac_address(interface_t *interface){

    node_t *node = interface->att_node;
    if(!node)
        return;
    
    unsigned int hash_code_val = 0;
    hash_code_val = hash_code(node->node_name, NODE_NAME_SIZE);
    hash_code_val *=hash_code(interface->if_name, IF_NAME_SIZE);
    memset(IF_MAC(interface), 0, sizeof(IF_MAC(interface)));
    memcpy(IF_MAC(interface), (char*)&hash_code_val, sizeof (unsigned int));
}


bool_t node_set_loopback_address(node_t *node, char *ip_addr){

    assert(ip_addr);
    node->node_nw_prop.is_lb_addr_config = TRUE;
    strncpy(NODE_LO_ADDR(node),ip_addr,16);
    NODE_LO_ADDR(node)[15]='\0';
    rt_table_add_direct_route(NODE_RT_TABLE(node), ip_addr, 32);
    return TRUE;
}

bool_t node_set_intf_ip_address(node_t *node, char *local_if, char *ip_addr, char mask){

    interface_t *interface = get_node_if_by_name(node, local_if);

    assert(interface!=NULL);

    interface->intf_nw_props.is_ipadd_config = TRUE;
    strncpy(IF_IP(interface),ip_addr,16);
    IF_IP(interface)[15]='\0';
    interface->intf_nw_props.mask = mask;
    IF_L2_MODE(interface) = L2_MODE_UNKNOWN;
    rt_table_add_direct_route(NODE_RT_TABLE(node), ip_addr, mask);
    return TRUE;
}

void dump_nw_graph(graph_t *graph){
    
    printf("\nGraph Name : %s\n", graph->topology_name);
    glthread_t *glthreadptr = NULL;
    node_t *node = NULL;
    ITERATE_GLTHREAD_BEGIN(&graph->node_list,glthreadptr){
        node = graph_glue_to_node(glthreadptr);
        dump_node_nw_props(node);

        interface_t *intf = NULL;
        for (int i = 0;i<MAX_INTF_PER_NODE;i++){
            intf = node->intf[i];
            if(!intf)
                break; 
            dump_intf_props(intf);
        }

    }ITERATE_GLTHREAD_END(&graph->node_list,glthreadptr)
}

void dump_node_nw_props(node_t *node){

    printf("\nNode name : %s\n",node->node_name);
    if(node->node_nw_prop.is_lb_addr_config==TRUE)
        printf("Loopback IP Addr : %s\n",NODE_LO_ADDR(node));

}
void dump_intf_props(interface_t *interface){

    dump_interface(interface);

    if(interface->intf_nw_props.is_ipadd_config == TRUE)
        printf("\tIntf IP Addr : %s, Mask : %u\n",IF_IP(interface), interface->intf_nw_props.mask);
    //else
        //printf("\tIntf IP Addr : %s\n","Nil");

    if(IF_L2_MODE(interface) == L2_MODE_UNKNOWN){
        printf("\tMAC Address : %u:%u:%u:%u:%u:%u",IF_MAC(interface)[0],IF_MAC(interface)[1],IF_MAC(interface)[2],
        IF_MAC(interface)[3],IF_MAC(interface)[4],IF_MAC(interface)[5]);
    }

    if(interface->intf_nw_props.is_ipadd_config!=TRUE){
        printf("\tIntf Mode : %s, VLAN Membership : ",
        intf_l2_mode_str(interface->intf_nw_props.intf_l2_mode));

        for(unsigned int i = 0; i<MAX_VLAN_MEMBERSHIP; i++){
            if(interface->intf_nw_props.vlans[i]!=0) {
                printf("%u ",interface->intf_nw_props.vlans[i]);
            }
        }
    }
    printf("\n\n");
}

/*does NOT manage byte ordering,
assumes correct byte ordering is returned*/
unsigned int
ip_addr_p_to_n(char *ip_addr){  

    char *temp = (char *)calloc(1,sizeof(ip_add_t));
    strncpy(temp,ip_addr,strnlen(ip_addr,sizeof(ip_add_t)));
    char* delimiter_character = ".";
    char *token = strtok(temp, delimiter_character);
    unsigned int ip_uint= 0;    
    int shift_count = 24;
    while(token!=NULL){
        //int temp = atoi(token) ;
        unsigned int temp = atoi(token) ;
        ip_uint^=(temp<<shift_count);
        shift_count-=8;
        token = strtok(NULL,delimiter_character);
    }

    return ip_uint;

}

/*does NOT manage byte ordering,
assumes correct byte ordering is passed to argument*/
void
ip_addr_n_to_p(unsigned int ip_addr, char *ip_addr_str){

    unsigned int mask = 0xFF000000;
    int shitf_count = 24;
     char ip_p [sizeof(ip_add_t)];
     char *write_position = ip_p;
    for (int i=0;i<4;i++){
        unsigned int ip_part = mask & ip_addr;
        ip_part>>=shitf_count;
        int n = sprintf(write_position,"%u",ip_part);
        write_position+=n;
        if(i!=3){
            sprintf(write_position,".");
            write_position++;
        }
        shitf_count-=8;
        mask = mask>>8;
    }
     
    memcpy(ip_addr_str,ip_p, sizeof(ip_add_t));
    return;
}

char *
pkt_buffer_shift_right(char *pkt, unsigned int pkt_size, 
    unsigned int total_buffer_size){
    
    unsigned int left_space = total_buffer_size-pkt_size;

    //check if overlap otherwise avoid memory allocation
    if(pkt_size*2>total_buffer_size){
        char *temp = calloc(1, pkt_size);
        memcpy(temp, pkt, pkt_size);
        memset(pkt, 0, total_buffer_size);
        memcpy(pkt+left_space, temp, pkt_size);
        free(temp);
        return pkt+left_space;
    }

    memcpy(pkt+left_space, pkt, pkt_size);
    memset(pkt, 0, pkt_size);
    return pkt+left_space;
}

unsigned int
convert_ip_from_str_to_int(char *ip_addr){

    unsigned int prefix = 0 ;
    inet_pton(AF_INET, ip_addr, &prefix);
    //prefix is now in network byte order i.e. big endian

    prefix = htonl(prefix);
    //reverses prefix from big endian to little endian

    /*Returns in host byte order*/
    return prefix;
}


void
convert_ip_fron_int_to_str(unsigned int ip_addr, char *output_buffer){

    output_buffer = !output_buffer ? calloc(1, sizeof(ip_add_t)): output_buffer;
    ip_addr = htonl(ip_addr);
    inet_ntop(AF_INET, &ip_addr, output_buffer, 16);
    output_buffer[15] = '\0';
}

interface_t *
node_get_matching_subnet_interface(node_t *node, char *ip_addr){

    char *ip_subnet = calloc(1, sizeof(ip_add_t)) ;
    char *intf_subnet = calloc(1, sizeof(ip_add_t));

    for(unsigned int i = 0; i < MAX_INTF_PER_NODE; i++){
        interface_t* intf = (node->intf)[i];

        if(!intf){
            return NULL;
        }

        if(intf->intf_nw_props.is_ipadd_config == TRUE){
            apply_mask(ip_addr, intf->intf_nw_props.mask, ip_subnet);
            apply_mask(IF_IP(intf), intf->intf_nw_props.mask, intf_subnet);
            if(strncmp(ip_subnet, intf_subnet, 16) == 0){
                return intf;
            }

        }
    }
    return NULL;
}

unsigned int 
get_access_intf_operating_vlan_id(interface_t *interface){

    assert(IF_L2_MODE(interface)==ACCESS);
    return (interface->intf_nw_props.vlans[0]);
}

bool_t 
is_trunk_interface_vlan_enabled(interface_t *interface, unsigned int vlan_id){

    assert(IF_L2_MODE(interface)==TRUNK);
    for(unsigned int i = 0; i < MAX_VLAN_MEMBERSHIP; i++){
        if(interface->intf_nw_props.vlans[i]==vlan_id){
            return TRUE;
        }
    }
    return FALSE;
}