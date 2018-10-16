#!/usr/bin/python3

def read_waterclusterfile(code_input):
    while True:
        c = code_input.read(1)
        if len(c)==0:
            return

        order = ord(c)

        zeroCount = 0

        g = [set() for i in range(order)]

        while zeroCount < order:
            c = code_input.read(1)
            if ord(c) == 0:
                zeroCount += 1
            else:
                g[zeroCount].add(ord(c)-1)
                g[ord(c)-1].add(zeroCount)

        yield(g)

def graph_to_multi(g):
    sys.stdout.write('{:c}'.format(len(g)))
    for current, neighbours in enumerate(g[:len(g)-1]):
        for neighbour in neighbours:
            if neighbour > current:
                sys.stdout.write('{:c}'.format(neighbour + 1))
        sys.stdout.write('{:c}'.format(0))

if __name__ == '__main__':
    import sys
    sys.stdout.write('>>multi_code<<')
    for g in read_waterclusterfile(sys.stdin):
        graph_to_multi(g)
