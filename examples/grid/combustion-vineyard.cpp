/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2005
 */

#include <utilities/sys.h>
#include <utilities/debug.h>

#include <iostream>
#include <fstream>
#include <algorithm>

#include "grid2D.h"
#include "grid2Dvineyard.h"

const int xsize = 600;
const int ysize = 600;
const int var = 0;			// which variable to use out of nc of them in each record in the file

template<typename T>
void read(std::ifstream& ifs, T& var)
{
	ifs.read(reinterpret_cast<char*>(&var), sizeof(T));
}

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::cout << "Usage: combustion-vineyard FRAME1 FRAME2" << std::endl;
		exit(0);
	}
	
	int size0, nc0;
	int size1, nc1;

	std::cout << "Reading: " << argv[1] << std::endl;
	std::ifstream ifs0(argv[1], std::ios::binary);
	std::cout << "Reading: " << argv[2] << std::endl;
	std::ifstream ifs1(argv[2], std::ios::binary);

	if (!ifs0 || !ifs1)
	{
		std::cout << "Could not open the frames" << std::endl;
		exit(0);
	}

	read(ifs0, size0); read(ifs0, nc0);
	read(ifs1, size1); read(ifs1, nc1);

	assert(size0 == size1); assert(nc0 == nc1);
	assert(size0 == xsize*ysize);
	
	Grid2D g0(xsize, ysize), g1(xsize, ysize);
	
	for (int y = 0; y < ysize; ++y)
		for (int x = 0; x < xsize; ++x)
			for (int d = 0; d < nc0; ++d)
			{
				float val0, val1;
				read(ifs0, val0);
				read(ifs1, val1);
				if (d == var)
				{
					g0(x,y) = val0;
					g1(x,y) = val1;
				}
			}
	std::cout << "Grids read" << std::endl;
	
	// Generate filtration and compute pairing
	Grid2DVineyard v(&g0);
	std::cout << "Filtration generated, size: " << v.filtration()->size() << std::endl;
	v.compute_pairing();
	std::cout << "Pairing computed" << std::endl;
	
	// Compute vineyard
	v.compute_vineyard(&g1, true);
	std::cout << "Vineyard computed" << std::endl;

	v.vineyard()->save_edges("combustion");
}
