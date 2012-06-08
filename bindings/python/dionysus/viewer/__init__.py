from    diagram     import show_diagram as _show_diagram
from    complex2d   import show_complex_2D
from    complex3d   import show_complex_3D

from    PyQt4       import QtGui

_app = QtGui.QApplication([])

def show_complex(points, complex = None, values = None):
    if len(points[0]) == 2:
        show_complex_2D(points, complex, values, app = _app)
    if len(points[0]) == 3:
        show_complex_3D(points, complex, values, app = _app)

def show_diagram(dgm, noise = 0):
    return _show_diagram(dgm, noise, _app)
