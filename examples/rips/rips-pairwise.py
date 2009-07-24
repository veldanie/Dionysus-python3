#/usr/bin/env python


from    dionysus    import Rips, PairwiseDistances, StaticPersistence, Filtration, points_file, \
                           ExplicitDistances, data_dim_cmp
from    sys         import argv, exit
import  time

def main(filename, skeleton, max):
    points = [p for p in points_file(filename)]
    distances = PairwiseDistances(points)
    distances = ExplicitDistances(distances)           # speeds up generation of the Rips complex at the expense of memory usage
    rips = Rips(distances)
    print time.asctime(), "Rips initialized"

    simplices = []
    rips.generate(skeleton, max, simplices.append)
    print time.asctime(), "Generated complex: %d simplices" % len(simplices)

    # While this step is unnecessary (Filtration below can be passed rips.cmp), 
    # it greatly speeds up the running times
    for s in simplices: s.data = rips.eval(s)
    print time.asctime(), simplices[0], '...', simplices[-1]

    f = Filtration(simplices, data_dim_cmp)             # could be rips.cmp if s.data for s in simplices is not set
    print time.asctime(), "Set up filtration"

    p = StaticPersistence(f)
    print time.asctime(), "Initialized StaticPersistence"

    p.pair_simplices()
    print time.asctime(), "Simplices paired"

    print "Outputting persistence diagram"
    for i in p:
        if i.sign():
            b = simplices[f[p(i)]]

            if b.dimension() >= skeleton: continue

            if i == i.pair:
                print b.dimension(), b.data, "inf"
                continue

            d = simplices[f[p(i.pair)]]
            print b.dimension(), b.data, d.data

if __name__ == '__main__':
    if len(argv) < 4:
        print "Usage: %s POINTS SKELETON MAX" % argv[0]
        exit()

    filename = argv[1]
    skeleton = int(argv[2])
    max = float(argv[3])

    main(filename, skeleton, max)
