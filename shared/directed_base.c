/* 
 * File:   directed_base.c
 * Author: nvcleemp
 *
 * Created on October 18, 2018, 1:36 PM
 */

#include <stdio.h>
#include "directed_base.h"

void add_arc(GRAPH graph, DEGREES out, DEGREES in, int from, int to){
    graph[from][out[from]] = to;
    out[from]++;
    in[to]++;
}

void remove_arc(GRAPH graph, DEGREES out, DEGREES in, int from, int to){
    int i;
    while (i < out[from] && graph[from][i] != to) i++;
    
    if(i == out[from]){
        //ERROR: arc is not present
        fprintf(stderr, "Trying to remove an non-existing arc: %d->%d -- exiting!\n", from, to);
        exit(1);
    } else {
        graph[from][i] = graph[from][out[from]-1];
        out[from]--;
        in[to]--;
    }
}

void flip_arc(GRAPH graph, DEGREES out, DEGREES in, int from, int to){
    remove_arc(graph, out, in, from, to);
    add_arc(graph, out, in, to, from);
}

boolean has_arc(GRAPH graph, DEGREES out, int from, int to){
    int i;
    while (i < out[from] && graph[from][i] != to) i++;    
    return i != out[from];
}

void copy_graph(GRAPH orig_graph, DEGREES orig_out, DEGREES orig_in, GRAPH copy_graph, DEGREES copy_out, DEGREES copy_in){
    int i, j;
    for(i = 1; i <= orig_graph[0][0]; i++){
        for(j = 0; j < orig_out[i]; j++){
            copy_graph[i][j] = orig_graph[i][j];
        }
        copy_out[i] = orig_out[i];
        copy_in[i] = orig_in[i];
    }
}

void relabel_graph(GRAPH orig_graph, DEGREES orig_out, DEGREES orig_in, GRAPH copy_graph, DEGREES copy_out, DEGREES copy_in, int relabeling[MAXN+1], int new_order){
    int i, j;
    
    //clear copy
    copy_graph[0][0] = new_order;
    for(i = 1; i <= new_order; i++){
        copy_in[i] = copy_out[i] = 0;
    }
    
    //copy structure
    for(i = 1; i <= orig_graph[0][0]; i++){
        int new_label = relabeling[i];
        for(j = 0; j < orig_out[i]; j++){
            copy_graph[new_label][j] = relabeling[orig_graph[i][j]];
        }
        copy_out[new_label] = orig_out[i];
        copy_in[new_label] = orig_in[i];
    }
}