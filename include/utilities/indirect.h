#ifndef __INDIRECT_H__
#define __INDIRECT_H__

#include <boost/iterator/iterator_adaptor.hpp>

// TODO: write documentation


template<class Iterator_, class Comparison_>
struct IndirectComparison
{
    typedef         Iterator_                               Iterator;
    typedef         Comparison_                             Comparison;
    
                    IndirectComparison(const Comparison& cmp):
                        cmp_(cmp)
                    {}

    bool            operator()(Iterator a, Iterator b) const
    { return cmp_(*a, *b); }

    const Comparison&               cmp_;
};

template<class Comparison>
struct ThreeOutcomeCompare: public Comparison
{
    typedef             typename Comparison::first_argument_type                            first_argument_type;
    typedef             typename Comparison::second_argument_type                           second_argument_type;

    ThreeOutcomeCompare(const Comparison& cmp = Comparison()): Comparison(cmp)              {}

    int                     compare(const first_argument_type& a, const second_argument_type& b) const          
    {   if (operator()(a,b))        return -1;
        else if (operator()(b,a))   return 1;
        else                        return 0;
    }
};

template<class Iterator_>
class RecursiveIterator: public boost::iterator_adaptor<RecursiveIterator<Iterator_>,       // Derived
                                                        Iterator_,                          // Base
                                                        Iterator_>                          // Value
{
    private:
        struct      enabler                                                 {};

    public:
        typedef     Iterator_                                               Iterator;
        typedef     boost::iterator_adaptor<RecursiveIterator<Iterator>, 
                                                              Iterator, 
                                                              Iterator>     Parent;

                    RecursiveIterator()                                     {}
        explicit    RecursiveIterator(Iterator iter):
                        Parent(iter)                                        {}
    
    private:
        friend class    boost::iterator_core_access;
        typename Parent::reference       
                        dereference() const                                 { return const_cast<typename Parent::reference>(this->base()); }
        // FIXME: I dislike to const_cast, but it's not obvious how to get rid of it
};


#endif // __INDIRECT_H__
