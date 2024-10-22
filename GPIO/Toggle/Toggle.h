#ifndef __TOGGLE_H__
#define __TOGGLE_H__

// Standard C++
#include <vector>
#include <functional>

// ESP32
#include "GPIO/GPIO.h"

class Toggle final {
    public:
        explicit Toggle(uint8_t pin);
        ~Toggle();
        void onLow(const std::function<void()>& handler);
        void onHigh(const std::function<void()>& handler);
        void onChange(const std::function<void(bool state)>& handler);
        bool getState() const;

    private:
        bool _state;
        GPIO _gpio;
        TimerHandle_t _timer;
        std::vector<std::function<void()>> _onLowHandlers;
        std::vector<std::function<void()>> _onHighHandlers;
        std::vector<std::function<void(bool state)>> _onChangeHandlers;
        static void timerHandler(TimerHandle_t timer);
};

#endif
