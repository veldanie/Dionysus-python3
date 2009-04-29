from    math        import sqrt

def l2(p):
    return sqrt(                                        # take the square root
            reduce(lambda x, y: x + y,                  # add them all up
                   map(lambda x: x**2, p)))             # square each coordinate

# Pairwise distances between the elements of `points` with respect to some `norm`
class PairwiseDistances:
    def __init__(self, points, norm = l2):
        self.points = points
        self.norm = norm

    def __len__(self):
        return len(self.points)

    def __call__(self, p1, p2):
        return self.norm((x - y for (x,y) in zip(self.points[p1], self.points[p2])))

# Caches all distances specified by `distances`
class ExplicitDistances:
    def __init__(self, distances):
        self.len = len(distances)
        self.distances = []
        for i in xrange(self.len): 
            self.distances.append([])
            for j in xrange(self.len):
                self.distances[-1].append(distances(i,j))

    def __len__(self):
        return self.len

    def __call__(self, p1, p2):
        return self.distances[p1][p2]

# Generator of all points in a file `filename` with one point per line
def points_file(filename):
    fd = open(filename)
    for line in fd.xreadlines():
        yield map(float, line.strip().split())
    fd.close()
