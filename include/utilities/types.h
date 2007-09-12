#ifndef __TYPES_H__
#define __TYPES_H__

#include <limits>

/* Types */
typedef 	bool					Sign;
typedef		short int				Dimension;
const 		Sign	 				POS = true;
const 		Sign					NEG = false;
typedef		double					RealType;
typedef		unsigned int			SizeType;

static RealType Infinity = std::numeric_limits<RealType>::infinity();

typedef 	const unsigned int&		version_type;

#endif // __TYPES_H__
