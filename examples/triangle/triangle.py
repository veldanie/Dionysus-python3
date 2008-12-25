from dionysus import Simplex, Filtration, StaticPersistence, \
                     vertex_cmp, data_cmp, data_dim_cmp \
#                    ,enable_log

complex = [Simplex((0,),        0),                 # A
           Simplex((1,),        1),                 # B
           Simplex((2,),        2),                 # C
           Simplex((0,1),       2.5),               # AB
           Simplex((1,2),       2.9),               # BC
           Simplex((0,2),       3.5),               # CA
           Simplex((0,1,2),     5)]                 # ABC

print "Complex:", complex
print "Vertex: ", sorted(complex, vertex_cmp)
print "Data:   ", sorted(complex, data_cmp)
print "DataDim:", sorted(complex, data_dim_cmp)

complex.sort(vertex_cmp)

f = Filtration(complex, data_cmp)
for i in f: print i,
print

#enable_log("topology/persistence")
p = StaticPersistence(f)
p.pair_simplices()
for i in p: 
    print i.sign()#, i.pair
    #for ii in i.cycle: print ii
