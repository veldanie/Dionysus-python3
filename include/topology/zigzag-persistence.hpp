#include <utilities/log.h>
#include <boost/utility.hpp>
#include <algorithm>
#include <utilities/indirect.h>
#include <functional>

#ifdef LOGGING
static rlog::RLogChannel* rlZigzagAdd =                   DEF_CHANNEL("topology/persistence/zigzag/add",        rlog::Log_Debug);
static rlog::RLogChannel* rlZigzagRemove =                DEF_CHANNEL("topology/persistence/zigzag/remove",     rlog::Log_Debug);
static rlog::RLogChannel* rlZigzagAddChain =              DEF_CHANNEL("topology/persistence/zigzag/addchain",   rlog::Log_Debug);
static rlog::RLogChannel* rlZigzagCheckConsistency=       DEF_CHANNEL("topology/persistence/zigzag/check",      rlog::Log_Debug);
#endif // LOGGING


template<class BID>
typename ZigzagPersistence<BID>::IndexDeathPair
ZigzagPersistence<BID>::
add(ZColumn bdry, const BirthID& birth)
{
    rLog(rlZigzagAdd,       "Entered ZigzagPersistence::add()");
    rLog(rlZigzagAdd,       "  Boundary: %s", bdry.tostring(out).c_str());
    rLog(rlZigzagAdd,       "  Boundary size: %d", bdry.size());
    AssertMsg(check_consistency(), "Must be consistent before addition");

    {   // scoping to not pollute with the name order
        unsigned order      = s_list.empty() ? 0 : boost::prior(s_list.end())->order + 1;
        s_list.push_back(SimplexNode(order, z_list.end()));
    }
    SimplexIndex last_s     = boost::prior(s_list.end());
    last_s->low             = z_list.end();
#if !NDEBUG
    last_s->boundary        = bdry;     // NB: debug only    
#endif

    rLog(rlZigzagAdd,   "  Reducing among cycles");
    // Reduce bdry among the cycles
    rLog(rlZigzagAdd,       "    Boundary: %s", bdry.tostring(out).c_str());
    BColumn v;                // representation of the boundary in the cycle basis
    while (!bdry.empty())
    {
        SimplexIndex l      = bdry.back();
        ZIndex k            = l->low;
        v.append(k, cmp);
        bdry.add(k->z_column, cmp);
        rLog(rlZigzagAdd,       "    Boundary: %s", bdry.tostring(out).c_str());
    }
    rLog(rlZigzagAdd,   "  Reduced among cycles");

    // Reduce v among boundaries
    BRow u;
    while (!(v.empty()))
    {
        ZIndex l = v.back();
        BIndex k = l->low;

        if (k == b_list.end())
            break;

        u.append(k, cmp);
        v.add(k->b_column, cmp);
    }
    rLog(rlZigzagAdd,   "  Reduced among boundaries");

    if (v.empty())
    {
        rLog(rlZigzagAdd,       "  Birth case in add");

        // Birth
        int order                   = z_list.empty() ? 0 : boost::prior(z_list.end())->order + 1;
        z_list.push_back(ZNode(order, birth, b_list.end()));
        ZIndex last_z               = boost::prior(z_list.end());

        // Set z_column
        ZColumn& z                  = last_z->z_column;
        std::for_each(u.begin(), u.end(), make_adder(&BNode::c_column, z));
        z.append(last_s, cmp);

        // Set s_row
        std::for_each(z.begin(), z.end(), make_appender(&SimplexNode::z_row, last_z));

        // Set low
        last_z->low                 = b_list.end();
        last_s->low                 = last_z;

        return std::make_pair(last_s, Death());
    } else
    {
        rLog(rlZigzagAdd,       "  Death case in add");

        // Death
        unsigned order              = b_list.empty() ? 0 : boost::prior(b_list.end())->order + 1;
        b_list.push_back(BNode(order));
        BIndex last_b               = boost::prior(b_list.end());
        
        // Set b_column and low
        last_b->b_column.swap(v);            
        last_b->b_column.back()->low = last_b;

        // Set b_row
        std::for_each(last_b->b_column.begin(), last_b->b_column.end(), make_appender(&ZNode::b_row, last_b));

        // Set c_column
        CColumn& c                  = last_b->c_column;
        std::for_each(u.begin(), u.end(), make_adder(&BNode::c_column, c));
        c.append(last_s, cmp);

        // Set c_row
        std::for_each(c.begin(), c.end(), make_appender(&SimplexNode::c_row, last_b));

        return std::make_pair(last_s, Death(last_b->b_column.back()->birth));
    }
}


template<class BID>
typename ZigzagPersistence<BID>::Death
ZigzagPersistence<BID>::
remove(SimplexIndex s, const BirthID& birth)
{
    rLog(rlZigzagRemove,        "Entered ZigzagPersistence::remove(%d)", s->order);
    AssertMsg(check_consistency(), "Must be consistent before removal");

    if (s->z_row.empty())
    {
        AssertMsg(!(s->c_row.empty()),  "Birth after removal, row in C must be non-empty");

        // Birth
        //show_all();
        rLog(rlZigzagRemove,        "Birth case in remove");
        
        int order                   = z_list.empty() ? 0 : z_list.begin()->order - 1; 
        z_list.push_front(ZNode(order, birth, b_list.end()));
        ZIndex first_z              = z_list.begin();
        ZColumn& z                  = first_z->z_column;
        first_z->low                = b_list.end();
        
        // Prepend DC[j] = ZB[j] to Z
        rLog(rlZigzagRemove,        "Computing the column DC[j] = ZB[j] to prepend to Z");
        BIndex j                    = s->c_row.front();
        rLog(rlZigzagRemove,        "  j = %d", j->order);
        std::for_each(j->b_column.begin(), j->b_column.end(), make_adder(&ZNode::z_column, z));
        std::for_each(z.begin(),           z.end(),           make_appender(&SimplexNode::z_row, first_z));
        rLog(rlZigzagRemove,        "  Prepended %d [%s]", first_z->order, z.tostring(out).c_str());
        //AssertMsg(check_consistency(),  "Must be consistent after prepending DC[j] = ZB[j] to Z");

        // Prepend row of s in C to B
        rLog(rlZigzagRemove,        "Prepending the row of s to B");
        first_z->b_row = s->c_row;      // copying instead of swapping is inefficient, 
                                        // but it simplifies logic when subtracting chains later
        std::for_each(first_z->b_row.begin(), first_z->b_row.end(), make_appender(&BNode::b_column, first_z));
        //AssertMsg(check_consistency(),  "Must be consistent after prepending row of s to B");

#if !NDEBUG
        {
            ZColumn zz;
            std::for_each(j->b_column.begin(), j->b_column.end(), make_adder(&ZNode::z_column, zz));
            AssertMsg(zz.empty(),       "ZB[j] must be 0 after we prepended the row of s in C to B");
        }
#endif

        // Subtract C[j] from every column of C that contains s
        AssertMsg(s->c_row == first_z->b_row,   "s->c_row == first_z->b_row before subtracting C[j]");
        rLog(rlZigzagRemove,        "Subtracting C[j]=[%s] from every column of C that contains s=%d with row [%s]",
                                    j->c_column.tostring(out).c_str(), 
                                    s->order, s->c_row.tostring(out).c_str());
        add_chains(first_z->b_row.rbegin(), first_z->b_row.rend(), j, &BNode::c_column, &SimplexNode::c_row);
        AssertMsg(check_consistency(s_list.end(), z_list.begin(), b_list.end()),  "Must be consistent after subtracting C[j] in remove::birth");

        // Subtract B[j] from every other column of B that has l
        ZIndex l                    = j->b_column.back();
        BRow   l_row                = l->b_row;
        rLog(rlZigzagRemove,    "Subtracting B[j], j is %d, l is %d, l_row: [%s]", 
                                j->order, l->order, l_row.tostring(out).c_str());
        typedef         std::not_equal_to<BIndex>       NotEqualBIndex;
        add_chains(boost::make_filter_iterator(std::bind2nd(NotEqualBIndex(), j), l_row.rbegin(), l_row.rend()),
                   boost::make_filter_iterator(std::bind2nd(NotEqualBIndex(), j), l_row.rend(),   l_row.rend()),
                   j, &BNode::b_column, &ZNode::b_row);
        j->b_column.back()->low = b_list.end();     // redundant since l will be deleted (here for check_consistency only)
        add_chain(j, j, &BNode::b_column, &ZNode::b_row);
        AssertMsg(check_consistency(s_list.end(), first_z, b_list.end()),  "Must be consistent after subtracting B[j] in remove::birth");


        // Drop j, l, and s
        // 
        // l->z_column is the only non-empty thing, but we drop it,
        // the basis is preserved because we added first_z
        l->z_column.back()->low     = z_list.end();
        std::for_each(l->z_column.begin(), l->z_column.end(), make_remover(&SimplexNode::z_row, l));

        //show_all();
        rLog(rlZigzagRemove,        "l=%d has z_column: [%s]", l->order, l->z_column.tostring(out).c_str());

        AssertMsg(l->b_row.empty(),     "b_row of l must be empty before erasing in remove::birth");
        AssertMsg(s->z_row.empty(),     "z_row of s must be empty before erasing in remove::birth");
        AssertMsg(s->c_row.empty(),     "c_row of s must be empty before erasing in remove::birth");
        b_list.erase(j);
        z_list.erase(l);
        s_list.erase(s);
        AssertMsg(check_consistency(s_list.end(), first_z, b_list.end()),  "Must be consistent before reducing Z in remove::birth");

        // Reduce Z
        rLog(rlZigzagRemove,        "Reducing Z");
        SimplexIndex ls = first_z->z_column.back();
        while(ls->low != first_z)
        {
            if (ls->low == z_list.end())    { ls->low = first_z; break; }

            // if ls->low precedes first_z, swap them
            if (cmp(ls->low, first_z))      std::swap(ls->low, first_z);
            
            add_chain(first_z, ls->low, &ZNode::b_row, &BNode::b_column);
            add_chain(ls->low, first_z, &ZNode::z_column, &SimplexNode::z_row);
            std::swap(ls->low, first_z);

            ls = first_z->z_column.back();
        }
        AssertMsg(check_consistency(),  "Must be consistent at the end of birth case in remove");

        return Death();
    } else
    {
        // Death
        rLog(rlZigzagRemove,        "Death case in remove");

        ZIndex j                    = s->z_row.front();
        CRow c_row                  = s->c_row;
        
        // Subtract Z[j] from every chain in C that contains s
        // (it's Ok to go in the forward order since we are subtracting a column in Z from C)
        add_chains(c_row.begin(), c_row.end(), j, &BNode::c_column, &SimplexNode::c_row, &ZNode::z_column);
        AssertMsg(check_consistency(),  "Must be consistent after subtracting Z[j] from C");
        
        // Change basis to remove s from Z 
        // Compute reducers --- columns that we will be adding to other columns
        ZRow z_row                  = s->z_row;
        typedef typename ZRow::reverse_iterator             ZRowReverseIterator;
        typedef std::list<ZRowReverseIterator>              ReducersContainer;
        ReducersContainer  reducers;                        // list of ZColumns that we will be adding to other columns
        reducers.push_back(boost::prior(z_row.rend()));     // j is the first reducer
        AssertMsg(*(reducers.back()) == j,  "The first element of reducers should be j");
        SimplexIndex low            = j->z_column.back();
        rLog(rlZigzagRemove,        "   Added reducer %d [%s] with low=%d", 
                                    j->order, j->z_column.tostring(out).c_str(),
                                    low->order);
        for (typename ZRow::iterator cur = z_row.begin(); cur != z_row.end(); ++cur)
            if (cmp((*cur)->z_column.back(), low))
            { 
                reducers.push_back(ZRowReverseIterator(boost::next(cur))); 
                low = (*(reducers.back()))->z_column.back();
                rLog(rlZigzagRemove,        "   Added reducer %d [%s] with low=%d", 
                                            (*cur)->order, (*cur)->z_column.tostring(out).c_str(),
                                            low->order);
                rLog(rlZigzagRemove,        "   reducers.back(): %d [%s] with low=%d", 
                                            (*(reducers.back()))->order,
                                            (*(reducers.back()))->z_column.tostring(out).c_str(),
                                            (*(reducers.back()))->z_column.back()->order);
            }
        rLog(rlZigzagRemove,        " Reducers size: %d, s is %d", 
                                    reducers.size(), s->order);

        //show_all();

        // Add each reducer to the columns that follow them until the next reducer
        typename ReducersContainer::reverse_iterator    cur     = reducers.rbegin();
        ZRowReverseIterator                             zcur    = z_row.rbegin();
        
        while (cur != reducers.rend())
        {
            rLog(rlZigzagRemove,        " Cur reducer: %d [%s]", (**cur)->order,
                                                                 (**cur)->z_column.tostring(out).c_str());
            change_basis(zcur, *cur, **cur, 
                         &ZNode::z_column, &SimplexNode::z_row,
                         &ZNode::b_row,    &BNode::b_column);
            if (cur != reducers.rbegin())
            {
                AssertMsg((*zcur)->z_column.back() == (**cur)->z_column.back(),
                          "The back of the z_columns must be the same.");
                (*zcur)->z_column.back()->low = *zcur;
            }
            else
                (**cur)->z_column.back()->low = z_list.end();

            zcur = *cur++;
            // This makes it inconsistent until the next iteration of this update loop
        }
        
        // Drop j and s
        BirthID birth               = j->birth;
        if (j->z_column.back()->low == j)
            j->z_column.back()->low = z_list.end();
        std::for_each(j->z_column.begin(), j->z_column.end(), make_remover(&SimplexNode::z_row, j));
        rLog(rlZigzagRemove,            "j->b_row: [%s]", j->b_row.tostring(out).c_str());
        AssertMsg(j->b_row.empty(),     "b_row of j must be empty before erasing in remove()");
        AssertMsg(s->z_row.empty(),     "z_row of s must be empty before erasing in remove()");
        AssertMsg(s->c_row.empty(),     "c_row of s must be empty before erasing in remove()");
        z_list.erase(j);
        s_list.erase(s);

        //show_all();

        AssertMsg(check_consistency(),  "Must be consistent when done in remove()");
        
        return Death(birth);
    }
}
        
template<class BID>
void
ZigzagPersistence<BID>::
show_all()
{
    std::cout << "s_list:" << std::endl;
    for (SimplexIndex cur = s_list.begin(); cur != s_list.end(); ++cur)
    {
        std::cout << "  " << cur->order << ":" << std::endl;

        std::cout << "    z_row: ";
        for (typename ZRow::const_iterator zcur = cur->z_row.begin(); zcur != cur->z_row.end(); ++zcur)
            std::cout << (*zcur)->order << " ";
        std::cout << std::endl;

        std::cout << "    c_row: ";
        for (typename CRow::const_iterator ccur = cur->c_row.begin(); ccur != cur->c_row.end(); ++ccur)
            std::cout << (*ccur)->order << " ";
        std::cout << std::endl;
        
        std::cout << "    low: ";
        if (cur->low != z_list.end())
            std::cout << cur->low->order;
        else
            std::cout << "none";
        std::cout << std::endl;
    }
    
    std::cout << "z_list:" << std::endl;
    for (ZIndex cur = z_list.begin(); cur != z_list.end(); ++cur)
    {
        std::cout << "  " << cur->order << ":" << std::endl;
        
        std::cout << "    birth: " << cur->birth << std::endl;

        std::cout << "    z_column: ";
        for (typename ZColumn::const_iterator zcur = cur->z_column.begin(); zcur != cur->z_column.end(); ++zcur)
            std::cout << (*zcur)->order << " ";
        std::cout << std::endl;

        std::cout << "    b_row: ";
        for (typename BRow::const_iterator bcur = cur->b_row.begin(); bcur != cur->b_row.end(); ++bcur)
            std::cout << (*bcur)->order << " ";
        std::cout << std::endl;

        std::cout << "    low: ";
        if (cur->low != b_list.end()) 
            std::cout << cur->low->order;
        else
            std::cout << "none";
        std::cout << std::endl;
    }

    std::cout << "b_list:" << std::endl;
    for (BIndex cur = b_list.begin(); cur != b_list.end(); ++cur)
    {
        std::cout << "  " << cur->order << ":" << std::endl;

        std::cout << "    b_column: ";
        for (typename BColumn::const_iterator bcur = cur->b_column.begin(); bcur != cur->b_column.end(); ++bcur)
            std::cout << (*bcur)->order << " ";
        std::cout << std::endl;

        std::cout << "    c_column: ";
        for (typename CColumn::const_iterator ccur = cur->c_column.begin(); ccur != cur->c_column.end(); ++ccur)
            std::cout << (*ccur)->order << " ";
        std::cout << std::endl;
    }
}

template<class BID>
bool
ZigzagPersistence<BID>::
check_consistency(SimplexIndex s_skip, ZIndex z_skip, BIndex b_skip)
{
#if !NDEBUG
    for (SimplexIndex cur = s_list.begin(); cur != s_list.end(); ++cur)
    {
        if (cur == s_skip) continue;
        //rLog(rlZigzagCheckConsistency,      "SimplexIndex cur: %d", cur->order);
        for (typename ZRow::const_iterator zcur = cur->z_row.begin(); zcur != cur->z_row.end(); ++zcur)
            if (std::find((*zcur)->z_column.begin(), (*zcur)->z_column.end(), cur) == (*zcur)->z_column.end())
            {
                rError("In check_consistency(): SimplexNode %d not found in z_column of %d", cur->order, (*zcur)->order);
                return false;
            }
        for (typename CRow::const_iterator ccur = cur->c_row.begin(); ccur != cur->c_row.end(); ++ccur)
            if (std::find((*ccur)->c_column.begin(), (*ccur)->c_column.end(), cur) == (*ccur)->c_column.end())
            {
                rError("In check_consistency(): SimplexNode %d not found in c_column of %d", cur->order, (*ccur)->order);
                return false;
            }
        if (cur->low != z_list.end())
            AssertMsg(!(cur->low->z_column.empty()),        "z_column must not be empty");
        if (cur->low != z_list.end() && cur->low->z_column.back() != cur) 
        {
            rError("low of SimplexNode %d is incorrect", cur->order);
            return false;
        }
    }

    for (ZIndex cur = z_list.begin(); cur != z_list.end(); ++cur)
    {
        if (cur == z_skip) continue;

        //rLog(rlZigzagCheckConsistency,      "ZIndex cur: %d", cur->order);
        for (typename ZColumn::const_iterator scur = cur->z_column.begin(); scur != cur->z_column.end(); ++scur)
            if (std::find((*scur)->z_row.begin(), (*scur)->z_row.end(), cur) == (*scur)->z_row.end())
            {
                rError("In check_consistency(): ZNode %d not found in z_row of %d", cur->order, (*scur)->order);
                return false;
            }
        for (typename BRow::const_iterator bcur = cur->b_row.begin(); bcur != cur->b_row.end(); ++bcur)
            if (std::find((*bcur)->b_column.begin(), (*bcur)->b_column.end(), cur) == (*bcur)->b_column.end())
            {
                rError("In check_consistency(): ZNode %d not found in b_column of %d", cur->order, (*bcur)->order);
                return false;
            }
        if (cur->low != b_list.end() && cur->low->b_column.back() != cur) 
        {
            rError("low of ZNode %d is incorrect", cur->order);
            return false;
        }
        if (cur->z_column.back()->low != cur)
        {
            rError("The low of the back of the z_column must be set correctly");
            rError("  %d [%s], its back %d with low=%d", cur->order,
                                                         cur->z_column.tostring(out).c_str(),
                                                         cur->z_column.back()->order,
                                                         (cur->z_column.back()->low == z_list.end()) ? 0 : cur->z_column.back()->low->order);
            return false;
        }
    }

    for (BIndex cur = b_list.begin(); cur != b_list.end(); ++cur)
    {
        if (cur == b_skip) continue;

        //rLog(rlZigzagCheckConsistency,      "BIndex cur: %d", cur->order);
        for (typename BColumn::const_iterator zcur = cur->b_column.begin(); zcur != cur->b_column.end(); ++zcur)
            if (std::find((*zcur)->b_row.begin(), (*zcur)->b_row.end(), cur) == (*zcur)->b_row.end())
            {
                rError("In check_consistency(): BNode %d not found in b_row of %d", cur->order, (*zcur)->order);
                return false;
            }
        for (typename CColumn::const_iterator scur = cur->c_column.begin(); scur != cur->c_column.end(); ++scur)
            if (std::find((*scur)->c_row.begin(), (*scur)->c_row.end(), cur) == (*scur)->c_row.end())
            {
                rError("In check_consistency(): BNode %d not found in c_row of %d", cur->order, (*scur)->order);
                return false;
            }
        if (!(cur->b_column.empty() || cur->b_column.back()->low == cur))
        {
            rError("The low of the back of the b_column must be set correctly");
            return false;
        }

        // ZB == DC
        ZColumn zb, dc;
        std::for_each(cur->b_column.begin(), cur->b_column.end(), make_adder(&ZNode::z_column, zb));
        std::for_each(cur->c_column.begin(), cur->c_column.end(), make_adder(&SimplexNode::boundary, dc));
        zb.add(dc, cmp);
        if (!zb.empty())
        {
            rError("   b_column: [%s]",    cur->b_column.tostring(out).c_str());
            rError("   c_column: [%s]",    cur->c_column.tostring(out).c_str());
            rError("   zb - dc:  [%s]",    zb.tostring(out).c_str());
            rError("ZB = DC");
            return false;
        }
    }
#endif

    return true;
}

/* Private */

// Class: Appender
//   
// Functor that appends given element to the given member of whatever parameter it is invoked with
template<class BID>
template<class Member, class Element>
struct ZigzagPersistence<BID>::Appender
{
                Appender(Member mm, Element ee): 
                    m(mm), e(ee)                        {}

    template<class T>
    void        operator()(T& a)                        { ((*a).*m).append(e, cmp); }
                
    Member          m;
    Element         e;
    OrderComparison cmp;
};

// Class: Remover
//   
// Functor that removes given element from the given member of whatever parameter it is invoked with
template<class BID>
template<class Member, class Element>
struct ZigzagPersistence<BID>::Remover
{
                Remover(Member mm, Element ee): 
                    m(mm), e(ee)                        {}

    template<class T>
    void        operator()(T& a)                        { ((*a).*m).remove(e); }
                
    Member  m;
    Element e;
};

// Class: Adder
//   
// Functor that adds the given member of whatever it is invoked with to the given chain
template<class BID>
template<class Member, class Chain>
struct ZigzagPersistence<BID>::Adder
{
                Adder(Member mm, Chain& cc):
                    m(mm), c(cc)                        {}

    template<class T>
    void        operator()(T& a)                        { c.add((*a).*m, cmp); }

    Member          m;
    Chain&          c;
    OrderComparison cmp;
};

        
// Function: add_chains()
//   
// Special case of add_chains where all Indexes are the same, and 
// therefore PrimaryMemberFrom and PrimaryMemberTo are the same
template<class BID>
template<class Index, class IndexFrom, class PrimaryMember, class SecondaryMember>
void
ZigzagPersistence<BID>::
add_chains(Index bg, Index end, IndexFrom j, PrimaryMember pm, SecondaryMember sm)
{
    add_chains(bg, end, j, pm, sm, pm);
}

// Function: add_chains()
//   
// Adds PrimaryMember pm of j to pm of every element in the range [bg,end)
// Fixes SecondaryMembers by adding and removing the corresponding elements.
// For example, if we add a column to a number of other columns, then PrimaryMember is that
// column member, and SecondaryMember is the corresponding row member.
template<class BID>
template<class IndexTo, class IndexFrom, class PrimaryMemberTo, class SecondaryMemberTo, class PrimaryMemberFrom>
void
ZigzagPersistence<BID>::
add_chains(IndexTo bg, IndexTo end, IndexFrom j, PrimaryMemberTo pmt, SecondaryMemberTo smt, PrimaryMemberFrom pmf)
{
    for (IndexTo cur = bg; cur != end; ++cur)
        add_chain(*cur, j, pmt, smt, pmf);
}

// Function: change_basis()
//
// Changes basis by adding PrimaryMember pm of j to pm of every element in range [bg, end).
// In parallel it performs the reverse (complementary) update on the dual members, i.e.
// column and row operations are performed in sync, so that the product of the two matrices doesn't change
template<class BID>
template<class IndexTo, class IndexFrom, class PrimaryMember, class SecondaryMember, class DualPrimaryMember, class DualSecondaryMember>
void
ZigzagPersistence<BID>::
change_basis(IndexTo bg, IndexTo end, IndexFrom j, PrimaryMember pm, SecondaryMember sm, DualPrimaryMember dpm, DualSecondaryMember dsm)
{
    for (IndexTo cur = bg; cur != end; ++cur)
    {
        add_chain(*cur, j,  pm,  sm,  pm);
        add_chain(j, *cur, dpm, dsm, dpm);
    }
}

template<class BID>
template<class Index, class PrimaryMember, class SecondaryMember>
void
ZigzagPersistence<BID>::
add_chain(Index to, Index from, PrimaryMember pm, SecondaryMember sm)
{
    add_chain(to, from, pm, sm, pm);
}

// Function: add_chain()
//
// Adds PrimaryMemberFrom pmf of `from` to PrimaryMemberTo pmt of `to`. 
// Fixes SecondaryMemberTos. See add_chains().
template<class BID>
template<class IndexTo, class IndexFrom, class PrimaryMemberTo, class SecondaryMemberTo, class PrimaryMemberFrom>
void
ZigzagPersistence<BID>::
add_chain(IndexTo to, IndexFrom from, PrimaryMemberTo pmt, SecondaryMemberTo smt, PrimaryMemberFrom pmf)
{
    rLog(rlZigzagAddChain,  "Adding %d [%s] to %d [%s]", 
                            (*from).order, 
                            ((*from).*pmf).tostring(out).c_str(),
                            (*to).order,
                            ((*to).*pmt).tostring(out).c_str());

    // Fix secondaries
    std::for_each(make_intersection_iterator(((*from).*pmf).begin(),  ((*from).*pmf).end(),
                                               ((*to).*pmt).begin(),    ((*to).*pmt).end(),
                                             cmp),
                  make_intersection_iterator(((*from).*pmf).end(),    ((*from).*pmf).end(),
                                               ((*to).*pmt).end(),      ((*to).*pmt).end(),
                                             cmp),
                  make_remover(smt, to));
    std::for_each(make_difference_iterator(((*from).*pmf).begin(),    ((*from).*pmf).end(),
                                             ((*to).*pmt).begin(),      ((*to).*pmt).end(),
                                           cmp),
                  make_difference_iterator(((*from).*pmf).end(),      ((*from).*pmf).end(),
                                             ((*to).*pmt).end(),        ((*to).*pmt).end(),
                                           cmp),
                  make_appender(smt, to));
 
    // Add primaries
    ((*to).*pmt).add((*from).*pmf, cmp);
    rLog(rlZigzagAddChain,  "Got %s", ((*to).*pmt).tostring(out).c_str());
}
