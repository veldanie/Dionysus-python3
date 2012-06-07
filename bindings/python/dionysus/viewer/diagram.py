from    PyQt4       import QtGui, QtCore

class DiagramPoint(QtGui.QGraphicsEllipseItem):
    def __init__(self,x,y,radius, p):
        super(QtGui.QGraphicsEllipseItem, self).__init__(x - radius,y - radius,2*radius, 2*radius)
        self.p = p

    # for debugging purposes
    def color(self):
        pen = QtGui.QPen()
        pen.setColor(QtCore.Qt.red)
        self.setPen(pen)

class DiagramViewer(QtGui.QGraphicsView):
    def __init__(self, dgm):
        super(QtGui.QGraphicsView, self).__init__()

        self.selection = None

        self.setRenderHint(QtGui.QPainter.Antialiasing)
        self.scene = QtGui.QGraphicsScene(self)
        self.setScene(self.scene)

        inf = float('inf')
        minx = min(p[0] for p in dgm)
        miny = min(p[1] for p in dgm)
        maxx = max(p[0] for p in dgm if p[0] != inf)
        maxy = max(p[1] for p in dgm if p[1] != inf)

        radius = max(.005, min(maxx - minx, maxy - miny)/200)
        border = 25
        self.scene.setSceneRect(minx - border*radius, miny - border*radius, (maxx - minx) + 2*border*radius, (maxy - miny) + 2*border*radius)

        self.draw_axes(minx,miny,maxx,maxy)

        for p in dgm:
            x,y = p[0],p[1]
            if x == inf or y == inf: continue
            item = DiagramPoint(x,y,radius, p)
            self.scene.addItem(item)

        # Flip y-axis
        self.scale(1,-1)

        # Set the correct view
        rect = self.scene.itemsBoundingRect()
        self.fitInView(rect, QtCore.Qt.KeepAspectRatio)

    def mousePressEvent(self, event):
        p = self.mapToScene(event.pos())
        item = self.scene.itemAt(p)
        if isinstance(item, DiagramPoint):
            #item.color()
            self.selection = item.p
            self.close()

    def draw_axes(self, minx, miny, maxx, maxy):
        # Draw axes and diagonal
        if maxx > 0:
            self.scene.addItem(QtGui.QGraphicsLineItem(0,0, maxx, 0))
        if minx < 0:
            self.scene.addItem(QtGui.QGraphicsLineItem(minx,0, 0, 0))
        if maxy > 0:
            self.scene.addItem(QtGui.QGraphicsLineItem(0,0, 0, maxy))
        if miny < 0:
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


def show_diagram(dgm, app):
    #app = QtGui.QApplication([])
    view = DiagramViewer(dgm)
    view.show()
    view.raise_()
    app.exec_()
    return view.selection
