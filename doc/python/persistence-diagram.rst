:class:`PersistenceDiagram` class
==================================

.. class:: PersistenceDiagram

    .. method:: __init__( point_seq )
    
        Initializes :class:`PersistenceDiagram` from the given sequence `seq` of `Point` objects, e.g.::
    
            dia = PersistenceDiagram( [Point(1,2)] )

    .. method:: __init__( dimension )
    
        Initializes : an empty( no points ) :class:`PersistenceDiagram` object and sets
        the :attr:`~PersistenceDiagram.dimension` attribute( must be integer ) e.g.::
    
            dia = PersistenceDiagram( 1 )

    .. method:: append( p )
        
        Adds point `p` to the persistence diagram.

    .. attribute:: dimension

        Dimension of the persistence diagram. Must be an integer. Must be set at initialization.

    .. attribute:: points

        Iterator over the points in the persistence diagram,
        e.g.::
            
            for p in dia.points: print p

    .. method:: __len__( )

        :returns: The number of points in the diagram.

    .. method:: __iter__( )

        Provides another way to access the :attr:`~PersistenceDiagram.points` iterator, e.g.::

            for p in dia: print p


Utility functions for persistence diagrams
--------------------------------------------


.. function:: bottleneck_distance(dia1, dia2)
    
    Calculates the bottleneck distance between the two persistence diagrams. 
