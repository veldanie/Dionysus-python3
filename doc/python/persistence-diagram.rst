:class:`PersistenceDiagram` class
==================================

.. class:: PersistenceDiagram

    .. method:: __init__( dimension )
    
        Initializes : an empty( no points ) :class:`PersistenceDiagram` object and sets
        the :attr:`~PersistenceDiagram.dimension` attribute( must be integer ) e.g.::
    
            dia = PersistenceDiagram( 1 )

    .. method:: __init__( dimension, point_seq )
    
        Initializes :class:`PersistenceDiagram` of specified dimension from the given sequence `seq` of :class:`Point` objects, e.g.::
    
            dia = PersistenceDiagram( 1, [Point(1,2)] )

    .. method:: append( p )
        
        Adds point `p` to the persistence diagram.

    .. attribute:: dimension

        Dimension of the persistence diagram. Must be an integer. Must be set at initialization.

    .. method:: __iter__( )

        Iterator over the points in the persistence diagram,
        e.g.::
            
            for p in dia: print p

    .. method:: __len__( )

        :returns: The number of points in the diagram.



Utility functions for persistence diagrams
--------------------------------------------


.. function:: bottleneck_distance(dia1, dia2)
    
    Calculates the bottleneck distance between the two persistence diagrams. 



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

