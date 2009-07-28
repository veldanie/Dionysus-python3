.. _cohomology-parametrization:

Parametrizing a point set using circle valued functions
=======================================================

The procedure described below is explained in detail in [dSVJ09]_.

.. program:: rips-pairwise-cohomology

One can use :sfile:`examples/cohomology/rips-pairwise-cohomology.cpp` to compute
persistent pairing of the Rips filtration using the persistent cohomology
algorithm. It takes as input a file containing a point set in Euclidean space
(one per line) as well as the following command-line flags:

.. cmdoption:: -p, --prime

    The prime to use in the computation (defaults to 11).

.. cmdoption:: -m, --max-distance

    Maximum cutoff parameter up to which to compute the complex.

.. cmdoption:: -s, --skeleton-dimension

    Skeleton to compute; persistent pairs output will be this number minus 1
    (defaults to 2).

.. cmdoption:: -b, --boundary

    Filename where to output the boundary matrix.

.. cmdoption:: -c, --cocycle

    Prefix of the filenames where to output the 1-dimensional cocycles.

.. cmdoption:: -v, --vertices

    Filename where to output the simplex vertex mapping.

.. cmdoption:: -d, --diagram

    Filename where to output the persistence diagram.


For example::

    rips-pairwise-cohomology points.txt -m 1 -b points.bdry -c points -v points.vrt -d points.dgm

Assuming that at the threshold value of 1 (``-m 1`` above) Rips complex contains
1-dimensional cocycles, they will be output into filenames of the form
``points-0.ccl``, ``points-1.ccl``, etc.

Subsequently one can use :sfile:`examples/cohomology/cocycle.py` to assign to
each vertex of the input point set a circle-valued function. It takes the
boundary matrix, cocycle, and simplex-vertex map as an input (all produced at
the previous step)::

    cocycle.py points.bdry points-0.ccl points.vrt

The above command outputs a file ``points-0.val`` which contains values assigned
to the input points (the lines match the lines of the input file
``points.txt``, but also contains the indices).


Plotting
--------

Two auxilliary tools allow one to visualize the values assigned to the points
(using Matplotlib_): :sfile:`tools/plot-values/plot.py` and
:sfile:`tools/plot-values/scatter.py`::
    
    plot.py points-0.val points.txt scatter.py points-0.val points-1.val

.. _Matplotlib:                             http://matplotlib.sourceforge.net/    


Dependency
----------

The Python `LSQR code`_ (ported from the `Stanford MATLAB implementation`_ to
Python by `Jeffery Kline`_) included with Dionysus, and used in
:sfile:`examples/cohomology/cocycle.py`, requires CVXOPT_.

.. _`LSQR code`:                            http://pages.cs.wisc.edu/~kline/cvxopt/
.. _CVXOPT:                                 http://abel.ee.ucla.edu/cvxopt/
.. _`Stanford MATLAB implementation`:       http://www.stanford.edu/group/SOL/software/lsqr.html
.. _`Jeffery Kline`:                        http://pages.cs.wisc.edu/~kline/
