/*-----------------------------------------------------------------------*/
/* dotens.c: */
typedef struct dotens_closure_s {
     void (*apply)(struct dotens_closure_s *k, int indx, int ondx);
} dotens_closure;

void X(dotens)(tensor sz, dotens_closure *k);

/*-----------------------------------------------------------------------*/
/* dotens2.c: */
typedef struct dotens2_closure_s {
     void (*apply)(struct dotens2_closure_s *k, 
		   int indx0, int ondx0, int indx1, int ondx1);
} dotens2_closure;

void X(dotens2)(tensor sz0, tensor sz1, dotens2_closure *k);


