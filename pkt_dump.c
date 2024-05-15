#include <stdio.h>
#include "Layer2/layer2.h"
#include "tcpconst.h"

static void
print_mac_address(char *mac){

    printf("MAC : %u:%u:%u:%u:%u:%u\n", 
        mac[0], mac[1],mac[2],mac[3],mac[4],mac[5]);
}

static void
print_arp_hdr(char *payload){

    arp_hdr_t *arp_hdr = (arp_hdr_t *)payload;
    printf("ARP -> Hw Type : %hd, Proto Type : %hd, Op Code : %hd\n",
        arp_hdr->hw_type, arp_hdr->proto_type, arp_hdr->op_code);

    printf("Arp Src "); print_mac_address(arp_hdr->src_mac.mac);
    printf("Arp Dst "); print_mac_address(arp_hdr->dst_mac.mac);

    /*In implementation of this project, the dst_ip and src_ip are
    stored in host byte order in the eth packets for Little endian systems*/
    printf("Src IP : %u, Dst IP : %u\n", arp_hdr->src_ip, arp_hdr->dst_ip);
}

static void
print_payload_and_type(unsigned int type, char *payload){

    printf("Eth Hdr Type : %u\n",type);
    switch(type){
        case ARP_MSG :
            print_arp_hdr(payload);
    }
}

void
pkt_dump(ethernet_hdr_t *ethernet_hdr, unsigned int pkt_size){

    printf("Printing Pkt Contents\n");
    unsigned int payload_size = 0;
    if(is_pkt_vlan_tagged(ethernet_hdr)!=NULL){
        vlan_ethernet_hdr_t *vlan_ethernet_hdr = (vlan_ethernet_hdr_t *)ethernet_hdr;
        payload_size = pkt_size-VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD;

        printf("Dst ");
        print_mac_address(vlan_ethernet_hdr->dst_mac.mac);

        printf("Src ");
        print_mac_address(vlan_ethernet_hdr->src_mac.mac);

        printf("VLAN Id : %hd\n",vlan_ethernet_hdr->vlan_8021q_hdr.tci_vid);
        print_payload_and_type(vlan_ethernet_hdr->type, vlan_ethernet_hdr->payload);
    }
    else{
        payload_size = pkt_size-ETH_HDR_SIZE_EXCL_PAYLOAD;

        printf("Dst ");
        print_mac_address(ethernet_hdr->dst_mac.mac);

        printf("Src ");
        print_mac_address(ethernet_hdr->src_mac.mac);

        print_payload_and_type(ethernet_hdr->type, ethernet_hdr->payload);
    }
        printf("FCS : %u\n\n",GET_COMMON_ETH_FCS(ethernet_hdr, payload_size));
}