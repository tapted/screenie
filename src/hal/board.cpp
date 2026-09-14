#include "hal/board.hpp"

namespace halpp::board {

static_assert(GPIO_NUM_0 == config::System::PIN_BOOT);
// static_assert(GPIO_NUM_1 == available);
// static_assert(GPIO_NUM_2 == available);
// static_assert(GPIO_NUM_3 == available);
static_assert(GPIO_NUM_4 == config::SdCard::PIN_CHIP_SELECT);
static_assert(GPIO_NUM_5 == config::SdCard::PIN_MISO);
static_assert(GPIO_NUM_6 == config::SpiBus::PIN_MOSI);
static_assert(GPIO_NUM_7 == config::SpiBus::PIN_SERIAL_CLOCK);  // Also SdCard::PIN_SERIAL_CLOCK
static_assert(GPIO_NUM_8 == config::IndicatorLed::PIN_RGB);
static_assert(GPIO_NUM_9 == config::System::PIN_KEY);
// static_assert(GPIO_NUM_10 == config::I2CConfig::PIN_SCL);
// static_assert(GPIO_NUM_11 == config::I2CConfig::PIN_SDA);
// static_assert(GPIO_NUM_12 == available);
// static_assert(GPIO_NUM_13 == available);
static_assert(GPIO_NUM_14 == config::SpiBus::PIN_CHIP_SELECT);
static_assert(GPIO_NUM_15 == config::Display::PIN_DATA_COMMAND);
static_assert(GPIO_NUM_16 == config::Usb::PIN_UART_TX);
static_assert(GPIO_NUM_17 == config::Usb::PIN_UART_RX);
static_assert(GPIO_NUM_18 == config::I2CConfig::PIN_SDA);
static_assert(GPIO_NUM_19 == config::I2CConfig::PIN_SCL);
static_assert(GPIO_NUM_20 == config::Buzzer::PIN_PWM);
static_assert(GPIO_NUM_21 == config::Display::PIN_RESET);
static_assert(GPIO_NUM_22 == config::Display::PIN_BACKLIGHT_PWM);
// static_assert(GPIO_NUM_23 == available);
// Unavailable: 24-30

}  // namespace halpp::board