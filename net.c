#include "net.h"
#include "graph.h"
#include "utils.h"
#include <stdio.h>
#include <memory.h>

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
   return TRUE;
}

bool_t node_set_intf_ip_address(node_t *node, char *local_if, char *ip_addr, char mask){
    interface_t *interface = get_node_if_by_name(node, local_if);

    assert(interface!=NULL);

    interface->intf_nw_props.is_ipadd_config = TRUE;
    strncpy(IF_IP(interface),ip_addr,16);
    IF_IP(interface)[15]='\0';
    interface->intf_nw_props.mask = mask;
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
    printf("\tFlags : %u\n",node->node_nw_prop.flags);
    if(node->node_nw_prop.is_lb_addr_config==TRUE)
        printf("\tIP Addr : %s",NODE_LO_ADDR(node));

}
void dump_intf_props(interface_t *interface){
    dump_interface(interface);
    if(interface->intf_nw_props.is_ipadd_config == TRUE)
        printf("\tIntf IP Addr : %s, Mask : %u\n",IF_IP(interface), interface->intf_nw_props.mask);
    else
        printf("\tIntf IP Addr : %s\n","Nil");

    printf("MAC Address : %u:%u:%u:%u:%u:%u:%u:%u\n",IF_MAC(interface)[0],IF_MAC(interface)[1],IF_MAC(interface)[2],
    IF_MAC(interface)[3],IF_MAC(interface)[4],IF_MAC(interface)[5]);

}
//
//unsigned int
//convert_ip_from_str_to_int(char *ip_addr){
//    ;
//}
//
//
//void
//convert_ip_fron_int_to_str(unsigned int ip_addr, char *output_buffer){
//    ;
//}