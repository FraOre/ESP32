#ifndef __TOGGLE_H__
#define __TOGGLE_H__

// Standard C++
#include <functional>

// ESP32
#include "GPIO/GPIO.h"

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
        GPIO _gpio;
        TimerHandle_t _timer;
        std::function<void()> _onLowHandler;
        std::function<void()> _onHighHandler;
        std::function<void(bool)> _onChangeHandler;
        static void timerHandler(TimerHandle_t timer);
};

#endif
