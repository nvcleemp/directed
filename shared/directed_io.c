/* 
 * File:   directed_io.c
 * Author: nvcleemp
 *
 * Created on October 18, 2018, 1:33 PM
 */

#include <stdlib.h>
#include "directed_io.h"

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

boolean read_graph_from_watercluster_file(FILE *f, GRAPH graph, DEGREES out, DEGREES in){
    int length;
    if (read_watercluster_format(code, &length, f)) {
        decode_watercluster_format(code, length, graph, out, in);
        return TRUE;
    } else {
        return FALSE;
    }
}

void write_watercluster_format(GRAPH graph, DEGREES out, FILE *f){
    int i, j;
    
    int vertex_count = graph[0][0];
    
    //write the number of vertices
    fputc(vertex_count, f);
    
    for(i=1; i<=vertex_count; i++){
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
    if (nvcleemp_getline(f)) {
        decode_digraph6(graph, out, in);
        return TRUE;
    } else {
        return FALSE;
    }
}

