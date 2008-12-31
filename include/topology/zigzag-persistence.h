#ifndef __ZIGZAG_PERSISTENCE_H__
#define __ZIGZAG_PERSISTENCE_H__

#include <list>
#include "cycles.h"
#include "utilities/types.h"
#include <sstream>

/**
 * Class: ZigzagPersistence
 * TODO: this should probably be parametrized by Chain or Field
 */
template<class BirthID_ = Empty<> >
class ZigzagPersistence
{
    public:
        typedef                         BirthID_                                BirthID;

        struct ZNode;
        struct BNode;
        struct SimplexNode;

        typedef                         std::list<ZNode>                                ZList;
        typedef                         typename ZList::iterator                        ZIndex;
        typedef                         std::list<BNode>                                BList;
        typedef                         typename BList::iterator                        BIndex;
        typedef                         std::list<SimplexNode>                          SimplexList;
        typedef                         typename SimplexList::iterator                  SimplexIndex;

        // TODO: should all chains be DequeChains? probably not
        typedef                         typename DequeChains<ZIndex>::Chain             ZRow;
        typedef                         typename DequeChains<ZIndex>::Chain             BColumn;
        typedef                         typename VectorChains<BIndex>::Chain            BRow;
        typedef                         typename VectorChains<BIndex>::Chain            CRow;
        typedef                         typename VectorChains<SimplexIndex>::Chain      ZColumn;
        typedef                         typename VectorChains<SimplexIndex>::Chain      CColumn;

        typedef                         boost::optional<BirthID>                        Death;
        typedef                         std::pair<SimplexIndex, Death>                  IndexDeathPair;

        // TODO: probably should store something to identify the birth to the outside world; this should probably
        //       be a template parameter (perhaps a template parameter to ZigzagPersistence)
        struct ZNode
        {
                                        ZNode(int o, const BirthID& b): 
                                            order(o), birth(b)                          {}

            int                         order;
            ZColumn                     z_column;
            BRow                        b_row;
            BIndex                      low;            // which BColumn has this ZIndex as low

            BirthID                     birth;          // TODO: need to do empty-member optimization
        };

        struct BNode
        {
                                        BNode(unsigned o): order(o)                     {}

            unsigned                    order;
            BColumn                     b_column;
            CColumn                     c_column;
        };

        struct SimplexNode
        {
                                        SimplexNode(unsigned o): order(o)               {}

            unsigned                    order;
            ZRow                        z_row;
            CRow                        c_row;
            ZIndex                      low;            // which ZColumn has this SimplexNode as low
        };

        // Constructor: ZigzagPersistence()
                                        ZigzagPersistence()                             {}
 
        // Function: add(s)
        IndexDeathPair                  add(ZColumn bdry, const BirthID& birth = BirthID());

        // Function: remove(s)
        Death                           remove(SimplexIndex s, const BirthID& birth = BirthID());

        // Debug
        void                            show_all();

    private:
        ZList                           z_list;
        BList                           b_list;
        SimplexList                     s_list;

        /* Helper functors */
        template<class Member, class Element>                                           struct Appender;
        template<class Member, class Element>                                           struct Remover;
        template<class Member, class Chain>                                             struct Adder;

        template<class Member, class Element>
        Appender<Member, Element>       make_appender(Member m, Element e) const        { return Appender<Member, Element>(m,e); }
        template<class Member, class Element>
        Remover<Member, Element>        make_remover(Member m, Element e) const         { return Remover<Member, Element>(m,e); }
        template<class Member, class Chain>
        Adder<Member, Chain>            make_adder(Member m, Chain& c) const            { return Adder<Member, Chain>(m, c); }

        template<class Index, class IndexFrom, class PrimaryMember, class SecondaryMember>
        void                            add_chains(Index bg, Index end, IndexFrom j, PrimaryMember pm, SecondaryMember sm);
        template<class IndexTo, class IndexFrom, class PrimaryMemberTo, class SecondaryMemberTo, class PrimaryMemberFrom>
        void                            add_chains(IndexTo bg, IndexTo end, IndexFrom j, 
                                                   PrimaryMemberTo   pmt, SecondaryMemberTo smt,
                                                   PrimaryMemberFrom pmf);
        template<class Index, class PrimaryMember, class SecondaryMember>
        void                            add_chain(Index to, Index from, 
                                                  PrimaryMember   pmt, SecondaryMember smt);
        template<class IndexTo, class IndexFrom, class PrimaryMemberTo, class SecondaryMemberTo, class PrimaryMemberFrom>
        void                            add_chain(IndexTo to, IndexFrom from, 
                                                  PrimaryMemberTo   pmt, SecondaryMemberTo smt,
                                                  PrimaryMemberFrom pmf);
        template<class IndexTo, class IndexFrom, class PrimaryMember, class SecondaryMember, class DualPrimaryMember, class DualSecondaryMember>
        void                            change_basis(IndexTo bg, IndexTo end, IndexFrom j, 
                                                     PrimaryMember pm, SecondaryMember sm, 
                                                     DualPrimaryMember dpm, DualSecondaryMember dsm);

    public:
        struct OrderComparison
        {
            template<class T> 
            bool                        operator()(T a, T b) const                      { return a->order < b->order; }
        }                               cmp;
        
        struct OrderOutput
        {
            template<class T> 
            std::string                 operator()(T a) const                           { std::stringstream s; s << a->order; return s.str(); }
        }                               out;
};

#include "zigzag-persistence.hpp"

#endif // __ZIGZAG_PERSISTENCE_H__
