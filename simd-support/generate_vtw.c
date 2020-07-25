#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

unsigned int rp2(unsigned int size) {
  size = size | (size >> 1);
  size = size | (size >> 2);
  size = size | (size >> 4);
  size = size | (size >> 8);
  size = size | (size >> 16);
//  size = size | (size >> 32);
  size = size - (size >> 1);
  return size;
}

int main(int argc, char **argv) {
	if (argc < 3) {
	printf("usage: %s <array_name> <width>\n", argv[0]);
	exit(-1);
	}
	if (strncmp(argv[1], "VTW1", 4) == 0) {
		unsigned int osize = atoi(argv[2]);
		unsigned int size = rp2(osize);
		if (osize != size)
			exit(-4);
		if (size < 1)
			exit(-2);
		if (size > 256)
			exit(-3);
		printf("#define VTW1(v,x) ");
		for (unsigned int i = 0 ; i < size ; i++) {
			printf("{TW_CEXP, v+%d, x}%s%s", i, (i == size-1?"":","), ((i%4==3 && i!=size-1)?" \\\n\t":" "));
		}
		printf("\n");
	}
        if (strncmp(argv[1], "VTW2", 4) == 0) {
                unsigned int osize = atoi(argv[2]);
                unsigned int size = rp2(osize);
                if (osize != size)
                        exit(-4);
                if (size < 1)
                        exit(-2);
                if (size > 256)
                        exit(-3);
                printf("#define VTW2(v,x) ");
                for (unsigned int i = 0 ; i < size ; i++) {
                        printf("{TW_COS, v+%d, x}%s%s", i/2, ",", ((i%4==3)?" \\\n\t":" "));
                }
		for (unsigned int i = 0 ; i < size ; i++) {
                        printf("{TW_SIN, v+%d, %sx}%s%s", i/2, (i%2==0?"-":""), (i == size-1?"":","), ((i%4==3 && i!=size-1)?" \\\n\t":" "));
                }

                printf("\n");
        }
        if (strncmp(argv[1], "VTWS", 4) == 0) {
                unsigned int osize = atoi(argv[2]);
                unsigned int size = rp2(osize);
                if (osize != size)
                        exit(-4);
                if (size < 1)
                        exit(-2);
                if (size > 256)
                        exit(-3);
                printf("#define VTWS(v,x) ");
                for (unsigned int i = 0 ; i < size ; i++) {
                        printf("{TW_COS, v+%d, x}%s%s", i, ",", ((i%4==3)?" \\\n\t":" "));
                }
                for (unsigned int i = 0 ; i < size ; i++) {
                        printf("{TW_SIN, v+%d, x}%s%s", i, (i == size-1?"":","), ((i%4==3 && i!=size-1)?" \\\n\t":" "));
                }

                printf("\n");
        }



	return 0;
}
