/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2005 -- 2006
 *
 * Depends on Daniel Russel's "simple C++ PDB" (aka DSR-PDB).
 */

#ifndef __PDBDISTANCE_H__
#define __PDBDISTANCE_H__

#include <fstream>
#include <string>
#include <dsrpdb/Protein.h>
#include <dsrpdb/iterator.h>
#include <cmath>

#include <boost/serialization/access.hpp>

#include "types.h"
#include "grid2D.h"

#include <boost/serialization/export.hpp>


class PDBDistanceGrid: public Grid2D
{
	public:
		PDBDistanceGrid()
		{}
	
		PDBDistanceGrid(std::istream& in)
		{
			load_stream(in);
		}

		void	load_stream(std::istream& in)
		{
			dsrpdb::Protein p(in);
			std::vector<dsrpdb::Point> CAs(ca_coordinates_begin(p), ca_coordinates_end(p));
			std::cout << "CAs created, size: " << CAs.size() << std::endl;

			Grid2D::change_dimensions(CAs.size(), CAs.size());
			for (Grid2D::CoordinateIndex i = 0; i < CAs.size(); ++i)
				for (Grid2D::CoordinateIndex j = 0; j < CAs.size(); ++j)
				{
					if (i < j)
						Grid2D::operator()(i,j) = distance(CAs[i], CAs[j]);
					else
						Grid2D::operator()(i,j) = 0;
				}
		}

	private:
		Grid2D::ValueType distance(dsrpdb::Point p1, dsrpdb::Point p2) const
		{
			dsrpdb::Vector v = p1 - p2;
			return std::sqrt(v*v);
		}

#if 0
	private:
		// Serialization
		friend class boost::serialization::access;
		
		template<class Archive>
		void serialize(Archive& ar, version_type version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Grid2D);
		}
#endif
};

//BOOST_CLASS_EXPORT(PDBDistanceGrid)

#endif // __PDBDISTANCE_H__
