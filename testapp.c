#include "graph.h"
#include <unistd.h>
#include "CommandParser/libcli.h"
#include "net.h"
#include "comm.h"
#include <stdio.h>

extern graph_t *build_first_topo();
extern void nw_init_cli();

graph_t *topo = NULL;

int 
main(int argc, char** argv){
    nw_init_cli();
    topo = build_first_topo();

    sleep(2);
    node_t *snode = get_node_by_node_name(topo, "R0_re");
    interface_t *oif= get_node_if_by_name(snode,"eth0/0");
    char msg[] = "Hello, how are you\0" ;   
    //printf("%d \n",strnlen(msg));
    send_pkt_out(msg, strlen(msg),oif);
    start_shell();
    return 0;    
}