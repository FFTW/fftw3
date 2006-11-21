#include <ifftw.h>

#ifdef FFTW_SINGLE 
#define MAX_N 4096
#define REQUIRE_N_MULTIPLE_OF 4
#define VL 2
#else
#define MAX_N 2048
#define REQUIRE_N_MULTIPLE_OF 1
#define VL 1
#endif

#define MAX_PLAN_LEN 4
#define SPU_NUM_THREADS 8
#define CELL_DFT_MAXVRANK 2

struct spu_radices {
     signed char r[MAX_PLAN_LEN];
};

struct cell_iodim {
     int n0;  /* lower bound, n0 <= n */
     int n1;  /* upper bound, n < n1 */

     /* strides expressed in bytes  */
     int is_bytes;
     int os_bytes;
};

struct cell_iotensor {
     struct cell_iodim dims[CELL_DFT_MAXVRANK];
};

struct dft_context {
     struct spu_radices r;
     /* strides expressed in bytes  */
     int n, is_bytes, os_bytes;
     struct cell_iotensor v;
     int sign;
     int Wsz_bytes;
     /* pointers, converted to ulonglong */
     long long xi, xo;
     long long W;
};

struct transpose_context {
     long long A;
     int n;
     int s0_bytes;
     int s1_bytes;
     int nspe;
     int my_id;
};

/* operations that the SPE's can execute */

enum spu_op {
     SPE_DFT,
     SPE_TRANSPOSE,
     SPE_EXIT
};

struct spu_context {
     union spu_context_u {
	  struct dft_context dft;
	  struct transpose_context transpose;
	  /* possibly others */
     } u;

     char pad[15 -
	      (((sizeof(union spu_context_u) + sizeof(enum spu_op)) - 1) 
	       & 15)];
     enum spu_op op;
};

extern const struct spu_radices 
   X(spu_radices)[(MAX_N/REQUIRE_N_MULTIPLE_OF) + 1];

void X(dft_direct_cell_register)(planner *p);

void *X(cell_aligned_malloc)(size_t n);
void X(cell_activate_spes)(void);
void X(cell_deactivate_spes)(void);

int X(cell_nspe)(void);
struct spu_context *X(cell_get_ctx)(int spe);
void X(cell_spe_awake_all)(void);
void X(cell_spe_wait_all)(void);
