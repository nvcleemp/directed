/*
 * 
 * Copyright (C) 2019 Ghent University.
 */

/* This program reads hypohamiltonian directed graphs in watercluster or 
 * digraph6 format from standard in and finds all copies of Z6 in them.
 * 
 * 
 * Compile with:
 *     
 *     cc -o directed_all_z6 -O4 directed_all_z6.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "../shared/directed_base.h"
#include "../shared/directed_io.h"

unsigned long long int graph_count = 0;

boolean quartic = FALSE;
boolean non_quartic = FALSE;

boolean are_all_different(int v1, int v2, int v3, int v4, int v5, int v6){
    return v1!=v2 && v1!=v3 && v1!=v4 && v1!=v5 && v1!=v6 &&
            v2!=v3 && v2!=v4 && v2!=v5 && v2!=v6 &&
            v3!=v4 && v3!=v5 && v3!=v6 &&
            v4!=v5 && v4!=v6 &&
            v5!=v6;
}

inline boolean satisfies_conditions_single_vertex(GRAPH graph, DEGREES out, DEGREES in, int v){
    return !quartic || (out[v]==2 && in[v]==2);
}

inline boolean satisfies_conditions_all_vertices(GRAPH graph, DEGREES out, DEGREES in, int x, int y, int z){
    return !non_quartic || (out[x] + out[y] + out[z] + in[x] + in[y] + in[z] > 12);
}

//directed 3-cycle is x -> y -> z -> x
void check_z6(GRAPH graph, DEGREES out, DEGREES in, int x, int y, int z){
    int i, j, k;
    for(i=0; i<out[y]; i++){
        if(has_arc(graph, out, graph[y][i], x)){
            for(j=0; j<out[z]; j++){
                if(has_arc(graph, out, graph[z][j], y)){
                    for(k=0; k<out[x]; k++){
                        if(has_arc(graph, out, graph[x][k], z)){
                            if(are_all_different(x,y,z,graph[x][k],graph[y][i],graph[z][j])){
                                fprintf(stdout, "%d, %d, %d - %d, %d, %d\n", x, y, z, graph[x][k],graph[y][i],graph[z][j]);
                            }
                        }
                    }
                }
            }
        }
    }
}

void find_z6(GRAPH graph, DEGREES out, DEGREES in){
    int x, i, j;
    
    int order = graph[0][0];
    
    //check all 3-cycles (we impose that x is the smallest of the three vertices)
    for(x = 1; x <= order; x++){
        if(satisfies_conditions_single_vertex(graph, out, in, x)){
            for(i = 0; i < out[x]; i++){
                int y = graph[x][i];
                if(y < x) continue;
                if(satisfies_conditions_single_vertex(graph, out, in, y)){
                    for(j = 0; j < out[y]; j++){
                        int z = graph[y][j];
                        if(z > x){
                            if(satisfies_conditions_single_vertex(graph, out, in, z) && 
                                    satisfies_conditions_all_vertices(graph, out, in, x, y, z)){
                                int k = 0;
                                while(k < out[z] && graph[z][k]!=x) k++;
                                if(k < out[z]){
                                    //found a 3-cycle
                                    check_z6(graph, out, in, x, y, z);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "Finds all copies of Z6 in directed graphs.\n\n");
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -4, --quartic\n");
    fprintf(stderr, "       Only check for a copy of Z6 with all indegrees and outdegrees equal to 2.\n");
    fprintf(stderr, "    -N, --non-quartic\n");
    fprintf(stderr, "       Only check for a copy of Z6 with at least one indegree or outdegree larger than 2.\n");
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
    boolean graph_number_header = TRUE;
    
    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"no-header", no_argument, NULL, 0},
        {"quartic", no_argument, NULL, '4'},
        {"non-quartic", no_argument, NULL, 'N'},
        {"digraph6", no_argument, NULL, 'D'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hD4N", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                switch(option_index) {
                    case 0:
                        graph_number_header = FALSE;
                        break;
                    default:
                        fprintf(stderr, "Illegal option index %d.\n", option_index);
                        usage(name);
                        return EXIT_FAILURE;  
                }
                break;
            case '4':
                quartic = TRUE;
                break;
            case 'N':
                non_quartic = TRUE;
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
    if(quartic && non_quartic){
        fprintf(stderr, "Quartic and non-quartic cannot be combined -- exiting!\n");
        exit(EXIT_FAILURE);
    }


    while (read_graph(stdin, graph, out, in)) {
        graph_count++;

        if(graph_number_header){
            fprintf(stdout, "Graph %llu:\n", graph_count);
        }
        find_z6(graph, out, in);
        if(graph_number_header){
            fprintf(stdout, "\n");
        }
    }
    
    fprintf(stderr, "Read %llu graph%s.\n", graph_count, graph_count==1 ? "" : "s");

    return (EXIT_SUCCESS);
}

