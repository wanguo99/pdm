#include <dlfcn.h>
#include <stdio.h>
#include "../library/libpdm_hello.h"

int main() {
    void *handle;
    void (*print_pdm_hello)(void);
    char *error;

    handle = dlopen("../library/libpdm_hello.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        return 1;
    }

    dlerror();    /* Clear any existing error */

    *(void **)(&print_pdm_hello) = dlsym(handle, "print_pdm_hello");

    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "%s\n", error);
        return 1;
    }

    print_pdm_hello();

    dlclose(handle);
    return 0;
}
