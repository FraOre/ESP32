#include "Toggle/Toggle.h"

Toggle::Toggle(const uint8_t pin)
    : _gpio(pin)
{
    _timer = xTimerCreate("Toggle Timer", pdMS_TO_TICKS(25), pdFALSE, this, timerHandler);
    _gpio.setInterruptType(GPIOInterruptType::ANY_EDGE);
    _gpio.addISRHandler([this]() -> void {
        xTimerStart(_timer, 0);
    });
    _state = _gpio.read();
}

Toggle::~Toggle()
{
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

void Toggle::timerHandler(TimerHandle_t timer)
{
    auto* toggle = static_cast<Toggle*>(pvTimerGetTimerID(timer));
    const bool currentState = toggle->_gpio.read();
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
