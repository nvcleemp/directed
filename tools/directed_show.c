/*
 * 
 * Copyright (C) 2019 Ghent University.
 */

/* This program reads directed graphs in watercluster or digraph6 format from 
 * standard in and prints the adjacency lists.
 * 
 * 
 * Compile with:
 *     
 *     cc -o directed_show -O4 directed_show.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "../shared/directed_base.h"
#include "../shared/directed_io.h"

unsigned long long int graph_count = 0;

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "Prints adjacency lists for directed graphs.\n\n");
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
        fprintf(stdout, "Graph %llu:\n", graph_count);
        int order = graph[0][0];
        int v, i;
        for(v = 1; v <= order; v++){
            fprintf(stdout, "%d: ", v);
            if(out[v] > 0){
                fprintf(stdout, "%d", graph[v][0]);
                for(i = 1; i < out[v]; i++){
                    fprintf(stdout, ", %d", graph[v][i]);
                }
            }
            fprintf(stdout, "\n");
        }
        fprintf(stdout, "\n");
    }
    
    fprintf(stderr, "Read %llu graph%s.\n", graph_count, graph_count==1 ? "" : "s");

    return (EXIT_SUCCESS);
}

