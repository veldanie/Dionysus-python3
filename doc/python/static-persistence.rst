:class:`StaticPersistence` class
================================

.. class:: StaticPersistence

    .. method:: __init__(filtration)

        Initializes :class:`StaticPersistence` with the given
        :class:`Filtration`. This operation effectively computes the boundary
        matrix of the complex captured by the filtration with rows and columns
        sorted with respect to the filtration ordering.

    .. method:: pair_simplices()

        Pairs simplices using the [ELZ02]_ algorithm.

    .. method:: __call__(i)

        Given an SPNode in the internal representation, the method returns its
        integer offset from the beginning of the filtration. This is useful to
        lookup the actual name of the simplex in the complex. For example, the
        following snippet prints out all the unpaired simplices::

            for i in persistence:
                if i == i.pair: print complex[filtration[persistence(i)]]

    .. method:: __iter__()

        Iterator over the nodes (representing individual simplices). See
        :class:`SPNode`.

    .. method:: __len__()

        Returns the number of nodes (i.e. the number of simplices).


.. class:: SPNode

    The class represents nodes stored in :class:`StaticPersistence`. These nodes
    are aware of their :meth:`sign` and :attr:`pair` (and :meth:`cycle` if
    negative after :meth:`StaticPersistence.pair_simplices` has run).

    .. method:: sign()

        Returns the sign of the simplex: `True` for positive, `False` for
        negative.

    .. attribute:: pair

        Simplex's pair. The pair is set to self if the siplex is unpaired.

    .. method:: cycle()

        If the simplex is negative, its cycle (that it kills) is non-empty, and
        can be accessed using this method. The cycle itself is an iterable
        container of :class:`SPNode`. For example, one can print the basis for
        the (bounding) cycles::

            for i in persistence:
                for ii in i.cycle(): print complex[filtration[persistence(ii)]]

    .. method:: __eq__(other)

        Returns true if the two nodes are the same. Useful for determining if
        the node is unpaired (iff ``i == i.pair``), e.g::

            print len([i in persistence if i == i.pair])    # prints the number of unpaired simplices
