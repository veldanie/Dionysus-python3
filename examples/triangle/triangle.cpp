#include "vineyard.h"
#include "simplex.h"
#include <vector>
#include <iostream>

typedef 		SimplexWithValue<int> 		Simplex;
typedef			Vineyard<Simplex>			TriangleFiltration;
//typedef			Filtration<Simplex>			TriangleFiltration;

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

	f.fill_simplex_index_map();
}

int main()
{
#ifdef CWDEBUG
	dionysus::debug::init();

	Debug(dc::filtration.on());
	Debug(dc::cycle.on());
	Debug(dc::vineyard.on());
#endif

	TriangleFiltration tf;
	fillTriangleSimplices(tf);
	
	tf.pair_simplices(tf.begin());
	
	std::cout << "Filtration size: " << tf.size() << std::endl;
	std::cout << tf << std::endl;

#if 1
	Simplex BC; BC.add(1); BC.add(2);
	Simplex AB; AB.add(0); AB.add(1);
	std::cout << BC << std::endl;
	std::cout << *tf.get_index(BC) << std::endl;
	tf.transpose(tf.get_index(BC));
	std::cout << tf;
	tf.transpose(tf.get_index(AB));
	std::cout << tf;
#endif
}

