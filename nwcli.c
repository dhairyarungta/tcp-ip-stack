#include "CommandParser/clistd.h"
#include "CommandParser/libcli.h"
#include "CommandParser/cmdtlv.h"
#include "CommandParser/libcliid.h"
#include "cmdcodes.h"
#include "graph.h"
#include <stdio.h>

extern graph_t *topo;

static int
show_nw_topology_handler(param_t *param, ser_buff_t *tlv_buf, 
    op_mode enable_or_disable){

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
        init_param();
        libcli_register_param(show,&topology);
        set_param_cmd_code()
    }

}