#include "topology/filtration.h"
#include "topology/simplex.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <boost/program_options.hpp>

typedef 		SimplexWithValue<int> 			Simplex;
typedef			Filtration<Simplex>				SimplexFiltration;

namespace po = boost::program_options;

int main(int argc, char** argv)
{
    std::string infilename;

    // Parse program options
    po::options_description hidden("Hidden options");
    hidden.add_options()
        ("input-file",  po::value<std::string>(&infilename),
                        "Filtration filename");

    po::options_description visible("Allowed options");
    visible.add_options()
        ("help,h",      "produce help message");
    po::positional_options_description p;
    p.add("input-file", 1);
    
    po::options_description all; all.add(visible).add(hidden);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
                  options(all).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help") || !vm.count("input-file"))
    { 
        std::cout << "Usage: " << argv[0] << " [options] FILENAME" << std::endl;
        std::cout << visible << std::endl; 
        return 1; 
    }


	Evaluator<Simplex> e;
	SimplexFiltration::Vineyard v(&e);
	SimplexFiltration f(&v);

    std::ifstream in(infilename.c_str());
    unsigned int i = 0;
    std::string s;
    std::getline(in, s);
    while(in)
    {
        std::istringstream linestream(s);
        Simplex simplex(float(i++));
        unsigned int vertex;
        linestream >> vertex;
        while(linestream)
        {
            simplex.add(vertex);
            linestream >> vertex;
        }
        std::cout << simplex << std::endl;
        f.append(simplex);
        std::getline(in, s);
    }
	
    f.fill_simplex_index_map();
	f.pair_simplices();
	v.start_vines(f.begin(), f.end());
	
	std::cout << "Filtration size: " << f.size() << std::endl;
    for (SimplexFiltration::Index cur = f.begin(); cur != f.end(); ++cur)
        if (cur->sign()) 
            std::cout << cur->dimension() << " " 
                      << cur->get_value() << " " 
                      << cur->pair()->get_value() << std::endl;
}

