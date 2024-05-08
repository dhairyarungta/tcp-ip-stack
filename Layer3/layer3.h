#ifndef __LAYER3__
#define __LAYER3__

#include "../net.h"
#include "../graph.h"

void 
promote_pkt_to_layer3(node_t *node, interface_t *interface, 
    char *pkt, unsigned int pkt_size);



#endif