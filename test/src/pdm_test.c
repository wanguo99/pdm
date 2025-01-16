#include <dlfcn.h>
#include <stdio.h>
#include "libpdm.h"

int main() {
    libpdm_switch_test();
    libpdm_dimmer_test();
    return 0;
}
