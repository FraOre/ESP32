#include "Button/Button.h"

Button::Button(uint8_t pin) {
    _pin = static_cast<gpio_num_t>(pin);
    _released = true;
    _clickCount = 0;
    _tickCounter = 0;
    _clickTimer = xTimerCreate("Click Timer", pdMS_TO_TICKS(25), pdTRUE, this, clickTimerHandler);
    _doubleClickTimer = xTimerCreate("Double Click Timer", pdMS_TO_TICKS(250), pdFALSE, this, doubleClickTimerHandler);
    gpio_config_t gpioConfig = {};
    gpioConfig.pin_bit_mask = 1ULL << pin;
    gpioConfig.mode = GPIO_MODE_INPUT;
    gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpioConfig.pull_up_en = GPIO_PULLUP_ENABLE;
    gpioConfig.intr_type = GPIO_INTR_LOW_LEVEL;
    gpio_config(&gpioConfig);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(_pin, interruptHandler, this);
}

Button::~Button() {
    xTimerDelete(_clickTimer, portMAX_DELAY);
    xTimerDelete(_doubleClickTimer, portMAX_DELAY);
    gpio_isr_handler_remove(_pin);
    gpio_reset_pin(_pin);
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

void Button::interruptHandler(void* argument) {
    auto* button = static_cast<Button*>(argument);
    if (button->_released) {
        gpio_set_intr_type(button->_pin, GPIO_INTR_DISABLE);
        button->_tickCounter = xTaskGetTickCountFromISR();
        button->_released = false;
        xTimerStartFromISR(button->_clickTimer, nullptr);
    }
    portYIELD_FROM_ISR();
}

void Button::clickTimerHandler(TimerHandle_t timer) {
    auto* button = static_cast<Button*>(pvTimerGetTimerID(timer));
    if (!gpio_get_level(button->_pin)) {
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
        gpio_set_intr_type(button->_pin, GPIO_INTR_LOW_LEVEL);
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
