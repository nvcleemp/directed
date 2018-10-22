/* 
 * File:   directed_io.h
 * Author: nvcleemp
 *
 * Created on October 18, 2018, 1:34 PM
 */

#ifndef DIRECTED_IO_H
#define	DIRECTED_IO_H

#include <stdio.h>
#include <limits.h>
#include "directed_base.h"

#define MAXCODELENGTH (MAXN * MAXVAL + MAXN)
#define EMPTY UCHAR_MAX

#ifdef	__cplusplus
extern "C" {
#endif

boolean read_graph_from_watercluster_file(FILE *f, GRAPH graph, DEGREES out, DEGREES in);

void write_watercluster_format(GRAPH graph, DEGREES out, FILE *f);

boolean read_graph_from_digraph6_file(FILE *f, GRAPH graph, DEGREES out, DEGREES in);

/**
 * Prints a human readable representation of the graph.
 */
void print_graph(FILE *f, GRAPH graph, DEGREES out, DEGREES in);

#ifdef	__cplusplus
}
#endif

#endif	/* DIRECTED_IO_H */

