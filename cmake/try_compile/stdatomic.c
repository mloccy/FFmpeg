#include <stdatomic.h>

int main()
{
  atomic_int foo, bar = ATOMIC_VAR_INIT(-1);
  atomic_store(&foo, 0);
  foo += bar;
}
