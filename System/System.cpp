#include "System.h"

void System::dumpHeapInfo() {
	multi_heap_info_t heapInfo;
	printf("         %10s %10s %10s %10s %13s %11s %12s\n", "Free", "Allocated", "Largest", "Minimum", "Alloc Blocks", "Free Blocks", "Total Blocks");
	heap_caps_get_info(&heapInfo, MALLOC_CAP_EXEC);
	printf("EXEC     %10d %10d %10d %10d %13d %11d %12d\n", heapInfo.total_free_bytes, heapInfo.total_allocated_bytes, heapInfo.largest_free_block, heapInfo.minimum_free_bytes, heapInfo.allocated_blocks, heapInfo.free_blocks, heapInfo.total_blocks);
	heap_caps_get_info(&heapInfo, MALLOC_CAP_32BIT);
	printf("32BIT    %10d %10d %10d %10d %13d %11d %12d\n", heapInfo.total_free_bytes, heapInfo.total_allocated_bytes, heapInfo.largest_free_block, heapInfo.minimum_free_bytes, heapInfo.allocated_blocks, heapInfo.free_blocks, heapInfo.total_blocks);
	heap_caps_get_info(&heapInfo, MALLOC_CAP_8BIT);
	printf("8BIT     %10d %10d %10d %10d %13d %11d %12d\n", heapInfo.total_free_bytes, heapInfo.total_allocated_bytes, heapInfo.largest_free_block, heapInfo.minimum_free_bytes, heapInfo.allocated_blocks, heapInfo.free_blocks, heapInfo.total_blocks);
	heap_caps_get_info(&heapInfo, MALLOC_CAP_DMA);
	printf("DMA      %10d %10d %10d %10d %13d %11d %12d\n", heapInfo.total_free_bytes, heapInfo.total_allocated_bytes, heapInfo.largest_free_block, heapInfo.minimum_free_bytes, heapInfo.allocated_blocks, heapInfo.free_blocks, heapInfo.total_blocks);
	heap_caps_get_info(&heapInfo, MALLOC_CAP_SPIRAM);
	printf("SPISRAM  %10d %10d %10d %10d %13d %11d %12d\n", heapInfo.total_free_bytes, heapInfo.total_allocated_bytes, heapInfo.largest_free_block, heapInfo.minimum_free_bytes, heapInfo.allocated_blocks, heapInfo.free_blocks, heapInfo.total_blocks);
	heap_caps_get_info(&heapInfo, MALLOC_CAP_INTERNAL);
	printf("INTERNAL %10d %10d %10d %10d %13d %11d %12d\n", heapInfo.total_free_bytes, heapInfo.total_allocated_bytes, heapInfo.largest_free_block, heapInfo.minimum_free_bytes, heapInfo.allocated_blocks, heapInfo.free_blocks, heapInfo.total_blocks);
	heap_caps_get_info(&heapInfo, MALLOC_CAP_DEFAULT);
	printf("DEFAULT  %10d %10d %10d %10d %13d %11d %12d\n", heapInfo.total_free_bytes, heapInfo.total_allocated_bytes, heapInfo.largest_free_block, heapInfo.minimum_free_bytes, heapInfo.allocated_blocks, heapInfo.free_blocks, heapInfo.total_blocks);
}

void System::dumpFlashPartitions() {
    printf("%-20s | %-10s | %-10s | %-10s | %-10s\n", "Name", "Type", "Subtype", "Offset", "Length");
    esp_partition_iterator_t iter = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, nullptr);
    while (iter) {
        const esp_partition_t *partition = esp_partition_get(iter);
        printf("%-20s | %-10s | %-10d | 0x%-8lx | 0x%-8lx\n", partition->label, "data", partition->subtype, partition->address, partition->size);
        iter = esp_partition_next(iter);
    }
    esp_partition_iterator_release(iter);
    iter = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, nullptr);
    while (iter) {
        const esp_partition_t* partition = esp_partition_get(iter);
        printf("%-20s | %-10s | %-10d | 0x%-8lx | 0x%-8lx\n", partition->label, "app", partition->subtype, partition->address, partition->size);
        iter = esp_partition_next(iter);
    }
    esp_partition_iterator_release(iter);
}

void System::dumpChipInfo() {
    esp_chip_info_t chipInfo;
    esp_chip_info(&chipInfo);
    printf("Chip revision: %d\n", chipInfo.revision);
    printf("Number of CPU cores: %d\n", chipInfo.cores);
    printf("Features: 0x%ld\n", chipInfo.features);
    uint32_t size_flash_chip;
    esp_flash_get_size(nullptr, &size_flash_chip);
    printf("Flash size: %lu bytes\n", size_flash_chip);
}

std::size_t System::getFreeHeapSize() {
	return heap_caps_get_free_size(MALLOC_CAP_8BIT);
}

std::string System::getIDFVersion() {
	return std::string{ esp_get_idf_version() };
}

std::size_t System::getMinimumFreeHeapSize() {
	return heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
}

void System::restart() {
	esp_restart();
}
