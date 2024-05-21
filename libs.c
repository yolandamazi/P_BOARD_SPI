#include <log.h>

#define ISNAN(x) (							\
  {									\
    union u { double d; unsigned int i[2]; } *t = (union u *)&(x);	\
    ((t->i[1] & 0x7ff00000) == 0x7ff00000) &&				\
    (t->i[0] != 0 || (t->i[1] & 0xfffff) != 0);				\
  })

int __unorddf2 (double, double);
int __unorddf2 (double a, double b)
{
  return ISNAN(a) || ISNAN(b);
}

int *__errno (void)
{
   return NULL;
}
