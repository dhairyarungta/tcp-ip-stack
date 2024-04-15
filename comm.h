#define MAX_PACKET_BUFFER_SIZE 2048
#include "graph.h"

int
send_pkt_out(char *pkt, unsigned int pkt_size, interface_t *interface);


void
init_udp_socket(node_t *node);
