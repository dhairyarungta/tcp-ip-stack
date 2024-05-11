#include "graph.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "comm.h"

void
insert_link_between_two_nodes(node_t *node1,
    node_t *node2, char* from_if_name, char *to_if_name, unsigned int cost){
        
        link_t* link = calloc(1,sizeof(link_t));
        
        strncpy(link->intf1.if_name, from_if_name,IF_NAME_SIZE);
        strncpy(link->intf2.if_name, to_if_name,IF_NAME_SIZE);
        link->intf1.att_node = node1;
        link->intf2.att_node = node2;
        link->intf1.link = link;
        link->intf2.link = link;
        link->cost = cost;

        int intf_index1 = get_node_intf_available_slot(node1);
        int intf_index2 = get_node_intf_available_slot(node2);

        node1->intf[intf_index1] = &(link->intf1);
        node2->intf[intf_index2] = &(link->intf2);

        init_intf_nw_prop(&link->intf1.intf_nw_props);
        init_intf_nw_prop(&link->intf2.intf_nw_props);

        /*Assign random generated MAC address to each interface*/
        interface_assign_mac_address(&link->intf1);
        interface_assign_mac_address(&link->intf2);
}

node_t*
create_graph_node(graph_t *graph, char *node_name){

    node_t *node = calloc(1,sizeof(node_t));
    strncpy(node->node_name,node_name,NODE_NAME_SIZE);
    node->node_name[NODE_NAME_SIZE-1] = '\0';
    init_udp_socket(node);
    init_glthread(&node->graph_glue);    
    glthread_add_next(&graph->node_list,&node->graph_glue);
    init_node_nw_prop(&node->node_nw_prop);
    return node;
}


graph_t* 
create_new_graph(char *topology_name){

    graph_t *graph = calloc(1, sizeof(graph_t));
    init_glthread(&graph->node_list);
    strncpy(graph->topology_name,topology_name,32);
    graph->topology_name[31]= '\0';
    
    return graph;
}

void
dump_graph(graph_t *graph){

    printf("Topology Name : %s\n",graph->topology_name);
    node_t* node = NULL;
    glthread_t *glthreadptr= NULL;
    ITERATE_GLTHREAD_BEGIN(&graph->node_list,glthreadptr){
        node = graph_glue_to_node(glthreadptr);
        dump_node(node);
    }ITERATE_GLTHREAD_END(&graph->node_list,glthreadptr);
}

void 
dump_node(node_t *node){
    
    printf("Node Name: %s\n", node->node_name);

    for (int i =0;i<MAX_INTF_PER_NODE;i++){
        if(node->intf[i])
            dump_interface(node->intf[i]);
        else 
            break;
    }
}

void 
dump_interface(interface_t *interface){

    node_t *att_node = interface->att_node; 
    node_t *nbr_node = get_nbr_node(interface);
    link_t *link = interface->link; 

    printf("\tInterface Name:  %s\n\tLocal Node: %s,  Nbr Node: %s, Cost = %u\n",
        interface->if_name, att_node->node_name, nbr_node->node_name,link->cost );
}