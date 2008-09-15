#include <utilities/log.h>

#include "topology/filtration.h"
#include "topology/simplex.h"
#include <vector>
#include <iostream>

typedef 		SimplexWithValue<int> 			Simplex;
typedef			Filtration<Simplex>				TriangleFiltration;

void fillTriangleSimplices(TriangleFiltration& f)
{
	typedef std::vector<int> VertexVector;
	VertexVector vertices(4);
	vertices[0] = 0; vertices[1] = 1; vertices[2] = 2; 
	vertices[3] = 0;
		
	VertexVector::const_iterator bg = vertices.begin();
	VertexVector::const_iterator end = vertices.end();
	f.append(Simplex(bg,     bg + 1, 0));				// 0 = A
	f.append(Simplex(bg + 1, bg + 2, 1));				// 1 = B
	f.append(Simplex(bg + 2, bg + 3, 2));				// 2 = C
	f.append(Simplex(bg,     bg + 2, 2.5));				// AB
	f.append(Simplex(bg + 1, bg + 3, 2.9));				// BC
	f.append(Simplex(bg + 2, end, 3.5));				// CA
	f.append(Simplex(bg,     bg + 3, 5));				// ABC
}

int main(int argc, char** argv)
{
#ifdef LOGGING
	rlog::RLogInit(argc, argv);

	stdoutLog.subscribeTo(RLOG_CHANNEL("topology/filtration"));
	//stdoutLog.subscribeTo(RLOG_CHANNEL("topology/cycle"));
	//stdoutLog.subscribeTo(RLOG_CHANNEL("topology/vineyard"));
#endif

	Evaluator<Simplex> e;
	TriangleFiltration::Vineyard v(&e);
	TriangleFiltration tf(&v);
	fillTriangleSimplices(tf);
	
	tf.fill_simplex_index_map();
	tf.pair_simplices();
	v.start_vines(tf.begin(), tf.end());
	
	std::cout << "Filtration size: " << tf.size() << std::endl;
	std::cout << tf << std::endl;

#if 1
	Simplex A;  A.add(0);
	std::cout << A << std::endl;
	std::cout << *tf.get_index(A) << std::endl;
    std::cout << "Transposing A: " << tf.transpose(tf.get_index(A)) << std::endl;
	std::cout << tf;

	Simplex BC; BC.add(1); BC.add(2);
	Simplex AB; AB.add(0); AB.add(1);
	std::cout << BC << std::endl;
	std::cout << *tf.get_index(BC) << std::endl;
	tf.transpose(tf.get_index(BC));
	std::cout << tf;
	std::cout << AB << std::endl;
	std::cout << *tf.get_index(AB) << std::endl;
	tf.transpose(tf.get_index(AB));
	std::cout << tf;
#endif
}

