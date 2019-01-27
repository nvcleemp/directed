/* 
 * File:   directed_base.h
 * Author: nvcleemp
 *
 * Created on October 18, 2018, 1:37 PM
 */

#ifndef DIRECTED_BASE_H
#define	DIRECTED_BASE_H

#define MAXN 32
#define MAXVAL 32

#define TRUE 1
#define FALSE 0

typedef int boolean;

typedef unsigned char GRAPH[MAXN + 1][MAXVAL + 1];
typedef unsigned char DEGREES[MAXN + 1];

#ifdef	__cplusplus
extern "C" {
#endif

void add_arc(GRAPH graph, DEGREES out, DEGREES in, int from, int to);
void remove_arc(GRAPH graph, DEGREES out, DEGREES in, int from, int to);
void flip_arc(GRAPH graph, DEGREES out, DEGREES in, int from, int to);

boolean has_arc(GRAPH graph, DEGREES out, int from, int to);

void prepare_graph(GRAPH graph, DEGREES out, DEGREES in, int order);
void copy_graph(GRAPH orig_graph, DEGREES orig_out, DEGREES orig_in, GRAPH copy_graph, DEGREES copy_out, DEGREES copy_in);
void relabel_graph(GRAPH orig_graph, DEGREES orig_out, DEGREES orig_in, GRAPH copy_graph, DEGREES copy_out, DEGREES copy_in, int relabeling[MAXN+1], int new_order);
void union_graphs(GRAPH graph1, DEGREES out1, DEGREES in1, GRAPH graph2, DEGREES out2, DEGREES in2, GRAPH union_graph, DEGREES union_out, DEGREES union_in);

#ifdef	__cplusplus
}
#endif

#endif	/* DIRECTED_BASE_H */

