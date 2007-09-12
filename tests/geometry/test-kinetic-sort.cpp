#include <polynomial.h>
#include <simulator.h>
#include <kinetic-sort.h>
#include <iostream>

#include <boost/utility.hpp>

//typedef		double							FieldType;
//typedef		ZZ								FieldType;
typedef		QQ								FieldType;
typedef 	UPolynomial<FieldType>			PolyKernel;
typedef		PolyKernel::Polynomial			Polynomial;
typedef 	std::list<Polynomial>			SortDS;
typedef		Simulator<PolyKernel>			SimulatorFT;

class TrajectoryExtractor
{
	public:
		Polynomial		operator()(SortDS::iterator i) const			{ return *i; }
};

typedef		KineticSort<SortDS, TrajectoryExtractor, SimulatorFT>		KineticSortDS;

struct EvaluatedComparison: public std::binary_function<const Polynomial&, const Polynomial&, bool>
{
								EvaluatedComparison(FieldType v): vv(v)	{}
	bool						operator()(const Polynomial& p1, const Polynomial& p2)				
								{ return p1(vv) < p2(vv); }
	FieldType					vv;
};

void swap(SortDS* s, SortDS::iterator i)
{
	std::cout << "Swapping " << *i << " " << *boost::next(i) << std::endl;
	s->splice(i, *s, boost::next(i));
}

int main()
{
	SimulatorFT		simulator;
	SortDS 			list;

	// Insert polynomials and sort the list for current time
	list.push_back(Polynomial("x^3 - 3"));
	list.push_back(Polynomial("x^2 - 2*x - 2"));
	list.push_back(Polynomial("2*x - 4"));
	//list.sort(EvaluatedComparison(simulator.current_time()));
	list.sort(EvaluatedComparison(0));

	// Print out the list
	for (SortDS::const_iterator cur = list.begin(); cur != list.end(); ++cur)
		std::cout << *cur << std::endl;

	// Setup kinetic sort
	KineticSortDS	ks(&list, &simulator, swap);

	while(!simulator.reached_infinity() && simulator.current_time() < 1)
	{
		//std::cout << "Current time before: " << simulator.current_time() << std::endl;
		if (!ks.audit(&simulator)) return 1;
		//simulator.print(std::cout << "Auditing ");
		simulator.process();
		//std::cout << "Current time after: " << simulator.current_time() << std::endl;
	}

}
