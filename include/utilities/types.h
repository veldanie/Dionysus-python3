#ifndef __TYPES_H__
#define __TYPES_H__

#include <limits>
#include <iostream>

/* Types */
typedef 	bool					Sign;
typedef		unsigned short int		Dimension;
const 		Sign	 				POS = true;
const 		Sign					NEG = false;
typedef		double					RealType;
typedef		unsigned int			SizeType;

static RealType Infinity = std::numeric_limits<RealType>::infinity();

typedef 	const unsigned int&		version_type;

struct      Empty                   {};
//std::ostream& operator<<(std::ostream& out, Empty e) { return out; }

enum        SwitchType
{
            DiffDim     = 0,
            Case1       = 0x4,
            Case12      = 0x5,
            Case112     = 0x6,
            Case2       = 0x8,
            Case212     = 0x9,
            Case3       = 0x10,
            Case31      = 0x11,
            Case4       = 0x20,
};


// Nothing to do for serializing Empty, but still need to provide this function
namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive & ar, Empty&, const unsigned int )
{}

} // namespace serialization
} // namespace boost


#endif // __TYPES_H__
