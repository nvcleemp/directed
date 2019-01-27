/*
 * 
 * Copyright (C) 2019 Ghent University.
 */

/* This program reads hypohamiltonian directed graphs in watercluster or 
 * digraph6 format from standard in and determines whether they contain a
 * suitable 3-cycle.
 * 
 * 
 * Compile with:
 *     
 *     cc -o directed_has_suitable_3_cycle -O4 directed_has_suitable_3_cycle.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "../shared/directed_base.h"
#include "../shared/directed_io.h"

unsigned long long int graph_count = 0;
unsigned long long int filtered_count = 0;

//================ HYPOHAMILTONIAN ===================

boolean current_cycle[MAXN+1];

/**
  * 
  */
boolean continue_cycle(GRAPH graph, DEGREES out, int last, int remaining, int first) {
    int i;
    
    if(remaining==0){
        //TODO: use bitsets (although it appears this does not give a significant gain)
        for(i = 0; i < out[last]; i++){
            if(graph[last][i]==first){
                return TRUE;
            }
        }
        return FALSE;
    }
    
    for(i = 0; i < out[last]; i++){
        if(!current_cycle[graph[last][i]]){
            current_cycle[graph[last][i]]=TRUE;
            if(continue_cycle(graph, out, graph[last][i], remaining - 1, first)){
                return TRUE;
            }
            current_cycle[graph[last][i]]=FALSE;
        }
    }
    
    return FALSE;
}

boolean start_cycle_through_edge(GRAPH graph, DEGREES out, int v1, int v2, int order){
    int i;
    //mark the start vertices as being in the cycle
    current_cycle[v1] = TRUE;
    current_cycle[v2] = TRUE;
    for(i = 0; i < out[v2]; i++){
        if(!current_cycle[graph[v2][i]]){
            current_cycle[graph[v2][i]]=TRUE;
            //search for cycle containing the edge (v, graph[v][i])
            if(continue_cycle(graph, out, graph[v2][i], order - 3, v1)){
                return TRUE;
            }
            current_cycle[graph[v2][i]]=FALSE;
        }
    }
    current_cycle[v2] = FALSE;
    current_cycle[v1] = FALSE;
    
    return FALSE;
}

//directed 3-cycle is x -> y -> z -> x
boolean is_suitable_3_cycle(GRAPH graph, DEGREES out, DEGREES in, int x, int y, int z){
    int v, i;
    for(v = 1; v <= graph[0][0]; v++){
        //clear possible previous path
        for(i=0; i<=MAXN; i++){
            current_cycle[i] = FALSE;
        }
        //mark v as being removed
        current_cycle[v] = TRUE;
        
        //check for hamiltonian cycle through an edge of the 3-cycle
        if(v == x){
            if(!start_cycle_through_edge(graph, out, y, z, graph[0][0]-1)){
                return FALSE;
            }
        } else if(v == y){
            if(!start_cycle_through_edge(graph, out, z, x, graph[0][0]-1)){
                return FALSE;
            }
        } else if(v == z){
            if(!start_cycle_through_edge(graph, out, x, y, graph[0][0]-1)){
                return FALSE;
            }
        } else {
            if(!start_cycle_through_edge(graph, out, x, y, graph[0][0]-1) &&
                    !start_cycle_through_edge(graph, out, y, z, graph[0][0]-1) &&
                    !start_cycle_through_edge(graph, out, z, x, graph[0][0]-1)){
                return FALSE;
            }
        }
    }
    return TRUE;
}

boolean has_suitable_3_cycle(GRAPH graph, DEGREES out, DEGREES in){
    int x, i, j;
    
    int order = graph[0][0];
    
    //check all 3-cycles
    for(x = 1; x <= order; x++){
        for(i = 0; i < out[x]; i++){
            int y = graph[x][i];
            for(j = 0; j < out[y]; j++){
                int z = graph[y][j];
                if(z!=x){
                    int k = 0;
                    while(k < out[z] && graph[z][k]!=x) k++;
                    if(k < out[z]){
                        //found a 3-cycle
                        if(is_suitable_3_cycle(graph, out, in, x, y, z)){
                            return TRUE;
                        }
                    }
                }
            }
        }
    }
    
    return FALSE;
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "Checks directed graphs for having a suitable 3-cycle.\n\n");
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -f, --filter\n");
    fprintf(stderr, "       Filter graphs that have the property.\n");
    fprintf(stderr, "    -i, --invert\n");
    fprintf(stderr, "       Invert the filter.\n");
    fprintf(stderr, "    -u n, --update n\n");
    fprintf(stderr, "       Give an update every n graphs.\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s [options]\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * 
 */
int main(int argc, char** argv) {
    
    GRAPH graph;
    DEGREES out;
    DEGREES in;
    
    boolean do_filtering = FALSE;
    boolean invert = FALSE;
    
    boolean (*read_graph)(FILE *, GRAPH, DEGREES, DEGREES) = read_graph_from_watercluster_file;
    
    int update = 0;
    

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"update", required_argument, NULL, 'u'},
        {"invert", no_argument, NULL, 'i'},
        {"filter", no_argument, NULL, 'f'},
        {"digraph6", no_argument, NULL, 'D'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hfiu:D", long_options, &option_index)) != -1) {
        switch (c) {
            case 'u':
                update = atoi(optarg);
                break;
            case 'i':
                invert = TRUE;
                break;
            case 'f':
                do_filtering = TRUE;
                break;
            case 'D':
                read_graph = read_graph_from_digraph6_file;
                break;
            case 'h':
                help(name);
                return EXIT_SUCCESS;
            case '?':
                usage(name);
                return EXIT_FAILURE;
            default:
                fprintf(stderr, "Illegal option %c.\n", c);
                usage(name);
                return EXIT_FAILURE;
        }
    }
    if(!do_filtering && update){
        fprintf(stderr, "Updates are only available when filtering is enabled.\n");
        update = 0;
    }


    while (read_graph(stdin, graph, out, in)) {
        graph_count++;

        boolean value = has_suitable_3_cycle(graph, out, in);
        if(do_filtering){
            if(invert && !value){
                filtered_count++;
                write_watercluster_format(graph, out, stdout);
            } else if(!invert && value){
                filtered_count++;
                write_watercluster_format(graph, out, stdout);
            }
            if(update && !(graph_count % update)){
                fprintf(stderr, "Read: %llu. Filtered: %llu\n", graph_count, filtered_count);
            }
        } else {
            if(value){
                fprintf(stdout, "Graph %llu has a suitable 3-cycle.\n", graph_count);
            } else {
                fprintf(stdout, "Graph %llu does not have a suitable 3-cycle.\n", graph_count);
            }
        }
    }
    
    fprintf(stderr, "Read %llu graph%s.\n", graph_count, graph_count==1 ? "" : "s");
    if(do_filtering){
        fprintf(stderr, "Filtered %llu graph%s that %scontain a suitable 3-cycle.\n", filtered_count, filtered_count==1 ? "" : "s", invert ? "do not " : "");
    }

    return (EXIT_SUCCESS);
}

