#!/usr/bin/python3

import math

def read_waterclusterfile(code_input):
    while True:
        c = code_input.read(1)
        if len(c)==0:
            return

        order = ord(c)

        zeroCount = 0

        g = [([],[]) for i in range(order)]

        while zeroCount < order:
            c = code_input.read(1)
            if ord(c) == 0:
                zeroCount += 1
            else:
                g[zeroCount][0].append(ord(c)-1)
                g[ord(c)-1][1].append(zeroCount)

        yield(g)

def cartesian_to_polar(coord):
    x, y = coord
    return math.hypot(x,y),math.degrees(math.atan2(y,x))

def center_coordinates(coords):
    center_x = sum(x for x,_ in coords)/len(coords)
    center_y = sum(y for _,y in coords)/len(coords)
    return [(x-center_x, y-center_y) for x,y in coords]

def graph_to_multi(f, g):
    f.write('{:c}'.format(len(g)).encode('utf-8'))
    for current, neighbours in enumerate(g[:len(g)-1]):
        for neighbour in set(neighbours[0] + neighbours[1]):
            if neighbour > current:
                f.write('{:c}'.format(neighbour + 1).encode('utf-8'))
        f.write('{:c}'.format(0).encode('utf-8'))

def print_node(i, coords):
    x, y = coords
    print("\\node ({}) at ({:.3f},{:.3f}) {{}};".format(i, 10*x, 10*y))

def print_node_polar(i, coords):
    r, theta = cartesian_to_polar(coords)
    print("\\node ({}) at ({:.5f}:{:.3f}) {{}};".format(i, theta, 10*r))

def circular_embedding(g, node_printer):
    for i in range(len(g)):
        print("\\node ({}) at ({}:1) {{}};".format(i, i*360.0/len(g)))

def planar_embedding(g, node_printer):
    command = "./gconv -f graph6_old | ./planarg -p | ./gconv -f writegraph2d | ./embed -o YSd"

    import subprocess
    sp = subprocess.Popen(command, shell=True,
                          stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE, close_fds=True)

    sp.stdin.write(">>multi_code<<".encode('utf-8'))
    graph_to_multi(sp.stdin, g)

    sp.stdin.close()

    w2d = [l.strip() for l in sp.stdout]
    w2d = [tuple(float(f) for f in l.split()[1:3]) for l in w2d[1:len(w2d)-2]]
    for i, coords in enumerate(w2d):
        node_printer(i, coords)

def graph_to_tikz(g, embed_method, node_printer):
    print("\\begin{tikzpicture}\n")
    embed_method(g, node_printer)
    print()
    for i in range(len(g)):
        for j in g[i][0]:
            if i not in g[j][0]:
                print("\\draw[->] ({}) to ({});".format(i, j))
            elif i < j:
                print("\\draw[<->] ({}) to ({});".format(i, j))
    print("\\end{tikzpicture}")
    print()

if __name__ == '__main__':
    import argparse
    import sys
    import os.path

    parser = argparse.ArgumentParser(description='Creates image in TikZ format for directed graph in watercluster format.')
    if (not os.path.isfile('./gconv')) or (not os.path.isfile('./embed')) or (not os.path.isfile('./planarg')):
        print('Missing the tools to handle planar embeddings. The -p option is not available.')
    else:
        parser.add_argument('-p', '--planar', dest='embed_method', action='store_const',
                    const=planar_embedding, default=circular_embedding,
                    help='Gives a plane embedding (fails if the graph is not planar)')
    parser.add_argument('-P', '--polar', dest='node_printer', action='store_const',
                    const=print_node_polar, default=print_node,
                    help='Use polar coordinates')

    args = parser.parse_args()

    for g in read_waterclusterfile(sys.stdin):
        graph_to_tikz(g, args.embed_method, args.node_printer)
