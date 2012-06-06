from    PyQt4       import QtGui, QtCore

class DiagramPoint(QtGui.QGraphicsEllipseItem):
    def __init__(self,x,y,radius, p, viewer):
        super(QtGui.QGraphicsEllipseItem, self).__init__(x,y,radius, radius)
        self.p = p
        self.viewer = viewer

    def mousePressEvent(self, event):
        self.viewer.selection = self.p
        self.viewer.close()

class DiagramViewer(QtGui.QGraphicsView):
    def __init__(self, dgm):
        super(QtGui.QGraphicsView, self).__init__()

        self.selection = None

        self.setRenderHint(QtGui.QPainter.Antialiasing)
        self.scene = QtGui.QGraphicsScene(self)
        self.setScene(self.scene)

        minx = min(p[0] for p in dgm)
        miny = min(p[1] for p in dgm)
        maxx = max(p[0] for p in dgm)
        maxy = max(p[1] for p in dgm)

        self.draw_axes(minx,miny,maxx,maxy)

        radius = min(maxx - minx, maxy - miny)/100
        self.scene.setSceneRect(minx - 10*radius, miny - 10*radius, (maxx - minx) + 20*radius, (maxy - miny) + 20*radius)

        for p in dgm:
            x,y = p[0],p[1]
            item = DiagramPoint(x,y,radius, p, self)
            self.scene.addItem(item)

        # Flip y-axis
        self.scale(1,-1)

        # Set the correct view
        rect = self.scene.itemsBoundingRect()
        self.fitInView(rect, QtCore.Qt.KeepAspectRatio)

    def draw_axes(self, minx, miny, maxx, maxy):
        # Draw axes and diagonal
        self.scene.addItem(QtGui.QGraphicsLineItem(0,0, maxx, 0))
        self.scene.addItem(QtGui.QGraphicsLineItem(minx,0, 0, 0))
        self.scene.addItem(QtGui.QGraphicsLineItem(0,0, 0, maxy))
        self.scene.addItem(QtGui.QGraphicsLineItem(0,miny, 0, 0))
        self.scene.addItem(QtGui.QGraphicsLineItem(0,0, min(maxx, maxy), min(maxx, maxy)))
        self.scene.addItem(QtGui.QGraphicsLineItem(max(minx,miny), max(minx,miny), 0,0))

        # Dashed, gray integer lattice
        pen = QtGui.QPen(QtCore.Qt.DashLine)
        pen.setColor(QtCore.Qt.gray)
        for i in xrange(int(minx) + 1, int(maxx) + 1):
            line = QtGui.QGraphicsLineItem(i,0, i, maxy)
            line.setPen(pen)
            self.scene.addItem(line)
        for i in xrange(int(miny) + 1, int(maxy) + 1):
            line = QtGui.QGraphicsLineItem(0,i, maxx, i)
            line.setPen(pen)
            self.scene.addItem(line)


def show_diagram(dgm):
    app = QtGui.QApplication([])
    view = DiagramViewer(dgm)
    view.show()
    view.raise_()
    app.exec_()
    return view.selection
