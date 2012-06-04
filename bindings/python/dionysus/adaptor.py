from _dionysus import CohomologyPersistence, PersistenceDiagram

class StaticCohomologyPersistence(object):
    def __init__(self, filtration, prime = 2, subcomplex = lambda s: True):
        self.filtration = filtration
        self.subcomplex = subcomplex
        self.persistence = CohomologyPersistence(prime)
        self.pairs = []

    def pair_simplices(self):
        indices = []
        for i,s in enumerate(self.filtration):
            sc = self.subcomplex(s)
            boundary = (indices[self.filtration(ss)] for ss in s.boundary)
            idx,d = self.persistence.add(boundary, i, image = sc)
            indices.append(idx)
            self.pairs.append([i, sc, []])
            if d:                           # Death
                if self.pairs[d][1]:        # Birth was in the subcomplex
                    self.pairs[i][0] = d    # i killed d
                    self.pairs[d][0] = i    # d was killed by i
            else:
                cocycle = self.persistence.__iter__().next()
                self.pairs[-1][2] = [(n.coefficient, n.si.order) for n in cocycle]


    def __iter__(self):
        for i, (pair, subcomplex, cocycle) in enumerate(self.pairs):
            if pair == i:       # unpaired
                if subcomplex:
                    yield APNode(i, self.pairs)
            else:
                if pair > i and subcomplex:
                    yield APNode(i, self.pairs)
                elif pair < i:
                    pair_pair, pair_subcomplex, pair_cocycle = self.pairs[pair]
                    if pair_subcomplex:
                        yield APNode(i, self.pairs)

    def make_simplex_map(self, filtration):
        return APSimplexMap(filtration)

class ImagePersistence(StaticCohomologyPersistence):
    def __init__(self, filtration, subcomplex):
        super(ImagePersistence, self).__init__(filtration, subcomplex = subcomplex)

# Remaps APNodes into Simplices
class APSimplexMap:
    def __init__(self, filtration):
        self.filtration = filtration

    def __getitem__(self, n):
        return self.filtration[n.i]

class APNode:
    def __init__(self, i, pairs):
        self.i = i
        self.pairs = pairs

    def sign(self):
        return self.unpaired() or self.i < self._pair()

    def unpaired(self):
        return self.i == self._pair()

    def _pair(self):
        return self.pairs[self.i][0]

    def pair(self):
        return APNode(self._pair(), self.pairs)

    def cocycle(self):
        return self.pairs[self.i][2]

def init_diagrams_from_adaptor(p, f, evaluator, data):
    if not evaluator:
        evaluator = lambda s: s.data

    if not data:
        data = lambda s: None

    dgms = []
    smap = p.make_simplex_map(f)
    for n in p:
        if not n.sign(): continue

        dim = smap[n].dimension()
        if dim + 1 > len(dgms):
            dgms.append(PersistenceDiagram(dim))

        b = evaluator(smap[n])
        d = evaluator(smap[n.pair()]) if not n.unpaired() else float('inf')

        dgms[dim].append((b,d, data(smap[n])))

    return dgms
