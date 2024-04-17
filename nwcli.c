#include "CommandParser/libcli.h"
#include "CommandParser/cmdtlv.h"
#include "cmdcodes.h"
#include "graph.h"
#include <stdio.h>
#include "net.h"

extern graph_t *topo;

static int
validate_node_name (char *value){
    assert(strnlen(value,NODE_NAME_SIZE)<=NODE_NAME_SIZE);
    node_t *node = get_node_by_node_name(topo,value);
    if(node!=NULL)
        return VALIDATION_SUCCESS;
    else 
        return VALIDATION_FAILED;
}

static int
arp_handler (param_t *param, ser_buff_t *tlv_buf,
    op_mode enable_or_disable){
        int cmd_code = -1;
        cmd_code = EXTRACT_CMD_CODE(tlv_buf);
	//printf("cmd_code = %d\n", cmd_code);
        char *ip_addr = NULL;
        char *node_name = NULL;
        tlv_struct_t *tlvptr = NULL ; 

        TLV_LOOP_BEGIN(tlv_buf, tlvptr){
            if(strncmp(tlvptr->leaf_id, "ip-address", strlen("ip-address"))==0){
                ip_addr = tlvptr->value;
            }
            else if(strncmp(tlvptr->leaf_id, "node-name", strlen("node-name"))==0){
                node_name = tlvptr->value;
            }
        }TLV_LOOP_END;
        printf("Node Name : %s, IP Address : %s", node_name,ip_addr);
        return 0;

}

static int
show_nw_topology_handler(param_t *param, ser_buff_t *tlv_buf, 
    op_mode enable_or_disable){
        int cmd_code= -1;
        cmd_code = EXTRACT_CMD_CODE(tlv_buf);
        //printf("cmd_code = %d", cmd_code);
        switch (cmd_code) {
            case CMDCODE_SHOW_NW_TOPOLOGY: 
                dump_nw_graph(topo);
                break;
            default:
                ;
        }

        return 0;
}

void
nw_init_cli(){
    init_libcli();
    param_t *show = libcli_get_show_hook();
    param_t *debug = libcli_get_debug_hook();
    param_t *config = libcli_get_config_hook();
    param_t *run = libcli_get_run_hook();
    param_t *debug_show = libcli_get_debug_show_hook();
    param_t *root = libcli_get_root();

    {
        /*show topology*/
        static param_t topology;
        init_param(&topology, CMD, "topology", show_nw_topology_handler, 0, INVALID,
        0, "Dump Complete Network Topology");
        libcli_register_param(show,&topology);
        set_param_cmd_code(&topology,CMDCODE_SHOW_NW_TOPOLOGY);
        //{
            //static param_t ;
            //init_param(&)

        //}

    }

    {
        static param_t node;
        init_param(&node, CMD, "node",0,0,INVALID,0,"Run Instructions on a Node");
        libcli_register_param(run,&node);

        {
            static param_t node_name;
            init_param(&node_name, LEAF,0, 0, validate_node_name,STRING, "node-name","Node Name Present in Topology");
            libcli_register_param(&node, &node_name);

            {
                static param_t resolve_arp;
                init_param(&resolve_arp, CMD, "resolve-arp",0,0,INVALID,0,"Reslove Address Using ARP");
                libcli_register_param(&node_name,&resolve_arp);

                {
                    static param_t ip_addr;
                    init_param(&ip_addr, LEAF, 0, arp_handler, 0,IPV4,"ip-address","Run Resolve ARP on IP Address");
                    libcli_register_param(&resolve_arp,&ip_addr);
                    set_param_cmd_code(&ip_addr, CMDCODE_RUN_ARP_RESOLUTION_IP_ADDR);
                }
            }
        }
    }
    support_cmd_negation(config);
}
