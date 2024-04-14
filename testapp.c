#include "graph.h"

extern graph_t *build_first_topo();

graph_t *topo;

int 
main(int argc, char** argv){
    topo = build_first_topo();
    dump_graph(topo);
    return 0;    
}