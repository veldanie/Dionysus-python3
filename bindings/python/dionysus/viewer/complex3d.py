from    PyQt4       import QtGui, QtCore
from    PyGLWidget  import PyGLWidget
from    OpenGL.GL   import *
from    dionysus    import Simplex
from    math        import sqrt

class ComplexViewer3D(PyGLWidget):
    def __init__(self, points, complex = None, values = None):
        PyGLWidget.__init__(self)

        self.points = points
        if complex:
            self.complex = [s for s in complex]
        else:
            # Create vertex simplices if no complex provided
            self.complex = [Simplex([i]) for i in xrange(len(self.points))]

        self.values = values
        if not values:
            self.values = [0]*len(self.points)
        self.maxval, self.minval = max(self.values), min(self.values)

        center, radius = self.center_radius()
        self.set_radius(radius)
        self.set_center(center)

    def center_radius(self):
        c = [0,0,0]
        for p in self.points:
            for i in xrange(3): c[i] += p[i]
        for i in xrange(3): c[i] /= len(self.points)

        r = 0
        for p in self.points:
            d = sqrt((p[0] - c[0])**2 + (p[1] - c[1])**2 + (p[2] - c[2])**2)
            if d > r: r = d
        return c,r

    def paintGL(self):
        PyGLWidget.paintGL(self)
        self.complex.sort(lambda s1, s2: -cmp(s1.dimension(), s2.dimension()))
        for s in self.complex:
            vertices = [v for v in s.vertices]
            if s.dimension() == 0:              # point
                p = self.points[vertices[0]]
                v = self.values[vertices[0]]

                glPointSize(3.0)
                c = self.colormap(v)
                cr = float(c.red())/255
                cg = float(c.green())/255
                cb = float(c.blue())/255
                glColor3f(cr, cg, cb)
                glBegin(GL_POINTS)
                glVertex3f(p[0],p[1],p[2])
                glEnd()
            if s.dimension() == 1:            # edge
                p0 = self.points[vertices[0]]
                p1 = self.points[vertices[1]]

                glColor3f(0,0,1)
                glBegin(GL_LINES)
                glVertex3f(p0[0],p0[1],p0[2])
                glVertex3f(p1[0],p1[1],p1[2])
                glEnd()
            elif s.dimension() == 2:
                p0 = self.points[vertices[0]]
                p1 = self.points[vertices[1]]
                p2 = self.points[vertices[2]]

                glColor3f(1,1,0)
                glBegin(GL_TRIANGLES)
                glVertex3f(p0[0],p0[1],p0[2])
                glVertex3f(p1[0],p1[1],p1[2])
                glVertex3f(p2[0],p2[1],p2[2])
                glEnd()


    def colormap(self, v):
        if self.maxval <= self.minval:
            t = 0
        else:
            t = (v - self.minval)/(self.maxval - self.minval)
        c = QtGui.QColor()
        c.setHsv(int(t*255), 255, 255)
        return c

# TODO: cycle
def show_complex_3D(points, complex = None, values = None, app = None):
    #app = QtGui.QApplication([])
    view = ComplexViewer3D(points, complex, values)
    view.show()
    view.raise_()
    app.exec_()
