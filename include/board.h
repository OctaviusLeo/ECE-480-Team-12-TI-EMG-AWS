#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "driverlib/pin_map.h"

#define SYSCLK_HZ 80000000u

// SPI (SSI0) pins
#define OLED_SSI_BASE        SSI0_BASE
#define OLED_PORTA_BASE      GPIO_PORTA_BASE
#define OLED_PERIPH_SSI      SYSCTL_PERIPH_SSI0
#define OLED_PERIPH_PORTA    SYSCTL_PERIPH_GPIOA
#define OLED_PIN_CLK         GPIO_PIN_2   
#define OLED_PIN_TX          GPIO_PIN_5  

// Control pins
#define OLED_PERIPH_PORTB    SYSCTL_PERIPH_GPIOB
#define OLED_PORTB_BASE      GPIO_PORTB_BASE
#define OLED_PIN_DC          GPIO_PIN_2   
#define OLED_PIN_RST         GPIO_PIN_3  

// CS on PA3 as GPIO
#define OLED_PIN_CS          GPIO_PIN_3   

static inline void delay_cycles(volatile uint32_t n){ while(n--) __asm(" nop"); }

#endif
