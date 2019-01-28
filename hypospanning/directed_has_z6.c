/*
 * 
 * Copyright (C) 2019 Ghent University.
 */

/* This program reads hypohamiltonian directed graphs in watercluster or 
 * digraph6 format from standard in and determines whether they contain a
 * copy of Z6.
 * 
 * 
 * Compile with:
 *     
 *     cc -o directed_has_z6 -O4 directed_has_z6.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "../shared/directed_base.h"
#include "../shared/directed_io.h"

unsigned long long int graph_count = 0;
unsigned long long int filtered_count = 0;

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
boolean is_z6(GRAPH graph, DEGREES out, DEGREES in, int x, int y, int z){
    int i, j, k;
    for(i=0; i<out[y]; i++){
        if(has_arc(graph, out, graph[y][i], x)){
            for(j=0; j<out[z]; j++){
                if(has_arc(graph, out, graph[z][j], y)){
                    for(k=0; k<out[x]; k++){
                        if(has_arc(graph, out, graph[x][k], z)){
                            if(are_all_different(x,y,z,graph[x][k],graph[y][i],graph[z][j])){
                                return TRUE;
                            }
                        }
                    }
                }
            }
        }
    }
    return FALSE;
}

boolean has_z6(GRAPH graph, DEGREES out, DEGREES in){
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
                                    if(is_z6(graph, out, in, x, y, z)){
                                        return TRUE;
                                    }
                                }
                            }
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
    fprintf(stderr, "Checks directed graphs for containing a copy of Z6.\n\n");
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile if you need larger\n", MAXN);
    fprintf(stderr, "graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -4, --quartic\n");
    fprintf(stderr, "       Only check for a copy of Z6 with all indegrees and outdegrees equal to 2.\n");
    fprintf(stderr, "    -N, --non-quartic\n");
    fprintf(stderr, "       Only check for a copy of Z6 with at least one indegree or outdegree larger than 2.\n");
    fprintf(stderr, "    -f, --filter\n");
    fprintf(stderr, "       Filter graphs that contain a copy of Z6.\n");
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
        {"quartic", no_argument, NULL, '4'},
        {"non-quartic", no_argument, NULL, 'N'},
        {"update", required_argument, NULL, 'u'},
        {"invert", no_argument, NULL, 'i'},
        {"filter", no_argument, NULL, 'f'},
        {"digraph6", no_argument, NULL, 'D'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hfiu:D4N", long_options, &option_index)) != -1) {
        switch (c) {
            case '4':
                quartic = TRUE;
                break;
            case 'N':
                non_quartic = TRUE;
                break;
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
    if(quartic && non_quartic){
        fprintf(stderr, "Quartic and non-quartic cannot be combined -- exiting!\n");
        exit(EXIT_FAILURE);
    }


    while (read_graph(stdin, graph, out, in)) {
        graph_count++;

        boolean value = has_z6(graph, out, in);
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
                fprintf(stdout, "Graph %llu has a copy of Z6.\n", graph_count);
            } else {
                fprintf(stdout, "Graph %llu does not have a copy of Z6.\n", graph_count);
            }
        }
    }
    
    fprintf(stderr, "Read %llu graph%s.\n", graph_count, graph_count==1 ? "" : "s");
    if(do_filtering){
        fprintf(stderr, "Filtered %llu graph%s that %scontain a copy of Z6.\n", filtered_count, filtered_count==1 ? "" : "s", invert ? "do not " : "");
        if(quartic){
            fprintf(stderr, "Only quartic copies of Z6 where taken into account.\n");
        }
        if(non_quartic){
            fprintf(stderr, "Only non-quartic copies of Z6 where taken into account.\n");
        }
    }

    return (EXIT_SUCCESS);
}

