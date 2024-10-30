#include "GPIO.h"

bool GPIO::_isrServiceInstalled = false;

GPIO::GPIO(uint8_t pin, const GPIODirection direction)
{
    _pin = static_cast<gpio_num_t>(pin);
    gpio_config_t gpioConfig = {};
    gpioConfig.pin_bit_mask = 1ULL << _pin;
    gpioConfig.intr_type = GPIO_INTR_DISABLE;
    if (direction == GPIODirection::INPUT) {
        gpioConfig.mode = GPIO_MODE_INPUT;
        gpioConfig.pull_up_en = GPIO_PULLUP_ENABLE;
    }
    else {
        gpioConfig.mode = GPIO_MODE_OUTPUT;
    }
    gpio_config(&gpioConfig);
}

GPIO::~GPIO()
{
    gpio_isr_handler_remove(_pin);
    gpio_reset_pin(_pin);
}

bool GPIO::read() const
{
    return gpio_get_level(_pin);
}

void GPIO::high() const
{
    gpio_set_level(_pin, 1);
}

void GPIO::low() const
{
    gpio_set_level(_pin, 0);
}

void GPIO::interruptEnable() const
{
    gpio_intr_enable(_pin);
}

void GPIO::interruptDisable() const
{
    gpio_intr_disable(_pin);
}

void GPIO::setInterruptType(const GPIOInterruptType interruptType) const
{
    gpio_int_type_t type;
    switch (interruptType) {
        case GPIOInterruptType::DISABLED:
            type = GPIO_INTR_DISABLE;
        break;
        case GPIOInterruptType::NEGATIVE_EDGE:
            type = GPIO_INTR_NEGEDGE;
        break;
        case GPIOInterruptType::POSITIVE_EDGE:
            type = GPIO_INTR_POSEDGE;
        break;
        case GPIOInterruptType::ANY_EDGE:
            type = GPIO_INTR_ANYEDGE;
        break;
        case GPIOInterruptType::LOW_LEVEL:
            type = GPIO_INTR_LOW_LEVEL;
        break;
        case GPIOInterruptType::HIGH_LEVEL:
            type = GPIO_INTR_HIGH_LEVEL;
        break;
        default:
            type = GPIO_INTR_DISABLE;
    }
    gpio_set_intr_type(_pin, type);
}

void GPIO::addISRHandler(const std::function<void()>& handler)
{
    if (!_isrServiceInstalled) {
        gpio_install_isr_service(0);
        _isrServiceInstalled = true;
    }
    _isrHandler = handler;
    gpio_isr_handler_add(_pin, interruptHandler, this);
}

void GPIO::interruptHandler(void* arguments)
{
    const auto* gpio = static_cast<GPIO*>(arguments);
    gpio->_isrHandler();
    portYIELD_FROM_ISR();
}
