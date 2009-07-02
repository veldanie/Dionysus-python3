.. _alpha-shape-example:

Alpha shape example
===================

The example given in :sfile:`examples/alphashapes/alphashapes.py` takes a
filename containing points in 2D or 3D on the command line. It generates the
alpha shape filtration of those points, and computes its persistence. It then
outputs the persistence diagram in the format of a point (dimension, birth,
death) per line.

.. literalinclude:: ../../examples/alphashapes/alphashapes.py
   :language: python

After the points are read into the list ``points``, the functions
:ref:`fill_alpha*_complex <alphashapes>` fill the list simplices with the
simplices of the Delaunay triangulation. Each one has its :attr:`~Simplex.data`
attribute set to its alpha shape value (the minimum value of the squared
distance function on its dual Voronoi cell).

The simplices are put into lexicographic order (required for
:class:`Filtration`), and then a filtration is created that sorts simplices with
respect to their data and dimension (via :func:`data_dim_cmp`)::

    simplices.sort(vertex_cmp)
    f = Filtration(simplices, data_dim_cmp)

We initialize :class:`StaticPersistence`, and pair the simplices::

    p = StaticPersistence(f)
    p.pair_simplices()
    
Iterating over the :class:`StaticPersistence`, we output the points of the
persistence diagram (dimension, birth, death) in the last for loop. The ``i ==
i.pair`` condition indicates that the positive simplex is unpaired (i.e. the
class it creates survives till infinity).
