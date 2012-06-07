from    PyQt4       import QtGui, QtCore
from    dionysus    import Simplex

class ComplexViewer(QtGui.QGraphicsView):
    def __init__(self, points, complex = None, values = None):
        super(QtGui.QGraphicsView, self).__init__()

        self.points = points
        if complex:
            self.complex = [s for s in complex]
        else:
            # Create vertex simplices if no complex provided
            self.complex = [Simplex([i]) for i in xrange(len(self.points))]

        if not values:
            values = [0]*len(self.points)
        self.maxval, self.minval = max(values), min(values)

        self.setRenderHint(QtGui.QPainter.Antialiasing)
        self.scene = QtGui.QGraphicsScene(self)
        self.setScene(self.scene)

        minx = min(p[0] for p in points)
        miny = min(p[1] for p in points)
        maxx = max(p[0] for p in points)
        maxy = max(p[1] for p in points)

        radius = min(maxx - minx, maxy - miny)/100
        self.scene.setSceneRect(minx - 10*radius, miny - 10*radius, (maxx - minx) + 20*radius, (maxy - miny) + 20*radius)

        self.complex.sort(lambda s1, s2: -cmp(s1.dimension(), s2.dimension()))
        for s in self.complex:
            vertices = [v for v in s.vertices]
            if s.dimension() == 0:              # point
                p = points[vertices[0]]
                v = values[vertices[0]]
                item = QtGui.QGraphicsEllipseItem(p[0] - radius/2,p[1] - radius/2,radius,radius)
                color = self.colormap(v)
                item.setBrush(QtGui.QBrush(color))
                item.setPen(QtGui.QPen(color))
            elif s.dimension() == 1:            # edge
                p0 = points[vertices[0]]
                p1 = points[vertices[1]]
                item = QtGui.QGraphicsLineItem(p0[0], p0[1], p1[0], p1[1])
            else:                               # higher-d simplex
                pts = [QtCore.QPointF(points[v][0], points[v][1]) for v in vertices]
                item = QtGui.QGraphicsPolygonItem(QtGui.QPolygonF(pts))
                item.setBrush(QtCore.Qt.blue)

            self.scene.addItem(item)

        # Flip y-axis
        self.scale(1,-1)

        # Set the correct view
        rect = self.scene.itemsBoundingRect()
        self.fitInView(rect, QtCore.Qt.KeepAspectRatio)

    def colormap(self, v):
        if self.maxval <= self.minval:
            t = 0
        else:
            t = (v - self.minval)/(self.maxval - self.minval)
        c = QtGui.QColor()
        c.setHsv(int(t*255), 255, 255)
        return c

# TODO: cycle
def show_complex_2D(points, complex = None, values = None, app = None):
    #app = QtGui.QApplication([])
    view = ComplexViewer(points, complex, values)
    view.show()
    view.raise_()
    app.exec_()
