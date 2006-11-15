#include "fftw-cell.h"

static const solvtab s =
{
     SOLVTAB(X(dft_direct_cell_register)),
     SOLVTAB_END
};

void X(dft_conf_cell)(planner *p)
{
     X(solvtab_exec)(s, p);
}
