#ifndef __BUTTON_H__
#define __BUTTON_H__

// Standard C++
#include <vector>
#include <functional>

// FreeRTOS
#include <freertos/FreeRTOS.h>

// ESP32
#include "GPIO/GPIO.h"

class Button final {
    public:
        explicit Button(uint8_t pin);
        ~Button();
        void onClick(const std::function<void()>& handler);
        void onDoubleClick(const std::function<void()>& handler);
        void onPress(uint32_t pressTime, const std::function<void()>& handler);
        void onRelease(const std::function<void()>& handler);

    private:
        static constexpr TickType_t SHORT_PRESS_THRESHOLD = 1000;
        GPIO _gpio;
        bool _released;
        uint8_t _clickCount;
        TimerHandle_t _clickTimer;
        TimerHandle_t _doubleClickTimer;
        TickType_t _tickCounter;
        struct PressHandler {
            uint32_t pressTime;
            std::function<void()> handler;
            bool alreadyHandled = false;
        };
        std::vector<PressHandler> _onPressHandlers;
        std::vector<std::function<void()>> _onClickHandlers;
        std::vector<std::function<void()>> _onDoubleClickHandlers;
        std::vector<std::function<void()>> _onReleaseHandlers;
        static void clickTimerHandler(TimerHandle_t timer);
        static void doubleClickTimerHandler(TimerHandle_t timer);
};

#endif
