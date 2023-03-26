#include <stdatomic.h>

int main()
{
#ifdef _MSC_VER
    _Atomic int foo, bar;
    atomic_init(&bar, -1);
#else
  atomic_int foo, bar = ATOMIC_VAR_INIT(-1);
  #endif
  
  atomic_store(&foo, 0);
  foo += bar;
}
