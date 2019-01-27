/* 
 * File:   directed_base.c
 * Author: nvcleemp
 *
 * Created on October 18, 2018, 1:36 PM
 */

#include <stdlib.h>
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

void prepare_graph(GRAPH graph, DEGREES out, DEGREES in, int order){
    int i;
    
    graph[0][0] = order;
    
    for (i = 1; i <= order; i++) {
        out[i] = 0;
        in[i] = 0;
    }
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

void union_graphs(GRAPH graph1, DEGREES out1, DEGREES in1, GRAPH graph2, DEGREES out2, DEGREES in2, GRAPH union_graph, DEGREES union_out, DEGREES union_in){
    int i, j;
    
    //clear union
    union_graph[0][0] = graph1[0][0] > graph2[0][0] ? graph1[0][0] : graph2[0][0];
    for(i = 1; i <= union_graph[0][0]; i++){
        union_in[i] = union_out[i] = 0;
    }
    
    //copy in the structure of graph 1
    for(i = 1; i <= graph1[0][0]; i++){
        for(j = 0; j < out1[i]; j++){
            add_arc(union_graph, union_out, union_in, i, graph1[i][j]);
        }
    }
    
    //copy in the structure of graph 2
    for(i = 1; i <= graph2[0][0]; i++){
        for(j = 0; j < out2[i]; j++){
            add_arc(union_graph, union_out, union_in, i, graph2[i][j]);
        }
    }
}