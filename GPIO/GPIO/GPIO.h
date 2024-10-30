#ifndef __GPIO_H__
#define __GPIO_H__

// Standard C++
#include <functional>

// FreeRTOS
#include <freertos/FreeRTOS.h>

// ESP-IDF
#include <driver/gpio.h>

enum class GPIODirection {
    INPUT = 1,
    OUTPUT
};

enum class GPIOInterruptType {
    DISABLED,
    NEGATIVE_EDGE,
    POSITIVE_EDGE,
    ANY_EDGE,
    LOW_LEVEL,
    HIGH_LEVEL
};

class GPIO final {
    public:
        explicit GPIO(uint8_t pin, GPIODirection direction = GPIODirection::INPUT);
        ~GPIO();
        bool read() const;
        void high() const;
        void low() const;
        void interruptEnable() const;
        void interruptDisable() const;
        void setInterruptType(GPIOInterruptType interruptType) const;
        void addISRHandler(const std::function<void()>& handler);

    private:
        gpio_num_t _pin;
        std::function<void()> _isrHandler;
        static bool _isrServiceInstalled;
        static void IRAM_ATTR interruptHandler(void* arguments);
};

#endif
