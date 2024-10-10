#include "Memory.h"

void Memory::computeMemoryDistances(void* const pointer, uintptr_t& stackDistance, uintptr_t& heapDistance, uintptr_t& staticDistance) {
    static int staticInteger = 0;
    int integer = 0;
    auto pointerToInteger = new int(0);
    stackDistance = pointer > &integer ?
        reinterpret_cast<uintptr_t>(pointer) - reinterpret_cast<uintptr_t>(&integer) :
        reinterpret_cast<uintptr_t>(&integer) - reinterpret_cast<uintptr_t>(pointer);
    heapDistance = pointer > pointerToInteger ?
        reinterpret_cast<uintptr_t>(pointer) - reinterpret_cast<uintptr_t>(pointerToInteger) :
        reinterpret_cast<uintptr_t>(pointerToInteger) - reinterpret_cast<uintptr_t>(pointer);
    staticDistance = pointer > &staticInteger ?
        reinterpret_cast<uintptr_t>(pointer) - reinterpret_cast<uintptr_t>(&staticInteger) :
        reinterpret_cast<uintptr_t>(&staticInteger) - reinterpret_cast<uintptr_t>(pointer);
    delete pointerToInteger;
}

bool Memory::isOnHeap(void* const pointer) {
    uintptr_t stackDistance, heapDistance, staticDist;
    computeMemoryDistances(pointer, stackDistance, heapDistance, staticDist);
    return heapDistance < stackDistance && heapDistance < staticDist;
}

bool Memory::isOnStack(void* const pointer) {
    uintptr_t stackDistance, heapDistance, staticDist;
    computeMemoryDistances(pointer, stackDistance, heapDistance, staticDist);
    return stackDistance < heapDistance && stackDistance < staticDist;
}

bool Memory::isOnStatic(void* const pointer) {
    uintptr_t stackDistance, heapDistance, staticDist;
    computeMemoryDistances(pointer, stackDistance, heapDistance, staticDist);
    return staticDist < stackDistance && staticDist < heapDistance;
}
