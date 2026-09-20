/* $Id: pci_register.c,v 1.1 1999/06/18 05:49:49 vallon Exp $ */

#include <ppc/POWERMAC/pci/pci_register.h>
#include <ppc/POWERMAC/pci/aic7xxx_mach.h>

int pci_register() {
  int rtn = 0;
  int stat;

  if (stat = aic7xxx_init()) {
    printf("aic7xxx_init failed");
    rtn = 1;
  }

  return rtn;
}

