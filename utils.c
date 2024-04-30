#include "utils.h"
#include <string.h>
#include <arpa/inet.h> 
void
apply_mask(char *prefix, char mask, char *str_prefix){
    
}

void
layer2_fill_with_broadcast_mac(char *mac_array){
    for (int i=0;i< 6;i++){
        mac_array[i] = 0xFF;
    }
}

char *
tcp_ip_convert_ip_n_to_p(uint32_t ip_addr,
        char *output_buffer){
    
    char *out = NULL;
    static char str_ip[16];
    out = !output_buffer ? str_ip : output_buffer;
    memset(out, 0, 16);
    ip_addr = ntohl(ip_addr);
    inet_ntop(AF_INET, &ip_addr, out, 16);
    out[15] = '\0';
    return out;
}

uint32_t
tcp_ip_convert_ip_p_to_n(char *ip_addr){
    uint32_t binary_prefix = 0;    
    inet_pton(AF_INET, ip_addr, &binary_prefix);
    binary_prefix = htonl(binary_prefix);
    return binary_prefix;
}


