#include <utilities/log.h>

#include "topology/simplex.h"
#include "topology/filtration.h"
//#include "topology/static-persistence.h"
#include "topology/dynamic-persistence.h"
#include "topology/persistence-diagram.h"
#include <utilities/indirect.h>

#include <vector>
#include <map>
#include <iostream>


#if 1
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#endif

typedef         unsigned                        Vertex;
typedef         Simplex<Vertex, double>         Smplx;
typedef         std::vector<Smplx>              Complex;
typedef         Filtration<Complex, unsigned>   Fltr;
//typedef         StaticPersistence<>             Persistence;
typedef         DynamicPersistenceTrails<>      Persistence;
typedef         PersistenceDiagram<>            PDgm;

void fillTriangleSimplices(Complex& c)
{
    typedef std::vector<Vertex> VertexVector;
    VertexVector vertices(4);
    vertices[0] = 0; vertices[1] = 1; vertices[2] = 2; 
    vertices[3] = 0;
        
    VertexVector::const_iterator bg = vertices.begin();
    VertexVector::const_iterator end = vertices.end();
    c.push_back(Smplx(bg,     bg + 1, 0));                 // 0 = A
    c.push_back(Smplx(bg + 1, bg + 2, 1));                 // 1 = B
    c.push_back(Smplx(bg + 2, bg + 3, 2));                 // 2 = C
    c.push_back(Smplx(bg,     bg + 2, 2.5));               // AB
    c.push_back(Smplx(bg + 1, bg + 3, 2.9));               // BC
    c.push_back(Smplx(bg + 2, end,    3.5));               // CA
    c.push_back(Smplx(bg,     bg + 3, 5));                 // ABC

    std::sort(c.begin(), c.end(), Smplx::VertexComparison());
}

int main(int argc, char** argv)
{
#ifdef LOGGING
    rlog::RLogInit(argc, argv);

    stdoutLog.subscribeTo(RLOG_CHANNEL("topology/persistence"));
    //stdoutLog.subscribeTo(RLOG_CHANNEL("topology/chain"));
    //stdoutLog.subscribeTo(RLOG_CHANNEL("topology/vineyard"));
#endif

    Complex c;
    fillTriangleSimplices(c);
    std::cout << "Simplices filled" << std::endl;
    for (Complex::const_iterator cur = c.begin(); cur != c.end(); ++cur)
        std::cout << "  " << *cur << std::endl;

#if 1           // testing serialization of Complex (really Simplex)
  {  
    std::ofstream ofs("complex");
    boost::archive::text_oarchive oa(ofs);
    oa << c;
    c.clear();
  }  

  {
    std::ifstream ifs("complex");
    boost::archive::text_iarchive ia(ifs);
    ia >> c;
  }  
#endif

    Fltr f(c.begin(), c.end(), Smplx::DataComparison());
    std::cout << "Filtration initialized" << std::endl;
    std::cout << f << std::endl;

    Persistence p(f);
    std::cout << "Persistence initialized" << std::endl;

    p.pair_simplices();
    std::cout << "Simplices paired" << std::endl;

    std::map<Dimension, PDgm> dgms;
    init_diagrams(dgms, p.begin(), p.end(), 
                  evaluate_through_map(make_offset_map(p.begin(), f.begin()), 
                                       evaluate_through_filtration(f, Smplx::DataEvaluator())), 
                  evaluate_through_map(make_offset_map(p.begin(), f.begin()), 
                                       evaluate_through_filtration(f, Smplx::DimensionExtractor())));

    std::cout << 0 << std::endl << dgms[0] << std::endl;
    std::cout << 1 << std::endl << dgms[1] << std::endl;

    // Transpositions
    p.transpose(p.begin());         // transposition case 1.2 special

#if 0
    Smplx A;  A.add(0);
    std::cout << A << std::endl;
    std::cout << *tf.get_index(A) << std::endl;
    std::cout << "Transposing A: " << tf.transpose(tf.get_index(A)) << std::endl;
    std::cout << tf;

    Smplx BC; BC.add(1); BC.add(2);
    Smplx AB; AB.add(0); AB.add(1);
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

