from    PyQt4       import QtGui, QtCore

class ComplexViewer(QtGui.QGraphicsView):
    def __init__(self, complex, points):
        super(QtGui.QGraphicsView, self).__init__()

        self.complex = [s for s in complex]
        self.points = points

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
            if s.dimension() == 0:
                p = points[vertices[0]]
                item = QtGui.QGraphicsEllipseItem(p[0] - radius/2,p[1] - radius/2,radius,radius)
                # TODO: vertex colors
                color = QtCore.Qt.red
                item.setBrush(QtGui.QBrush(color))
                item.setPen(QtGui.QPen(color))
            elif s.dimension() == 1:
                p0 = points[vertices[0]]
                p1 = points[vertices[1]]
                item = QtGui.QGraphicsLineItem(p0[0], p0[1], p1[0], p1[1])
            else:
                pts = [QtCore.QPointF(points[v][0], points[v][1]) for v in vertices]
                item = QtGui.QGraphicsPolygonItem(QtGui.QPolygonF(pts))
                item.setBrush(QtCore.Qt.blue)

            self.scene.addItem(item)

        # Flip y-axis
        self.scale(1,-1)

        # Set the correct view
        rect = self.scene.itemsBoundingRect()
        self.fitInView(rect, QtCore.Qt.KeepAspectRatio)

# TODO: cycle
def show_complex_2D(complex, points):
    app = QtGui.QApplication([])
    view = ComplexViewer(complex, points)
    view.show()
    view.raise_()
    app.exec_()

