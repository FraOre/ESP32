#include "Toggle/Toggle.h"

Toggle::Toggle(uint8_t pin)
{
    _pin = static_cast<gpio_num_t>(pin);
    _timer = xTimerCreate("Toggle Timer", pdMS_TO_TICKS(50), pdFALSE, this, timerHandler);
    gpio_config_t gpioConfig = {};
    gpioConfig.pin_bit_mask = 1ULL << pin;
    gpioConfig.mode = GPIO_MODE_INPUT;
    gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpioConfig.pull_up_en = GPIO_PULLUP_ENABLE;
    gpioConfig.intr_type = GPIO_INTR_ANYEDGE;
    gpio_config(&gpioConfig);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(_pin, interruptHandler, this);
    _state = gpio_get_level(_pin);
}

Toggle::~Toggle()
{
    gpio_isr_handler_remove(_pin);
    xTimerDelete(_timer, portMAX_DELAY);
}

bool Toggle::getState() const
{
    return _state;
}

void Toggle::onLow(const std::function<void()> &handler)
{
    _onLowHandler = handler;
}

void Toggle::onHigh(const std::function<void()> &handler)
{
    _onHighHandler = handler;
}

void Toggle::onChange(const std::function<void(bool state)> &handler)
{
    _onChangeHandler = handler;
}

void IRAM_ATTR Toggle::interruptHandler(void* argument)
{
    const auto* toggle = static_cast<Toggle*>(argument);
    xTimerStartFromISR(toggle->_timer, nullptr);
    portYIELD_FROM_ISR();
}

void Toggle::timerHandler(TimerHandle_t timer)
{
    auto* toggle = static_cast<Toggle*>(pvTimerGetTimerID(timer));
    const bool currentState = gpio_get_level(toggle->_pin);
    if (currentState != toggle->_state) {
        toggle->_state = currentState;
        if (toggle->_onChangeHandler) {
            toggle->_onChangeHandler(toggle->_state);
        }
        if (toggle->_state == false && toggle->_onLowHandler) {
            toggle->_onLowHandler();
        }
        if (toggle->_state == true && toggle->_onHighHandler) {
            toggle->_onHighHandler();
        }
    }
}
