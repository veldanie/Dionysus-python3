#!/usr/bin/env python

from    dionysus        import Simplex, Filtration, StaticPersistence, vertex_cmp
from    sys             import argv, exit


def max_vertex(s, vertices):
    return max((vertices[v] for v in s.vertices))

def max_vertex_cmp(s1, s2, vertices):
    m1 = max_vertex(s1, vertices)
    m2 = max_vertex(s2, vertices)
    return cmp(m1, m2) or cmp(s1.dimension(), s2.dimension())

def lsf(values_filename, simplices_filename):
    # Read vertices
    vertices = []
    with open(values_filename) as f:
        for line in f:
            if line.startswith('#'): continue
            vertices.append(float(line.split()[1]))

    # Read simplices
    simplices = []            
    with open(simplices_filename) as f:
        for line in f:
            if line.startswith('#'): continue
            simplices.append(map(int, line.split()))

    # Setup the complex
    complex = [Simplex(s) for s in simplices]
    complex.sort(vertex_cmp)

    # Compute persistence
    f = Filtration(complex, lambda x,y: max_vertex_cmp(x,y,vertices))
    p = StaticPersistence(f)
    p.pair_simplices()
    
    # Output the persistence diagram
    for i in p:
        if not i.sign(): continue

        b = complex[f[p(i)]]
        d = complex[f[p(i.pair)]]

        if i == i.pair:
            print b.dimension(), max_vertex(b, vertices), "inf"
            continue

        print b.dimension(), max_vertex(b, vertices), max_vertex(d, vertices)


if __name__ == '__main__':
    if len(argv) < 3:
        print "Usage: %s VERTICES SIMPLICES" % argv[0]
        print 
        print "Computes persistence of the lower star filtration of the simplicial "
        print "complex explicitly listed out in SIMPLICES with vertex values given in VERTICES."
        exit()

    values = argv[1]
    simplices = argv[2]

    lsf(values, simplices)
