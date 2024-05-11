#include <stdio.h>
#include "Layer2/layer2.h"
#include "tcpconst.h"

void
pkt_dump(ethernet_hdr_t *ethernet_hdr, unsigned int pkt_size){
    if(is_pkt_vlan_tagged(ethernet_hdr)!=NULL){

    }
    else{

    }
}