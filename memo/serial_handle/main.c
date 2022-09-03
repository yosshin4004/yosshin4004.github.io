#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"

#define NUM_HANDLE_BITS     16
#define NUM_HANDLES         (1 << NUM_HANDLE_BITS)
#define NUM_HANDLES_MINUS1  (NUM_HANDLES - 1)
struct {
    uint32_t serial;
    void *p;
} table[NUM_HANDLES] = {0};

uint32_t p2sh(void *p) {
    for (uint32_t handle = 0; handle < NUM_HANDLES; handle++) {
        if (table[handle].p == NULL) {
            uint32_t serialHandle = (++table[handle].serial << NUM_HANDLE_BITS) | handle;
            if (serialHandle == 0) {
                table[handle].serial++;
                serialHandle = 1;
            }
            table[handle].p = p;
            return serialHandle;
        }
    }
    return 0;       /* シリアルハンドルが確保できなかった */
}

void *sh2p(uint32_t serialHandle) {
    uint32_t handle = serialHandle & NUM_HANDLES_MINUS1;
    uint32_t serial = serialHandle >> NUM_HANDLE_BITS;
    if (table[handle].serial == serial) {
        return table[handle].p;
    }
    return NULL;    /* シリアルハンドルは無効だった */
}

bool dispose_sh(uint32_t serialHandle) {
    uint32_t handle = serialHandle & NUM_HANDLES_MINUS1;
    uint32_t serial = serialHandle >> NUM_HANDLE_BITS;
    if (table[handle].serial == serial) {
        table[handle].p = NULL;
        return true;
    }
    return false;
}

int main() {
    int *p = (int *)malloc(256);
    printf("p = %p\n", p);

    int sh = p2sh(p);
    printf("sh = %08X\n", sh);

    p = (int *)sh2p(sh);
    printf("p = %p\n", p);
    if (p != NULL) { free(p); dispose_sh(sh); }

    p = (int *)sh2p(sh);
    printf("p = %p\n", p);
    if (p != NULL) { *p = 0; }    /* 不正なメモリアクセスは回避される */

    return EXIT_SUCCESS;
}
