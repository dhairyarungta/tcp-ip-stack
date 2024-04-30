#include "layer2.h"
#include <stdlib.h>
#include "../tcpconst.h"

void 
layer2_frame_recv(node_t *node, interface_t *interface, 
    char *pkt, unsigned int pkt_size){
    /*Entry point into the TCP/IP stack after the physical layer*/
    ;
}

void 
init_arp_table(arp_table_t** arp_table){
    *arp_table = calloc(1, sizeof(arp_table_t));
    init_glthread(&((*arp_table)->arp_entries));
}

arp_entry_t *
arp_table_lookup(arp_table_t *arp_table, char *ip_addr){
    glthread_t *glthreadptr = NULL;
    arp_entry_t *arp_entry = NULL;
    ITERATE_GLTHREAD_BEGIN(&(arp_table->arp_entries), glthreadptr){
        arp_entry = arp_glue_to_arp(glthreadptr);
        if(strncmp(arp_entry->ip_addr.ip_addr,ip_addr,16)==0){
           return arp_entry; 
        }
    }ITERATE_GLTHREAD_END(&(arp_table->arp_entries), glthreadptr);
    return NULL;
}

bool_t 
arp_table_entry_add(arp_table_t *arp_table, arp_entry_t *arp_entry){

    arp_entry_t * arp_entry_old = arp_table_lookup(arp_table, arp_entry->ip_addr.ip_addr);
    
    if(arp_entry_old && memcmp(arp_entry_old, arp_entry, sizeof(arp_entry_t))==0){
        return FALSE;
    }
    
    if(arp_entry_old){
        delete_arp_table_entry(arp_table, arp_entry->ip_addr.ip_addr);
    }

    init_glthread(&arp_entry->arp_glue);
    glthread_add_next(&arp_table->arp_entries, &arp_entry->arp_glue);
    return TRUE;
}

void
arp_table_update_from_arp_reply(arp_table_t *arp_table, 
    arp_hdr_t *arp_hdr, interface_t *iif){

    unsigned int src_ip = 0;
    assert(arp_hdr->op_code == ARP_REPLY);
    arp_entry_t *arp_entry = calloc(1, sizeof(arp_entry_t));
    src_ip = htonl(arp_hdr->src_ip);
    arp_entry->ip_addr.ip_addr[15] = '\0';
    memcpy(arp_entry->mac_addr.mac,arp_hdr->src_mac.mac,sizeof(mac_add_t));
    strncpy(arp_entry->oif_name, iif->if_name, IF_NAME_SIZE);

    bool_t rc = arp_table_entry_add(arp_table, arp_entry);
    if(rc == FALSE){
        free(arp_entry);
    }

}

void 
delete_arp_table_entry(arp_table_t *arp_table, char *ip_addr){

    arp_entry_t *arp_entry = arp_table_lookup(arp_table,ip_addr);
    if(arp_entry==NULL)
        return;

    remove_glthread(&(arp_entry->arp_glue));
    free(arp_entry);
}

void
dump_arp_table(arp_table_t *arp_table){
    ;
}

void 
send_arp_broadcast_request(node_t *node, interface_t *oif, char *ip_addr){
    ;
}