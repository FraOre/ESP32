#ifndef __MEMORY_H__
#define __MEMORY_H__

// Standard C++
#include <cstdint>

class Memory final {
    public:
        static bool isOnHeap(void* pointer);
        static bool isOnStack(void* pointer);
        static bool isOnStatic(void* pointer);

    private:
        static void computeMemoryDistances(void* pointer, uintptr_t& stackDistance, uintptr_t& heapDistance, uintptr_t& staticDistance);
};

#endif
