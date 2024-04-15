#include "cmdtlv.h"
#include "libcli.h"

#define CMDODE_SHOW_NODE 1
#define CMDODE_SHOW_NODE_LOOPBACK 2
#define CMDODE_CONFIG_NODE_LOOPBACK 3
int validate_node_name(char *value){
    printf("%s() is called .. \n",__FUNCTION__);
    return VALIDATION_SUCCESS;
}

int validate_loopback_address(char *value){
    printf("%s() is called ..\n",__FUNCTION__);
    return VALIDATION_SUCCESS;
}

int node_callback_handler(param_t *param, ser_buff_t *tlv_buf, op_mode enable_or_disable){
    printf("%s() is called .. \n",__FUNCTION__);
    return 0;
}

int node_loopback_call_handler(param_t *param, ser_buff_t *tlv_buf, op_mode enable_or_disable){

    printf("%s() is called ..\n",__FUNCTION__);
    unsigned int CMD_CODE = EXTRACT_CMD_CODE(tlv_buf);
    printf("CMD_CODE = %d\n",CMD_CODE);

    char *node_name = NULL;
    char *lo_address = NULL;
    tlv_struct_t *tlv = NULL;

    TLV_LOOP_BEGIN(tlv_buf, tlv){
        if(strncmp(tlv->leaf_id,"node_name",strlen("node_name"))==0){
            node_name = tlv->value;
        }
        else if(strncmp(tlv->leaf_id,"lo-address" ,strlen("lo-address" ))==0){
            lo_address = tlv->value;
        }

    }TLV_LOOP_END;

    switch (CMD_CODE) {
        case CMDODE_SHOW_NODE :
            printf("node_name = %s. lo-address = NULL", node_name);
            break;
        case CMDODE_SHOW_NODE_LOOPBACK:
            printf( "node_name = %s. lo-address = %s\n", node_name,lo_address);
            break;
        case CMDODE_CONFIG_NODE_LOOPBACK: 
            printf("node name = %s. lo-address = %s , op_code = %s\n",node_name,lo_address,
            enable_or_disable == CONFIG_ENABLE ?"CONFIG_ENABLE":"CONFIG_DISABLE");
            break;
        default:
            ;
    }
    return 0;
}


int
main(int argc, char **argv){

    init_libcli();
    param_t *show   = libcli_get_show_hook();
    param_t *debug  = libcli_get_debug_hook();
    param_t *config = libcli_get_config_hook();
    param_t *clear  = libcli_get_clear_hook();
    param_t *run    = libcli_get_run_hook();

    /*Impelementing CM1 : show node <node_name>*/
    {
        /*show node*/
        static param_t node;
        init_param(&node,
        CMD,
        "node",
        0,
        0,
        INVALID,
        0,
        "Help : node");
        libcli_register_param(show, &node);
        {
            static param_t node_name;
            init_param(&node_name,
            LEAF,
            0,
            node_loopback_call_handler,
            validate_node_name,
            STRING,
            "node_name",
            "Help : Node-name");
            libcli_register_param(&node, &node_name);
            set_param_cmd_code(&node_name, CMDODE_SHOW_NODE);
            {

                static param_t loopback;
                init_param(&loopback,
                CMD,
                "loopback",
                0,
                0,
                INVALID,
                0,
                "Help : loopback");
                libcli_register_param(&node_name, &loopback);
                {
                    static param_t loopback_address;
                    init_param(&loopback_address,
                    LEAF,
                    0,
                    node_loopback_call_handler,
                    validate_loopback_address,
                    IPV4,
                    "lo-address",
                    "Help : Node's loopback address");
                    libcli_register_param(&loopback,&loopback_address);
                   set_param_cmd_code(&loopback_address, CMDODE_SHOW_NODE_LOOPBACK);
                }
            }
        }
    }
    
    {
        /*config node*/
        static param_t node ;

        init_param(&node,
        CMD,
        "node",
        0,
        0,
        INVALID,
        0,
        "Help : node");

        libcli_register_param(config,&node);
        {
            static param_t node_name;
            init_param(&node_name,
            LEAF,
            0,
            0,
            validate_node_name,
            STRING,
            "node-name",
            "Help : Node name");
            
            libcli_register_param(&node,&node_name);
            {
                static param_t loopback;
                init_param(&loopback,
                CMD,
                "loopback", 
                0,
                0,
                INVALID,
                0,
                "Help : node");

                libcli_register_param(&node_name,&loopback);

                {
                    static param_t loopback_address;
                    init_param(&loopback_address,
                    LEAF,
                    0,
                    node_loopback_call_handler,
                    validate_loopback_address,
                    IPV4,
                    "lo_address",
                    "Help : Node's loopback address");
                    libcli_register_param(&loopback,&loopback_address);

                    set_param_cmd_code(&loopback_address,CMDODE_CONFIG_NODE_LOOPBACK);

                }
            }
        }


        
    }
    support_cmd_negation(config);
    /*Do not add any param in config command tree after above line*/
    start_shell();
    return 0;
}

