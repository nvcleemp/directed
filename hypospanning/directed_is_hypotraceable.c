/*
 * 
 * Copyright (C) 2018 Ghent University.
 */

/* This program reads directed graphs in watercluster or digraph6 format from 
 * standard in and determines whether they are hypotraceable.
 * 
 * 
 * Compile with:
 *     
 *     cc -o directed_is_hypotraceable -O4 directed_is_hypotraceable.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "../shared/directed_base.h"
#include "../shared/directed_io.h"

unsigned long long int graph_count = 0;
unsigned long long int filtered_count = 0;
unsigned long long int valid_orientation = 0;

//================ HYPOTRACEABLE ===================

boolean current_path[MAXN+1];

/**
  * 
  */
boolean continue_path(GRAPH graph, DEGREES out, int last, int remaining) {
    int i;
    
    if(remaining==0){
        return TRUE;
    }
    
    for(i = 0; i < out[last]; i++){
        if(!current_path[graph[last][i]]){
            current_path[graph[last][i]]=TRUE;
            if(continue_path(graph, out, graph[last][i], remaining - 1)){
                return TRUE;
            }
            current_path[graph[last][i]]=FALSE;
        }
    }
    
    return FALSE;
}

boolean start_path(GRAPH graph, DEGREES out, int start_vertex, int order){
    int i;
    //mark the start vertex as being in the path
    current_path[start_vertex]=TRUE;
    for(i = 0; i < out[start_vertex]; i++){
        if(!current_path[graph[start_vertex][i]]){
            current_path[graph[start_vertex][i]]=TRUE;
            //search for path containing the edge (v, graph[v][i])
            if(continue_path(graph, out, graph[start_vertex][i], order - 2)){
                return TRUE;
            }
            current_path[graph[start_vertex][i]]=FALSE;
        }
    }
    current_path[start_vertex]=FALSE;
    
    return FALSE;
}

boolean remaining_graph_is_traceable(GRAPH graph, DEGREES out, int remaining_order, int removed_vertex){
    int i;
    
    for(i = 1; i < graph[0][0]; i++){ //we can skip last vertex, since a path needs two end points
        if(i!=removed_vertex){
            //we try to start the path from each vertex
            if(start_path(graph, out, i, remaining_order)){
                return TRUE;
            }
        }
    }
    return FALSE;
}

boolean remaining_graph_is_traceable__has_source(GRAPH graph, DEGREES out, int remaining_order, int removed_vertex, int source){
    //If there is a hamiltonian path, then it has to start from the source
    return start_path(graph, out, source, remaining_order);
}

boolean original_graph_is_traceable(GRAPH graph, DEGREES out, int order){
    int i;
    
    for(i = 1; i <= graph[0][0]; i++){
        //we try to start the path from each vertex
        if(start_path(graph, out, i, order)){
            return TRUE;
        }
    }
    return FALSE;
}

boolean original_graph_is_traceable__has_source(GRAPH graph, DEGREES out, int order, int source){
    //If there is a hamiltonian path, then it has to start from the source
    return start_path(graph, out, source, order);
}

boolean is_hypotraceable(GRAPH graph, DEGREES out, DEGREES in){
    int i, v, source;
    
    int order = graph[0][0];
    
    //check degrees
    boolean has_sink = FALSE;
    boolean has_source = FALSE;
    
    for(i = 1; i <= order; i++){
        if(out[i]==1 || in[i]==1){
            return FALSE;
        }
        if(out[i]==0){
            if(has_sink){
                return FALSE;
            } else {
                has_sink = TRUE;
            }
        }
        if(in[i]==0){
            if(has_source){
                return FALSE;
            } else {
                has_source = TRUE;
                source = i;
            }
        }
    }
    
    valid_orientation++;
    
    //clear possible previous path
    for(i=0; i<=MAXN; i++){
        current_path[i] = FALSE;
    }
    
    if(has_source){
        if(original_graph_is_traceable__has_source(graph, out, order, source)){
            return FALSE;
        }
    } else if(original_graph_is_traceable(graph, out, order)){
        return FALSE;
    }
    
    //just look for a hamiltonian path in all graphs
    if(has_source){
        for(v = 1; v <= order; v++){
            //clear possible previous path
            for(i=0; i<=MAXN; i++){
                current_path[i] = FALSE;
            }

            current_path[v] = TRUE;
            //we mark v as visited, so it is as if it got removed
            if(v!=source){
                if(!remaining_graph_is_traceable__has_source(graph, out, order-1, v, source)){
                    return FALSE;
                }
            } else if(!remaining_graph_is_traceable(graph, out, order-1, v)){
                return FALSE;
            }
        }
        return TRUE;
    } else {
        for(v = 1; v <= order; v++){
            //clear possible previous path
            for(i=0; i<=MAXN; i++){
                current_path[i] = FALSE;
            }

            current_path[v] = TRUE;
            //we mark v as visited, so it is as if it got removed
            if(!remaining_graph_is_traceable(graph, out, order-1, v)){
                return FALSE;
            }
        }
        return TRUE;
    }
    
    
    return TRUE;
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "Checks directed graphs for being hypotraceable.\n\n");
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

        boolean value = is_hypotraceable(graph, out, in);
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
                fprintf(stdout, "Graph %llu is hypotraceable.\n", graph_count);
            } else {
                fprintf(stdout, "Graph %llu is not hypotraceable.\n", graph_count);
            }
        }
    }
    
    fprintf(stderr, "Read %llu graph%s.\n", graph_count, graph_count==1 ? "" : "s");
    fprintf(stderr, "Valid orientation: %llu\n", valid_orientation);
    if(do_filtering){
        fprintf(stderr, "Filtered %llu graph%s.\n", filtered_count, filtered_count==1 ? "" : "s");
    }

    return (EXIT_SUCCESS);
}

