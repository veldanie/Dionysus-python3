#include <utilities/log.h>

#include <topology/simplex.h>
#include <topology/filtration.h>
#include "Miniball_dynamic_d.h"
#include <algorithm>

#include <iostream>
#include <fstream>


typedef 		std::vector<Point>										PointContainer;
typedef			unsigned int 											PointIndex;
typedef			SimplexWithValue<PointIndex>							Simplex;
typedef 		std::vector<Simplex> 									SimplexVector;
typedef 		Filtration<Simplex>										CechFiltration;

class DimensionValueComparison
{
	public:
		bool	operator()(const Simplex& s1, const Simplex& s2) const
		{
			if (s1.dimension() == s2.dimension())
				return s1.get_value() < s2.get_value();
			else
				return s1.dimension() < s2.dimension();
		}
};

int choose(int n, int k)
{
	if (k > n/2) k = n-k;
	int numerator = 1, denominator = 1;
	for (int i = 0; i < k; ++i)
	{ numerator *= (n-i); denominator *= (i+1); }
	return numerator/denominator;
}

void add_simplices(SimplexVector& sv, int d, const PointContainer& points)
{
	PointIndex indices[d+1];
	for (int i = 0; i < d+1; ++i) 
		indices[i] = d - i;

	while(indices[d] < points.size() - d)
	{
		// Add simplex
		Miniball mb(points[indices[0]].dim());
		Simplex s;
		for (int i = 0; i < d+1; ++i)
		{
			s.add(indices[i]);
			mb.check_in(points[indices[i]]);
		}
		mb.build();
		s.set_value(mb.squared_radius());
		sv.push_back(s);

		
		// Advance indices
		for (int i = 0; i < d+1; ++i)
		{
			++indices[i];
			if (indices[i] < points.size() - i)
			{
				for (int j = i-1; j >= 0; --j)
					indices[j] = indices[j+1] + 1;
				break;
			}
		}
	}
}

int main(int argc, char** argv) 
{
	SetFrequency(GetCounter("filtration/pair"), 10000);
	SetTrigger(GetCounter("filtration/pair"), GetCounter(""));

	// Read in the point set and compute its Delaunay triangulation
	std::istream& in = std::cin;
	int ambient_d, homology_d;	
	in >> ambient_d >> homology_d;
	
	std::cout << "Ambient dimension: " << ambient_d << std::endl;
	std::cout << "Will compute PD up to dimension: " << homology_d << std::endl;
	
	// Read points
	PointContainer points;
	while(in)
	{
		Point p(ambient_d);
		for (int i = 0; i < ambient_d; ++i)
			in >> p[i];
		points.push_back(p);
	}
	std::cout << "Points read: " << points.size() << std::endl;
   
	// Compute Cech values
	CechFiltration cf;
	{										// resource acquisition is initialization
		SimplexVector	sv;
		int num_simplices = 0;
		for (int i = 0; i <= homology_d + 1; ++i)
			num_simplices += choose(points.size(), i+1);
		sv.reserve(num_simplices);
		std::cout << "Reserved SimplexVector of size: " << num_simplices << std::endl;

		for (int i = 0; i <= homology_d + 1; ++i)
			add_simplices(sv, i, points);
		std::cout << "Size of SimplexVector: " << sv.size() << std::endl;
			
		std::sort(sv.begin(), sv.end(), DimensionValueComparison());
		
		for (SimplexVector::const_iterator cur = sv.begin(); cur != sv.end(); ++cur)
			cf.append(*cur);
	}

	// Compute persistence
	cf.fill_simplex_index_map();
	cf.pair_simplices(cf.begin(), cf.end(), false);
	std::cout << "Simplices paired" << std::endl;

	for (CechFiltration::Index i = cf.begin(); i != cf.end(); ++i)
		if (i->is_paired())
		{
			if (i->sign())
				std::cout << i->dimension() << " " << i->get_value() << " " << i->pair()->get_value() << std::endl;
		} //else std::cout << i->value() << std::endl;

}

