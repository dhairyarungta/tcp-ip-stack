#include "utils.h"
#include <string.h>
#include <arpa/inet.h> 

/*Returns mask applied to prefix, in the array pointed by str_prefix*/
void
apply_mask(char *prefix, char mask, char *str_prefix){

    uint32_t subnet_mask = 0xFFFFFFFF;
    uint32_t binary_prefix = 0;
    uint32_t mask_uint = (uint32_t)mask;

    if(mask_uint == 32){
        strncpy(str_prefix, prefix, 16);
        str_prefix[15] = '\0';
        return;
    }
    /*ntohl and htonl both have no way of knowing if the input to them is big or little
     *endian, they just know that the network byte order big endian and the byte order of
     *the host machine, if both are different then both functions just reverse the arguments 
     * irrespective of the arguments endianess*/

    inet_pton(AF_INET, prefix, &binary_prefix);
    //this puts binary_prefix in big endian form

    binary_prefix = htonl(binary_prefix);
    //converting binary prefix from big endian to little endian form

    subnet_mask =subnet_mask << (32-mask_uint);
    binary_prefix = subnet_mask & binary_prefix ;

    binary_prefix = htonl(binary_prefix);
    //converting binary_prefix from little endian to big endian

    //inet_ntop expects binary_prefix to be big endian 
    //endianess doens't matter for str_prefix, prefix or any other char, or char strings
    inet_ntop(AF_INET, &binary_prefix, str_prefix, 16);
    str_prefix[15] = '\0';
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
    ip_addr = htonl(ip_addr);

    /*inet_ntop requires input ip_addr in network byte order*/
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


