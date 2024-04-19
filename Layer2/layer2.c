#include "layer2.h"
#include <stdlib.h>

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



}

void
arp_table_update_from_arp_reply(arp_table_t *arp_table, arp_hdr_t *arp_hdr, interface_t *iif);//incoming intf

void 
delete_arp_table_entry(arp_table_t *arp_table, char *ip_addr){
    arp_entry_t *arp_entry = arp_table_lookup(arp_table,ip_addr);
    if(arp_entry==NULL)
        return;

    remove_glthread(&(arp_entry->arp_glue));
    free(arp_entry);

}

void
dump_arp_table(arp_table_t *arp_table);

void 
send_arp_broadcast_request(node_t *node, interface_t *oif, char *ip_addr){
    ;
}