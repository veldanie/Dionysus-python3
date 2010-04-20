from    _dionysus   import *
from    distances   import l2, ExplicitDistances, points_file
from    zigzag      import *


def init_with_none(self, iter, data = None):        # convenience: data defaults to None
    self._cpp_init_(iter, data)

def repr_with_data(self):
    str = self._cpp_repr_()
    if type(self.data) == float:
        str += ' %f' % self.data
    return str

Simplex._cpp_init_ =    Simplex.__init__
Simplex.__init__   =    init_with_none
Simplex._cpp_repr_ =    Simplex.__repr__
Simplex.__repr__   =    repr_with_data


def data_cmp(s1, s2):
    return cmp(s1.data, s2.data)

def data_dim_cmp(s1,s2):
    return cmp(s1.dimension(), s2.dimension()) or data_cmp(s1,s2)

def vertex_dim_cmp(s1, s2):
    return cmp(s1.dimension(), s2.dimension()) or vertex_cmp(s1, s2)
