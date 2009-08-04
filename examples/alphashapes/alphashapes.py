# Computes the persistence diagram of the alpha shapes in both 2D and 3D 
# (decided dynamically based on the input file)

from    dionysus        import Filtration, StaticPersistence, data_dim_cmp, vertex_cmp, \
                               fill_alpha3D_complex, fill_alpha2D_complex, points_file
from    sys             import argv, exit
from    math            import sqrt


if len(argv) < 2:
    print "Usage: %s POINTS" % argv[0]
    exit()

points = [p for p in points_file(argv[1])]
simplices = []
if   len(points[0]) == 2:           # 2D
    fill_alpha2D_complex(points, simplices)
elif len(points[1]) == 3:           # 3D
    fill_alpha3D_complex(points, simplices)

simplices.sort(vertex_cmp)                      # Must ensure lexicographic ordering
print "Total number of simplices:", len(simplices)

f = Filtration(simplices, data_dim_cmp)
print "Filtration initialized"

p = StaticPersistence(f)
print "StaticPersistence initialized" 

p.pair_simplices()
print "Simplices paired"

print "Outputting persistence diagram"
for i in p:
    if i.sign():
        b = simplices[f[p(i)]]
        if i == i.pair:
            print b.dimension(), sqrt(b.data), "inf"
            continue

        d = simplices[f[p(i.pair)]]
        print b.dimension(), sqrt(b.data), sqrt(d.data)
