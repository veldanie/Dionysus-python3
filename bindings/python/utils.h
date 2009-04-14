#ifndef __PYTHON_UTILS_H__
#define __PYTHON_UTILS_H__

struct PythonCmp
{
    template<class T>
    bool            operator()(T x1, T x2) const        { return cmp_(x1, x2) < 0; }

                    PythonCmp(object cmp): cmp_(cmp)    {}

    object cmp_;
};

#endif
