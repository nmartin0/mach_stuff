
#include <ppc/POWERMAC/pci/types.h>

int main(int argc, char **argv) {
  u_int64_t ui64;
  printf("sizeof(ui64) = %d\n", sizeof(ui64));
  assert(sizeof(ui64) == 8);
  return 0;
}

