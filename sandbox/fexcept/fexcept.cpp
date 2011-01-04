#include <stdio.h>
#include <fenv.h>

void print_flags(int flags)
{
  if (  flags & FE_DIVBYZERO  ) printf(" division by zero exception\n");
  if (!(flags & FE_ALL_EXCEPT)) printf(" no exceptions\n");
}

int main(void)
{
  fexcept_t flags;

  fegetexceptflag(&flags, FE_ALL_EXCEPT);

  print_flags(flags);
  
  feraiseexcept(FE_DIVBYZERO); /* raise divide-by-zero exception */
  int excepts = fetestexcept(FE_DIVBYZERO | FE_INEXACT);

  print_flags(excepts);
}