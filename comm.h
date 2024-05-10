#define MAX_PACKET_BUFFER_SIZE 2048
#include "graph.h"

int
send_pkt_out(char *pkt, unsigned int pkt_size, interface_t *interface);

void
init_udp_socket(node_t *node);

void 
network_start_pkt_receiver_thread(graph_t *topo);

int 
pkt_receive(node_t *node, interface_t *interface, char *pkt, unsigned int pkt_size);

int 
send_pkt_flood(node_t *node, interface_t *exempted_intf, char *pkt, unsigned int pkt_size);

int
send_pkt_flood_l2_intf_only(node_t *node, interface_t *exempted_intf,
    char *pkt, unsigned int pkt_size);