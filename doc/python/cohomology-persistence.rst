:class:`CohomologyPersistence` class
====================================

The :ref:`rips-pairwise-cohomology` illustrates the use of :class:`CohomologyPersistence`.

.. class:: CohomologyPersistence

    .. method:: __init__(prime)

        Initializes :class:`CohomologyPersistence` with the given `prime`; from
        this point on all the computation will be performed with coefficients
        coming from :math:`\mathbb{Z}/prime \mathbb{Z}`.

    .. method:: add(boundary, birth, [store = True], [image = True], [coefficients = []])
        
        Adds a simplex with the given `boundary` to the complex, i.e. 
        :math:`K_{i+1} = K_i \cup \sigma` and `boundary` = :math:`\partial \sigma`.
        If a new class is born as a result of the addition, `birth` is stored with 
        it for future reference. 
        
        If `store` is ``False`` and a class is born, it will not be stored in
        :class:`CohomologyPersistence`. This is useful to not waste space on the
        classes of the dimension equal to the maximum-dimensional simplices of
        the complex since such classes will never die.

        The `image` parameter allows one to work with a case of a space 
        :math:`L \subseteq K` where the filtration of :math:`K` induces a
        filtration of :math:`L`. In this case, one may want to compute **image
        persistence** (i.e. the persistence of the sequences of the images given
        by the inclusion of :math:`L` in :math:`K`). `image` indicates whether
        the simplex added belongs to :math:`L` or not.

        If given, `coefficients` is a list parallel to `boundary` that provides
        coefficients for the corresponding boundary elements. If empty, it is
        assumed to be :math:`(-1)^i`.

        :returns: a pair (`i`, `d`). The first element is the index `i`. 
                  It is the internal representation of the newly added simplex,
                  and should be used later for removal or when constructing the
                  boundaries of its cofaces. In other words, `boundary` must
                  consist of these indices.  The second element `d` is the death
                  element. It is `None` if a birth occurred, otherwise it
                  contains the value passed as `birth` to
                  :meth:`~CohomologyPersistence.add` when the class that just
                  died was born.

    .. method:: __iter__()

        Iterator over the live cocycles stored in
        :class:`CohomologyPersistence`. The returned elements are of the type
        :class:`Cocycle` below.

       
.. class:: Cocycle        

    .. attribute:: birth

        The birth value associated with the cocycle. It is passed to
        :class:`CohomologyPersistence` in method
        :meth:`~CohomologyPersistence.add`.

    .. method:: __iter__()

        Iterator over the individual nodes (simplices) of the cocycle, each of type
        :class:`CHSNode`.

.. class:: CHSNode

    .. attribute:: si

        The index of the simplex, of type :class:`CHSimplexIndex`.

    .. attribute:: coefficient

        Coefficient in :math:`\mathbb{Z}/prime \mathbb{Z}` associated with the
        simplex.


.. class:: CHSimplexIndex

    .. attribute:: order

        The count associated with the simplex when it is inserted into
        :class:`CohomologyPersistence`.
