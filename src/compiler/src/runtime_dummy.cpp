#include <cstdint>
#include <cstdlib>
#include <cstring>

extern "C" {
void* raccoon_alloc(uint64_t size) {
    void* ptr = malloc(size);
    memset(ptr, 0, size);
    return ptr;
}
}