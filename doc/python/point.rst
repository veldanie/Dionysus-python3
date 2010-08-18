:class:`Point` class
======================

.. class:: Point

    .. method:: __init__( x, y [, data])
    
        Initializes :class:`Point` with the given real-valued coordinates and
        optionally real value `data`, e.g.::
    
            s = Point( 1, 1.1, 1.11 )

    .. attribute:: x
        
        x coordinate of the point

    .. attribute:: y
        
        y coordinate of the point

    .. attribute:: data
        
        Real value stored in the simplex.

    .. method:: __iter__( )

        Point objects are iterable, returning two or three elements depending on presence of data, e.g.::

            p = Point( 1, 1.1, 1.11 )
            for i in p:  print p

            1
            1.1
            1.11
