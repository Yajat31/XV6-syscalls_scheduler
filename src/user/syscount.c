#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "../kernel/param.h"
//#include "../kernel/proc.h"
#include "user.h"

int
main(int argc, char *argv[])
{
   //printf("chk\n");
   if (argc < 3) {
    printf("Usage: syscount <mask> command [args]\n");
    exit(0);
  }

  int mask = atoi(argv[1]);
  if (mask < 0 || mask >= (1 << 23)) {
    printf("Invalid mask\n");
    exit(0);
  }
  //printf("mask: %d\n", mask);  

  if(syscount(mask) < 0){
    printf("syscount failed\n");
    exit(0);
  }

  exec(argv[2], &argv[2]);
  printf("exec %s failed\n", argv[2]);


  exit(0);
}