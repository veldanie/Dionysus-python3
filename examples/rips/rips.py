from    math        import fabs
from    dionysus    import Rips, Filtration, StaticPersistence #, enable_log

# Simple minded pairwise distance functor distance
class Distances:
    def __len__(self):
        return 5

    def __call__(self, x, y):
        return fabs(y-x)

dist = Distances()
r = Rips(dist)
lst = []
lst2 = []

#enable_log('rips')

r.generate(1, 3, lst.append)
r.generate(1, 3, lst2.append, [0,2,4])

print "Rips complex on all vertices:", lst
print "Rips complex on vertices [0,2,4]):", lst2

print "Values:", [map(r.eval, lst)]
print "Sorted:", sorted(lst, r.cmp)

cofaces = []
r.vertex_cofaces(2, 1, 3, cofaces.append)
print "Cofaces of vertex 2:", cofaces

cofaces = []
r.vertex_cofaces(2, 1, 3, cofaces.append, [0,2,4])
print "Cofaces of vertex 2 on vertices [0,2,4]:", cofaces

f = Filtration(lst, r.cmp)
p = StaticPersistence(f)
p.pair_simplices()
for s in p:
    print lst[f[p(s)]], s.sign()
