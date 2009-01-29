from _dionysus import *


#def init_with_data(s,v, d = None):
#    s._cpp_init_(v)
#    if d is not None:
#        s.data = d
#
#Simplex._cpp_init_ = Simplex.__init__
#Simplex.__init__ = init_with_data
#
#def data_cmp(s1, s2):
#    return cmp(s1.data,s2.data)
#
#def data_dim_cmp(s1,s2):
#    dim_cmp = cmp(s1.dimension(), s2.dimension())
#    if dim_cmp: return dim_cmp
#    else:       return data_cmp(s1,s2)
