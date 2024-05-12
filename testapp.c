#include "graph.h"
#include <unistd.h>
#include "CommandParser/libcli.h"
#include "net.h"
#include "comm.h"
#include <stdio.h>

#include "Layer2/layer2.h"
#include <stdlib.h>

extern graph_t * build_first_topo();
extern graph_t * build_simple_l2_switch_topo();
extern graph_t * build_dualswitch_topo();
extern void nw_init_cli();

graph_t *topo = NULL;

int 
main(int argc, char** argv){

    nw_init_cli();
    topo = build_dualswitch_topo();
    start_shell();
    return 0;    

    //char ip_addr[16];
    //unsigned int ip_uint = ;
    //ip_addr_n_to_p(ip_uint, ip_addr);
    //printf("%s\n",ip_addr);


    //printf("%u\n",ip_addr_p_to_n("127.0.0.1")); 
    //printf("%u\n",ip_addr_p_to_n("255.255.255.255")); 
    //nw_init_cli();
    //topo = build_first_topo();


    //printf("%d\n",(int)ETH_HDR_SIZE_EXCL_PAYLOAD);
    //printf("%d\n",(int)ETH_HDR_SIZE_EXCL_PAYLOAD1);

    //ethernet_hdr_t *eth_header = calloc(1,sizeof(ethernet_hdr_t));
    //eth_header->FCS =(unsigned int) 19;
    //printf("%u\n",(unsigned int)ETH_FCS(eth_header,sizeof(((ethernet_hdr_t*)eth_header)->payload )));
    //printf("%u\n",(unsigned int)ETH_FCS1(eth_header, sizeof(((ethernet_hdr_t*)eth_header)->payload )));
    //printf ("%u\n",eth_header->FCS);

    //sleep(2);
    //node_t *snode = get_node_by_node_name(topo, "R0_re");
    //interface_t *oif= get_node_if_by_name(snode,"eth0/0");
    //char msg[] = "Hello, how are you\0" ;   
    ////printf("%d \n",strnlen(msg));
    //send_pkt_out(msg, strlen(msg),oif);
    //start_shell();
}
