/*
 * 
 * Copyright (C) 2019 Ghent University.
 */

/* This program reads directed graphs in watercluster or digraph6 format from 
 * standard in and finds all vertex-deleted hamiltonian cycles.
 * 
 * 
 * Compile with:
 *     
 *     cc -o directed_all_vertex_deleted_hamiltonian_cycles -O4 directed_all_vertex_deleted_hamiltonian_cycles.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "../shared/directed_base.h"
#include "../shared/directed_io.h"

unsigned long long int graph_count = 0;

//================ VERTEX-DELETED HAMILTONIAN CYCLES ===================

boolean current_cycle[MAXN+1];
int cycle_vertices[MAXN+1];
int cycle_length;
int current_cycle_count;
int cycle_count[MAXN+1];

void output_cycle(){
    int i;
    
    current_cycle_count++;
    for(i = 0; i < cycle_length; i++){
        fprintf(stdout, "%d ", cycle_vertices[i]);
    }
    fprintf(stdout, "\n");
}

/**
  * 
  */
void continue_cycle(GRAPH graph, DEGREES out, int last, int remaining, int first) {
    int i;
    
    cycle_vertices[cycle_length] = last;
    cycle_length++;
    
    if(remaining==0){
        for(i = 0; i < out[last]; i++){
            if(graph[last][i]==first){
                output_cycle();
                break;
            }
        }
    } else {
        for(i = 0; i < out[last]; i++){
            if(!current_cycle[graph[last][i]]){
                current_cycle[graph[last][i]]=TRUE;
                continue_cycle(graph, out, graph[last][i], remaining - 1, first);
                current_cycle[graph[last][i]]=FALSE;
            }
        }
    }
    cycle_length--;
}

void start_cycle(GRAPH graph, DEGREES out, int start_vertex, int order){
    int i;
    //mark the start vertex as being in the cycle
    current_cycle[start_vertex] = TRUE;
    cycle_vertices[0] = start_vertex;
    cycle_length = 1;
    
    for(i = 0; i < out[start_vertex]; i++){
        if(!current_cycle[graph[start_vertex][i]]){
            current_cycle[graph[start_vertex][i]]=TRUE;
            //search for cycle containing the edge (v, graph[v][i])
            continue_cycle(graph, out, graph[start_vertex][i], order - 2, start_vertex);
            current_cycle[graph[start_vertex][i]]=FALSE;
        }
    }
    current_cycle[start_vertex] = FALSE;
}

void vertex_deleted_graph_find_all_hamiltonian_cycles(GRAPH graph, DEGREES out, int remaining_order, int removed_vertex){
    if(removed_vertex==1){
        start_cycle(graph, out, 2, remaining_order);
    } else {
        start_cycle(graph, out, 1, remaining_order);
    }
}

void find_all_vertex_deleted_hamiltonian_cycles(GRAPH graph, DEGREES out, DEGREES in){
    int i, v;
    
    int order = graph[0][0];
    
    //clear possible previous cycle
    for(i=0; i<=MAXN; i++){
        current_cycle[i] = FALSE;
        current_cycle_count = 0;
    }
    
    //just look for all hamiltonian cycles in each vertex-deleted graph
    for(v = 1; v <= order; v++){
        //clear possible previous cycle
        for(i=0; i<=MAXN; i++){
            current_cycle[i] = FALSE;
        }

        current_cycle[v] = TRUE;
        //we mark v as visited, so it is as if it got removed
        current_cycle_count = 0;
        vertex_deleted_graph_find_all_hamiltonian_cycles(graph, out, order-1, v);
        cycle_count[v] = current_cycle_count;
    }
    fprintf(stdout, "\n");
    
    for(v = 1; v <= order; v++){
        fprintf(stderr, "G-%d: %d cycle%s\n", v, cycle_count[v], cycle_count[v] == 1 ? "" : "s");
    }
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "Finds all vertex-deleted hamiltonian cycles.\n\n");
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile with a larger\n", MAXN);
    fprintf(stderr, "value for MAXN if you need to handle larger graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
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
    
    boolean (*read_graph)(FILE *, GRAPH, DEGREES, DEGREES) = read_graph_from_watercluster_file;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"digraph6", no_argument, NULL, 'D'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hD", long_options, &option_index)) != -1) {
        switch (c) {
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

    while (read_graph(stdin, graph, out, in)) {
        graph_count++;

        find_all_vertex_deleted_hamiltonian_cycles(graph, out, in);
    }
    
    fprintf(stderr, "Read %llu graph%s.\n", graph_count, graph_count==1 ? "" : "s");

    return (EXIT_SUCCESS);
}

