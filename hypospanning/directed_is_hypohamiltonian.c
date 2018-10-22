/*
 * 
 * Copyright (C) 2018 Ghent University.
 */

/* This program reads directed graphs in watercluster or digraph6 format from 
 * standard in and determines whether they are hypohamiltonian.
 * 
 * 
 * Compile with:
 *     
 *     cc -o directed_is_hypohamiltonian -O4 directed_is_hypohamiltonian.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "../shared/directed_base.h"
#include "../shared/directed_io.h"

unsigned long long int graph_count = 0;
unsigned long long int filtered_count = 0;
unsigned long long int valid_orientation_count = 0;

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

boolean start_cycle(GRAPH graph, DEGREES out, int startVertex, int order){
    int i;
    //mark the start vertex as being in the cycle
    current_cycle[startVertex] = TRUE;
    for(i = 0; i < out[startVertex]; i++){
        if(!current_cycle[graph[startVertex][i]]){
            current_cycle[graph[startVertex][i]]=TRUE;
            //search for cycle containing the edge (v, graph[v][i])
            if(continue_cycle(graph, out, graph[startVertex][i], order - 2, startVertex)){
                return TRUE;
            }
            current_cycle[graph[startVertex][i]]=FALSE;
        }
    }
    current_cycle[startVertex] = FALSE;
    
    return FALSE;
}

boolean vertex_deleted_graph_is_hamiltonian(GRAPH graph, DEGREES out, int remaining_order, int removed_vertex){
    if(removed_vertex==1){
        if(start_cycle(graph, out, 2, remaining_order)){
            return TRUE;
        }
    } else if(start_cycle(graph, out, 1, remaining_order)){
        return TRUE;
    }
    
    return FALSE;
}

boolean original_graph_is_hamiltonian(GRAPH graph, DEGREES out, int order){
    return start_cycle(graph, out, 1, order);
}

boolean is_hypohamiltonian(GRAPH graph, DEGREES out, DEGREES in){
    int i, v;
    
    int order = graph[0][0];
    
    //check degrees
    for(i = 1; i <= order; i++){
        if(out[i]<=1 || in[i]<=1){
            return FALSE;
        }
    }
    
    valid_orientation_count++;
    
    //clear possible previous cycle
    for(i=0; i<=MAXN; i++){
        current_cycle[i] = FALSE;
    }
    
    if(original_graph_is_hamiltonian(graph, out, order)){
        return FALSE;
    }
    
    //just look for a hamiltonian cycle in all vertex-deleted graphs
    for(v = 1; v <= order; v++){
        //clear possible previous cycle
        for(i=0; i<=MAXN; i++){
            current_cycle[i] = FALSE;
        }

        current_cycle[v] = TRUE;
        //we mark v as visited, so it is as if it got removed
        if(!vertex_deleted_graph_is_hamiltonian(graph, out, order-1, v)){
            return FALSE;
        }
    }
    
    return TRUE;
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "Checks directed graphs for being hypohamiltonian.\n\n");
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile with a larger\n", MAXN);
    fprintf(stderr, "value for MAXN if you need to handle larger graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -f, --filter\n");
    fprintf(stderr, "       Filter graphs that have the property.\n");
    fprintf(stderr, "    -i, --invert\n");
    fprintf(stderr, "       Invert the filter.\n");
    fprintf(stderr, "    -u n, --update n\n");
    fprintf(stderr, "       Give an update every n graphs.\n");
    fprintf(stderr, "    -D, --digraph6\n");
    fprintf(stderr, "       Reads graphs in digraph6 format instead of watercluster format.\n");
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

        boolean value = is_hypohamiltonian(graph, out, in);
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
                fprintf(stdout, "Graph %llu is hypohamiltonian.\n", graph_count);
            } else {
                fprintf(stdout, "Graph %llu is not hypohamiltonian.\n", graph_count);
            }
        }
    }
    
    fprintf(stderr, "Read %llu graph%s.\n", graph_count, graph_count==1 ? "" : "s");
    fprintf(stderr, "Valid orientation: %llu\n", valid_orientation_count);
    if(do_filtering){
        fprintf(stderr, "Filtered %llu graph%s.\n", filtered_count, filtered_count==1 ? "" : "s");
    }

    return (EXIT_SUCCESS);
}

