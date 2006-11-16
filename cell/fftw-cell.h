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

struct spu_radices {
     signed char r[MAX_PLAN_LEN];
};


struct dft_context {
     struct spu_radices r;
     /* strides expressed in bytes  */
     int n, is_bytes, os_bytes, v0, v1, ivs_bytes, ovs_bytes;
     int sign;
     int Wsz_bytes;
     /* pointers, converted to ulonglong */
     unsigned long long xi, xo;
     unsigned long long W;
};

/* operations that the SPE's can execute */

enum spu_op {
     SPE_DFT,
     SPE_EXIT
};

struct spu_context {
     union spu_context_u {
	  struct dft_context dft;
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
void X(dft_conf_cell)(planner *p);

void *X(cell_aligned_malloc)(size_t n);
void X(cell_activate_spes)(void);
void X(cell_deactivate_spes)(void);

int X(cell_nspe)(void);
struct spu_context *X(cell_get_ctx)(int spe);
void X(cell_spe_awake_all)(void);
void X(cell_spe_wait_all)(void);
