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
#include <limits.h>

#define MAXN 32
#define MAXVAL 32

#define TRUE 1
#define FALSE 0

typedef int boolean;

#define MAXCODELENGTH (MAXN * MAXVAL + MAXN)
#define EMPTY UCHAR_MAX

typedef unsigned char GRAPH[MAXN + 1][MAXVAL + 1];
typedef unsigned char DEGREES[MAXN + 1];

typedef int boolean;

int graph_count = 0;
int filtered_count = 0;
int valid_orientation_count = 0;

void print_graph(GRAPH graph, DEGREES out, DEGREES in){
    int i, j;
    
    for(i = 1; i <= graph[0][0]; i++){
        fprintf(stderr, "%d (%d/%d) ", i, in[i], out[i]);
        for(j = 0; j < out[i]; j++){
            fprintf(stderr, "%d ", graph[i][j]);
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

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

//================= I/O METHODS ====================

void decode_watercluster_format(unsigned char* code, int length, GRAPH graph, DEGREES out, DEGREES in) {
    int i, j, current_vertex;
    unsigned char vertex_count;

    graph[0][0] = vertex_count = code[0];

    //mark all vertices as having degree 0
    for (i = 1; i <= vertex_count; i++) {
        out[i] = 0;
        in[i] = 0;
        for (j = 0; j <= MAXVAL; j++) {
            graph[i][j] = EMPTY;
        }
    }
    //clear first row
    for (j = 1; j <= MAXVAL; j++) {
        graph[0][j] = 0;
    }

    //go through code and add edges
    current_vertex = 1;

    for (i = 1; i < length; i++) {
        if (code[i] == 0) {
            current_vertex++;
        } else {
            graph[current_vertex][out[current_vertex]] = code[i];
            out[current_vertex]++;
            in[code[i]]++;
            if (out[current_vertex] > MAXVAL || in[code[i]] > MAXVAL) {
                fprintf(stderr, "MAXVAL too small (%d)!\n", MAXVAL);
                exit(0);
            }
        }
    }
}

/**
 * 
 * @param code
 * @param length
 * @param file
 * @return returns 1 if a code was read and 0 otherwise. Exits in case of error.
 */
int read_watercluster_format(unsigned char code[], int *length, FILE *file) {
    unsigned char c;
    int pos, zero_count;

    if (fread(&c, sizeof (unsigned char), 1, file) == 0) {
        //nothing left in file
        return (0);
    }

    if (c > MAXN) {
        fprintf(stderr, "Constant N too small %d > %d \n", code[0], MAXN);
        exit(1);
    }
    code[0] = c;
    zero_count = 0;
    pos = 1;
    
    while (zero_count < code[0]) {
        code[pos] = (unsigned char) getc(file);
        if (code[pos] == 0) zero_count++;
        pos++;
    }

    *length = pos;
    return (1);
}

//array to store code in watercluster format
unsigned char code[MAXCODELENGTH];

boolean read_graph_from_watercluster_format(FILE *f, GRAPH graph, DEGREES out, DEGREES in){
    int length;
    if (read_watercluster_format(code, &length, stdin)) {
        decode_watercluster_format(code, length, graph, out, in);
        return TRUE;
    } else {
        return FALSE;
    }
}

void write_watercluster_format(GRAPH graph, DEGREES out, FILE *f){
    int i, j;
    
    int vertexCount = graph[0][0];
    
    //write the number of vertices
    fputc(vertexCount, f);
    
    for(i=1; i<=vertexCount; i++){
        for(j=0; j<out[i]; j++){
            fputc(graph[i][j], f);
        }
        fputc(0, f);
    }
}

#define D6BODYLEN(n) \
   ((n)*(size_t)((n)/6) + (((n)*(size_t)((n)%6)+5)/6))
#define D6LEN(n) (1 + SIZELEN(n) + D6BODYLEN(n))
  /* exact digraph6 string length excluding \n\0 
     This twisted expression works up to n=160529 in 32-bit arithmetic
     and for larger n if size_t has 64 bits.  */
#define FLOCKFILE(f) flockfile(f)
#define FUNLOCKFILE(f) funlockfile(f)
#define GETC(f) getc_unlocked(f)

int available_line_length = 0;
char *line;
     
/* read a line with error checking */
/* includes \n (if present) and \0.  Immediate EOF causes NULL return. */
boolean nvcleemp_getline(FILE *f){
    int c;
    long i;
    
    if(available_line_length==0){
        line = malloc(5000*sizeof(char));
        available_line_length = 5000;
    }
    
    if(line==NULL){
        fprintf(stderr, "Insufficient memory\n");
        exit(1);
    }

    FLOCKFILE(f);
    i = 0;
    while ((c = GETC(f)) != EOF && c != '\n'){
        if (i == available_line_length-3){
            line = realloc(line, (3*(available_line_length/2)+10000)*sizeof(char));
            if(line==NULL){
                fprintf(stderr, "Insufficient memory\n");
                exit(1);
            }
        }
        line[i] = (char)c;
        i++;
    }
    FUNLOCKFILE(f);
    
    if(i == 0 && c == EOF) return FALSE;

    if (c == '\n') line[i++] = '\n';
    line[i] = '\0';
    return TRUE;
}


boolean decode_digraph6(GRAPH graph, DEGREES out, DEGREES in){
    int pos;
    int i, j;
    
    if( line[0]==0 ) return FALSE;
    
    if( line[0]!='&' ){
        fprintf(stderr, "Not a digraph6 file\n");
        exit(1);
    }
    
    if( line[1]==126 ){
        if( line[2]==126 ){
            fprintf(stderr, "Graph too large\n");
            exit(1);
        }
        graph[0][0] = (line[2] - 63) * 4096 + (line[3] - 63) * 64 + (line[4] - 63);
        pos = 5;
    } else {
        graph[0][0] = line[1] - 63;
        pos = 2;
    }
    
    //mark all vertices as having degree 0
    for (i = 1; i <= graph[0][0]; i++) {
        out[i] = 0;
        in[i] = 0;
        for (j = 0; j <= MAXVAL; j++) {
            graph[i][j] = EMPTY;
        }
    }
    //clear first row
    for (j = 1; j <= MAXVAL; j++) {
        graph[0][j] = 0;
    }
    
    int index0 = 1;
    int index1 = 1;
    while (line[pos]!=0) {
        char k;
        char number = (char) (line[pos] - 63);
        pos++;
        for (k = 1 << 5; k >= 1; k >>= 1) {
            if ((number & k)) {
                graph[index1][out[index1]] = index0;
                out[index1]++;
                in[index0]++;
            }
            index0++;
            if (index0 == graph[0][0] + 1) {
                index1++;
                index0 = 1;
            }
        }
    }
}

boolean read_graph_from_digraph6_file(FILE *f, GRAPH graph, DEGREES out, DEGREES in){
    if (nvcleemp_getline(stdin)) {
        decode_digraph6(graph, out, in);
        return TRUE;
    } else {
        return FALSE;
    }
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
    
    boolean (*read_graph)(FILE *, GRAPH, DEGREES, DEGREES) = read_graph_from_watercluster_format;
    
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
                fprintf(stderr, "Read: %d. Filtered: %d\n", graph_count, filtered_count);
            }
        } else {
            if(value){
                fprintf(stdout, "Graph %d is hypohamiltonian.\n", graph_count);
            } else {
                fprintf(stdout, "Graph %d is not hypohamiltonian.\n", graph_count);
            }
        }
    }
    
    fprintf(stderr, "Read %d graph%s.\n", graph_count, graph_count==1 ? "" : "s");
    fprintf(stderr, "Valid orientation: %d\n", valid_orientation_count);
    if(do_filtering){
        fprintf(stderr, "Filtered %d graph%s.\n", filtered_count, filtered_count==1 ? "" : "s");
    }

    return (EXIT_SUCCESS);
}

