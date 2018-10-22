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