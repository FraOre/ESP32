#include "Button/Button.h"

Button::Button(const uint8_t pin)
    : _gpio(pin)
{
    _released = true;
    _clickCount = 0;
    _tickCounter = 0;
    _clickTimer = xTimerCreate("Click Timer", pdMS_TO_TICKS(25), pdTRUE, this, clickTimerHandler);
    _doubleClickTimer = xTimerCreate("Double Click Timer", pdMS_TO_TICKS(250), pdFALSE, this, doubleClickTimerHandler);
    _gpio.setInterruptType(GPIOInterruptType::LOW_LEVEL);
    _gpio.addISRHandler([this]() -> void {
        if (_released) {
            _gpio.setInterruptType(GPIOInterruptType::DISABLED);
            _tickCounter = xTaskGetTickCount();
            _released = false;
            xTimerStart(_clickTimer, 0);
        }
    });
}

Button::~Button() {
    xTimerDelete(_clickTimer, portMAX_DELAY);
    xTimerDelete(_doubleClickTimer, portMAX_DELAY);
}

void Button::onClick(const std::function<void()>& handler) {
    _onClickHandlers.push_back(handler);
}

void Button::onDoubleClick(const std::function<void()>& handler) {
    _onDoubleClickHandlers.push_back(handler);
}

void Button::onPress(const uint32_t pressTime, const std::function<void()>& handler) {
    _onPressHandlers.push_back({ pressTime, handler });
}

void Button::onRelease(const std::function<void()>& handler) {
    _onReleaseHandlers.push_back(handler);
}

void Button::clickTimerHandler(TimerHandle_t timer) {
    auto* button = static_cast<Button*>(pvTimerGetTimerID(timer));
    if (!button->_gpio.read()) {
        const TickType_t elapsedTime = pdTICKS_TO_MS(xTaskGetTickCount() - button->_tickCounter);
        for (auto&[pressTime, handler, alreadyHandled] : button->_onPressHandlers) {
            if (elapsedTime >= pressTime && !alreadyHandled) {
                handler();
                alreadyHandled = true;
            }
        }
    }
    else {
        button->_released = true;
        const TickType_t elapsedTime = pdTICKS_TO_MS(xTaskGetTickCount() - button->_tickCounter);
        if (elapsedTime < SHORT_PRESS_THRESHOLD) {
            button->_clickCount++;
            if (button->_clickCount == 1) {
                xTimerStart(button->_doubleClickTimer, 0);
            }
            else if (button->_clickCount == 2) {
                xTimerStop(button->_doubleClickTimer, 0);
                for (const auto& handler : button->_onDoubleClickHandlers) {
                    handler();
                }
                button->_clickCount = 0;
            }
        }
        else {
            for (const auto& handler : button->_onReleaseHandlers) {
                handler();
            }
        }
        xTimerStop(button->_clickTimer, 0);
        button->_gpio.setInterruptType(GPIOInterruptType::LOW_LEVEL);
        for (auto&[pressTime, handler, alreadyHandled] : button->_onPressHandlers) {
            alreadyHandled = false;
        }
    }
}

void Button::doubleClickTimerHandler(TimerHandle_t timer) {
    auto* button = static_cast<Button*>(pvTimerGetTimerID(timer));
    if (button->_clickCount == 1) {
        for (const auto& handler : button->_onClickHandlers) {
            handler();
        }
    }
    button->_clickCount = 0;
}
