:class:`Filtration` class
=========================

.. class:: Filtration
    
    This class serves as a bridge between a complex represented as a
    lexicographically sorted list of simplices, and the
    :class:`StaticPersistence` class which needs to know the order in which the
    simplices appear in the filtration.

    .. method:: __init__(simplices, cmp)
    
        Initializes :class:`Filtration` by internally arranging the indices of
        the elements in the list `simplices` in the order sorted with respect to
        `cmp`.

    .. method:: __getitem__()

        Random access to the elements of the filtration.

    .. method:: __iter__()
 
        Iterator over the elements of the filtration, which are simply the
        indices of the simplices in the original list `lst` sorted with respect
        to the comparison `cmp`. E.g.::

            simplices = [Simplex([0], 2), ..., Simplex([3,4,5], 3.5)]
            f = Filtration(simplices, data_dim_cmp)
            for i in f: print simplices[i]

    .. method:: __len__()

        Size of the filtration.
