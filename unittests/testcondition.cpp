#include "conditions.h"
#include "sblookuptable.h"
#include "regexp.h"
#include "util.h"

#include <iostream>
#include <vector>
using namespace std;

template <class T> class calc
{
  int _foo;
  public:
    calc (int foo);
    T multiply(T x, T y);
    T add(T x, T y);
};

template <class T> calc<T>::calc (int foo) { _foo = foo; }
template <class T> T calc<T>::multiply(T x,T y)
{
  return x*y;
}
template <class T> T calc<T>::add(T x, T y)
{
  return x+y;
}

int main () {
  ConditionParser cp ("input.conf");
  cp.parse();

  unsigned int a[] = {2, 3, 4};
  vector<unsigned int> dim_sizes (a, a+3);

  int tmp = 0;
  MDArray<int> mda (3, dim_sizes);

  calc<int> c(10);

  cout << c.add (3.0, 4.0) << endl;


  return 0;
}
