/*
 * 
 * Copyright (C) 2019 Ghent University.
 */

/* This program reads directed graphs in watercluster or digraph6 format from 
 * standard in and computes a specified invariant for them.
 * 
 * 
 * Compile with:
 *     
 *     cc -o directed_invariant -O4 directed_invariant.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "../shared/directed_base.h"
#include "../shared/directed_io.h"

//========================OVERVIEW==============================

typedef struct _overview_tree_element OVERVIEW_TREE_ELEMENT;
typedef OVERVIEW_TREE_ELEMENT OVERVIEW_TREE;

struct _overview_tree_element {
    int value;
    unsigned long long int count; 
    
    OVERVIEW_TREE_ELEMENT *smaller;
    OVERVIEW_TREE_ELEMENT *greater;
    
    FILE *file;
};

OVERVIEW_TREE_ELEMENT *new_overview_tree_element(int value) {
    OVERVIEW_TREE_ELEMENT *el = (OVERVIEW_TREE_ELEMENT *)malloc(sizeof(OVERVIEW_TREE_ELEMENT));
    
    el->value = value;
    el->count = 0ULL;
    el->smaller = NULL;
    el->greater = NULL;
    el->file = NULL;
    
    return el;
}

void free_overview_tree(OVERVIEW_TREE *overview) {
    if(overview->smaller != NULL) free_overview_tree(overview->smaller);
    if(overview->greater != NULL) free_overview_tree(overview->greater);
    if(overview->file != NULL) fclose(overview->file);
    
    free(overview);
}

OVERVIEW_TREE_ELEMENT *find_element_for_value(OVERVIEW_TREE_ELEMENT *ote, int value){
    if(value == ote->value){
        return ote;
    } else if (value < ote->value){
        if (ote->smaller == NULL){
            ote->smaller = new_overview_tree_element(value);
        }
        return find_element_for_value(ote->smaller, value);
    } else { // value > ote->value
        if (ote->greater == NULL){
            ote->greater = new_overview_tree_element(value);
        }
        return find_element_for_value(ote->greater, value);
    }
}

OVERVIEW_TREE *add_to_overview(OVERVIEW_TREE *overview, int value, boolean write_graph_to_file, char *file_prefix, GRAPH graph, DEGREES out, DEGREES in){
    //find correct element
    OVERVIEW_TREE_ELEMENT *ote;
    if(overview==NULL){
        overview = ote = new_overview_tree_element(value);
    } else {
        ote = find_element_for_value(overview, value);
    }
    
    //add graph
    ote->count++;
    
    if(write_graph_to_file){
        if(ote->file==NULL){
            char filename[100];
            if(sprintf(filename, "%s_%d.wcf", file_prefix, value)>0){
                ote->file = fopen(filename, "w");
            } else {
                fprintf(stderr, "Could not create file for value %d -- exiting!\n", value);
                exit(EXIT_FAILURE);
            }
            if (ote->file==NULL) {
                fprintf(stderr, "Could not create file for value %d -- exiting!\n", value);
                exit(EXIT_FAILURE);
            }
        }
        write_watercluster_format(graph, out, ote->file);
    }
    return overview;
}

void print_overview(OVERVIEW_TREE *overview, FILE *f){
    if(overview->smaller!=NULL) print_overview(overview->smaller, f);
    fprintf(f, "%llu graph%s value %d.\n", overview->count, (overview->count)==1 ? " has" : "s have", overview->value);
    if(overview->greater!=NULL) print_overview(overview->greater, f);
}

//========================INVARIANTS==============================

int arc_count(GRAPH graph, DEGREES out, DEGREES in){
    int i;
    int count = 0;
    for(i=1; i<=graph[0][0]; i++){
        count+=out[i];
    }
    return count;
}

int sink_count(GRAPH graph, DEGREES out, DEGREES in){
    int i;
    int count = 0;
    for(i=1; i<=graph[0][0]; i++){
        if(out[i]==0) count++;
    }
    return count;
}

int source_count(GRAPH graph, DEGREES out, DEGREES in){
    int i;
    int count = 0;
    for(i=1; i<=graph[0][0]; i++){
        if(in[i]==0) count++;
    }
    return count;
}

//====================== USAGE =======================

void help(char *name) {
    fprintf(stderr, "Computes an invariant for the directed graphs read from standard in.\n\n");
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s invariant [options]\n\n", name);
    fprintf(stderr, "\nThis program can handle graphs up to %d vertices. Recompile with a larger\n", MAXN);
    fprintf(stderr, "value for MAXN if you need to handle larger graphs.\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "    -i, --invariants\n");
    fprintf(stderr, "       Print a list of available invariants and return.\n");
    fprintf(stderr, "    -f #, --filter #\n");
    fprintf(stderr, "       Filter graphs that have the specified value for the invariant.\n");
    fprintf(stderr, "    -l, --less\n");
    fprintf(stderr, "       Filter allows graphs with value less than the specified value.\n");
    fprintf(stderr, "    -g, --greater\n");
    fprintf(stderr, "       Filter allows graphs with value greater than the specified value.\n");
    fprintf(stderr, "    -n, --not-equal\n");
    fprintf(stderr, "       Filter does not allow graphs with value equal to specified value.\n");
    fprintf(stderr, "    -s, --summary\n");
    fprintf(stderr, "       Print an overview of the distribution of all values.\n");
    fprintf(stderr, "    -S prefix, --split prefix\n");
    fprintf(stderr, "       If a graph has value #, then it is written to the file prefix_#.wcf.\n");
    fprintf(stderr, "    -D, --digraph6\n");
    fprintf(stderr, "       Reads graphs in digraph6 format instead of watercluster format.\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
}

void usage(char *name) {
    fprintf(stderr, "\033[31mUsage: %s invariant [options]\n", name);
    fprintf(stderr, "For more information type: %s -h\n", name);
    fprintf(stderr, "For a list of invariants type: %s -i\033[0m\n\n", name);
}

void invariants() {
    fprintf(stderr, "Available invariants are\n");
    fprintf(stderr, " * \033[1marcs\033[0m: the number of arcs in the graph\n");
    fprintf(stderr, " * \033[1msink\033[0m: the number of sinks in the graph\n");
    fprintf(stderr, " * \033[1msource\033[0m: the number of sources in the graph\n");
}

/*
 * 
 */
int main(int argc, char** argv) {
    
    boolean filter = FALSE;
    int filter_value = -1;
    int filtered_count = 0;
    
    boolean allow_equal = TRUE;
    boolean allow_less = FALSE;
    boolean allow_greater = FALSE;
    
    boolean give_overview = FALSE;
    boolean split_to_files = FALSE;
    char *prefix;

    int graph_count = 0;
    
    GRAPH graph;
    DEGREES out;
    DEGREES in;
    
    OVERVIEW_TREE *overview = NULL;
    
    boolean verbose = FALSE;
    
    boolean (*read_graph)(FILE *, GRAPH, DEGREES, DEGREES) = read_graph_from_watercluster_file;
    int (*invariant)(GRAPH, DEGREES, DEGREES) = NULL;

    /*=========== commandline parsing ===========*/

    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"not-equal", no_argument, NULL, 'n'},
        {"less", no_argument, NULL, 'l'},
        {"greater", no_argument, NULL, 'g'},
        {"filter", required_argument, NULL, 'f'},
        {"verbose", no_argument, NULL, 'v'},
        {"digraph6", no_argument, NULL, 'D'},
        {"summary", no_argument, NULL, 's'},
        {"split", required_argument, NULL, 'S'},
        {"invariants", no_argument, NULL, 'i'},
        {"help", no_argument, NULL, 'h'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hiDf:vS:snlg", long_options, &option_index)) != -1) {
        switch (c) {
            case 'n':
                allow_equal = FALSE;
                break;
            case 'l':
                allow_less = TRUE;
                break;
            case 'g':
                allow_greater = TRUE;
                break;
            case 'f':
                filter = TRUE;
                filter_value = atoi(optarg);
                break;
            case 'v':
                verbose = TRUE;
                break;
            case 'D':
                read_graph = read_graph_from_digraph6_file;
                break;
            case 'S':
                split_to_files = TRUE;
                prefix = optarg;
                break;
            case 's':
                give_overview = TRUE;
                break;
            case 'i':
                invariants();
                return EXIT_SUCCESS;
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
    
    if(argc - optind < 1){
        fprintf(stderr, "Please specifiy the invariant -- exiting!\n");
        usage(name);
        return EXIT_FAILURE;
    }
    
    if(strcmp(argv[optind], "arcs")==0){
        invariant = arc_count;
    } else if(strcmp(argv[optind], "sink")==0){
        invariant = sink_count;
    } else if(strcmp(argv[optind], "source")==0){
        invariant = source_count;
    } else {
        fprintf(stderr, "Unknown invariant -- exiting!\n");
        usage(name);
        exit(EXIT_FAILURE);
    }
    
    if(filter && !(allow_equal || allow_greater || allow_less)){
        fprintf(stderr, "This filter will not let any graph pass -- exiting!\n");
        usage(name);
        return EXIT_FAILURE;
    }
    
    if(filter && (allow_equal && allow_greater && allow_less)){
        fprintf(stderr, "This filter will let any graph pass -- exiting!\n");
        usage(name);
        return EXIT_FAILURE;
    }
    
    int value, min, max, min_count, max_count, min_graph, max_graph;
    
    boolean first = TRUE;

    while (read_graph(stdin, graph, out, in)) {
        graph_count++;
        value = invariant(graph, out, in);
        if(first){
            first = FALSE;
            min = max = value;
            min_count = max_count = 1;
            min_graph = max_graph = graph_count;
        } else {
            if(min > value){
                min = value;
                min_count = 1;
                min_graph = graph_count;
            } else if(min == value){
                min_count++;
            }
            if(max < value){
                max = value;
                max_count = 1;
                max_graph = graph_count;
            } else if(max == value){
                max_count++;
            }
        }
        if(verbose){
            fprintf(stderr, "Graph %d has value %d.\n", graph_count, value);
        }
        if(filter){
            if(allow_equal && filter_value == value){
                write_watercluster_format(graph, out, stdout);
                filtered_count++;
            } else if(allow_less && filter_value > value){
                write_watercluster_format(graph, out, stdout);
                filtered_count++;
            } else if(allow_greater && filter_value < value){
                write_watercluster_format(graph, out, stdout);
                filtered_count++;
            }
        }
        if(give_overview || split_to_files) {
            overview = add_to_overview(overview, value, split_to_files, prefix, graph, out, in);
        }
    }
    
    int filter_descriptor = (allow_equal ? 1 : 0) +
                            (allow_greater ? 2 : 0) + 
                            (allow_less ? 4 : 0);
    const char *filter_descriptions[8];
    filter_descriptions[0] = "ILLEGAL FILTER"; //pass none
    filter_descriptions[1] = "";
    filter_descriptions[2] = " greater than";
    filter_descriptions[3] = " greater than or equal to";
    filter_descriptions[4] = " less than";
    filter_descriptions[5] = " less than or equal to";
    filter_descriptions[6] = " different from";
    filter_descriptions[7] = "ILLEGAL FILTER"; //pass all
    
    fprintf(stderr, "Read %d graph%s.\n", graph_count, graph_count==1 ? "" : "s");
    if(filtered_count){
        fprintf(stderr, "Written %d graph%s with value%s %d.\n", 
                filtered_count, filtered_count==1 ? "" : "s",
                filter_descriptions[filter_descriptor],
                filter_value);
    }
    if(graph_count){
        fprintf(stderr, "Minimum: %d\nMaximum: %d\n", min, max);
        fprintf(stderr, "%d graph%s the minimum and %s is graph %d.\n",
                min_count, min_count==1 ? " has" : "s have",
                min_count==1 ? "this" : "the first of these",
                min_graph);
        fprintf(stderr, "%d graph%s the maximum and %s is graph %d.\n",
                max_count, max_count==1 ? " has" : "s have",
                max_count==1 ? "this" : "the first of these",
                max_graph);
    
        if(overview!=NULL){
            if(give_overview) {
                print_overview(overview, stderr);
            }
            free_overview_tree(overview);
        }
    }

    return (EXIT_SUCCESS);
}

