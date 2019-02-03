/*
 * 
 * Copyright (C) 2019 Ghent University.
 */

/* This program reads two directed graphs in watercluster or digraph6 format from 
 * standard in or from a file and performs the specified transformation.
 * 
 * 
 * Compile with:
 *     
 *     cc -o directed_transform -O4 directed_transform.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "../shared/directed_base.h"
#include "../shared/directed_io.h"

void flip_all_arcs(GRAPH graph, DEGREES out, DEGREES in, GRAPH graph_result, DEGREES out_result, DEGREES in_result){
    int i, j;
    
    prepare_graph(graph_result, out_result, in_result, graph[0][0]);
    
    for(i=1; i<=graph[0][0]; i++){
        for(j=0; j<out[i]; j++){
            add_arc(graph_result, out_result, in_result, graph[i][j], i);
        }
    }
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "Performs the specified transfromation.\n\n");
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s transformation [options]\n\n", name);
    fprintf(stderr, "Possible transformations are:");
    fprintf(stderr, "  * flip: flip all directions of all arcs");
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile with a larger\n", MAXN);
    fprintf(stderr, "value for MAXN if you need to handle larger graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -D, --digraph6\n");
    fprintf(stderr, "       Reads graphs in digraph6 format instead of watercluster format.\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "Usage: %s transformation [options]\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * 
 */
int main(int argc, char** argv) {
    
    boolean (*read_graph)(FILE *, GRAPH, DEGREES, DEGREES) = read_graph_from_watercluster_file;
    void (*transform_graph)(GRAPH, DEGREES, DEGREES, GRAPH, DEGREES, DEGREES) = NULL;
    

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    if(argc<2){
        fprintf(stderr, "No transformation specified -- exiting!\n");
        exit(EXIT_FAILURE);
    }
    char *transformation = argv[1];
    if(strcmp(transformation, "flip")){
        transform_graph = flip_all_arcs;
    } else {
        fprintf(stderr, "Unknown transformation: %s -- exiting!\n", transformation);
        exit(EXIT_FAILURE);
    }
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
    
    GRAPH graph;        DEGREES out;        DEGREES in;
    GRAPH graph_result; DEGREES out_result; DEGREES in_result;

    while(read_graph(stdin, graph, out, in)){
        transform_graph(graph, out, in, graph_result, out_result, in_result);
    
        write_watercluster_format(graph_result, out_result, stdout);
    }
    
    return (EXIT_SUCCESS);
}

