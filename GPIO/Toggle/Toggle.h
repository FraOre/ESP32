#ifndef __TOGGLE_H__
#define __TOGGLE_H__

// Standard C++
#include <functional>

// ESP-IDF
#include <driver/gpio.h>

// FreeRTOS
#include <freertos/FreeRTOS.h>

class Toggle final {
    public:
        explicit Toggle(uint8_t pin);
        ~Toggle();
        void onLow(const std::function<void()> &handler);
        void onHigh(const std::function<void()> &handler);
        void onChange(const std::function<void(bool state)> &handler);
        bool getState() const;

    private:
        bool _state;
        gpio_num_t _pin;
        TimerHandle_t _timer;
        std::function<void()> _onLowHandler;
        std::function<void()> _onHighHandler;
        std::function<void(bool)> _onChangeHandler;
        static void IRAM_ATTR interruptHandler(void* argument);
        static void timerHandler(TimerHandle_t timer);
};

#endif
