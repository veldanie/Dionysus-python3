#ifndef __L2_DISTANCE_H__
#define __L2_DISTANCE_H__

#include <utilities/types.h>

#include <vector>
#include <fstream>
#include <functional>
#include <cmath>


typedef     std::vector<double>                                     Point;
typedef     std::vector<Point>                                      PointContainer;

struct L2Distance:
    public std::binary_function<const Point&, const Point&, double>
{
    result_type     operator()(const Point& p1, const Point& p2) const
    {
        AssertMsg(p1.size() == p2.size(), "Points must be in the same dimension (in L2Distance)");
        result_type sum = 0;
        for (size_t i = 0; i < p1.size(); ++i)
            sum += (p1[i] - p2[i])*(p1[i] - p2[i]);

        return sqrt(sum);
    }
};

void    read_points(const std::string& infilename, PointContainer& points, Dimension ambient)
{
    std::ifstream in(infilename.c_str());
    while(in)
    {
        points.push_back(Point());
        for (unsigned i = 0; i < ambient; ++i)
        {
            double      x;
            in >> x;
            points.back().push_back(x);
        }
    }
}

#endif // __L2_DISTANCE_H__
