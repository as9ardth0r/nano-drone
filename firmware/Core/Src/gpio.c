#include "gpio.h"
#include "stm32f4xx.h"

/* GPIOA..GPIOI sont espacés de 0x400 dans la carte mémoire STM32F4
 * (RM0090 §2.3), et les bits GPIOxEN de RCC->AHB1ENR suivent le même
 * ordre (bit0=A, bit1=B, ...) — permet de généraliser sans switch/case. */
static GPIO_TypeDef *port_from_index(uint8_t index) {
    return (GPIO_TypeDef *)(AHB1PERIPH_BASE + (index * 0x0400UL));
}

static void enable_port_clock(uint8_t index) {
    RCC->AHB1ENR |= (1UL << index);
}

void gpio_init_output(uint8_t port_pin) {
    uint8_t port_index = port_pin >> 4;
    uint8_t pin = port_pin & 0x0FU;

    enable_port_clock(port_index);
    GPIO_TypeDef *port = port_from_index(port_index);

    port->MODER &= ~(3UL << (pin * 2));
    port->MODER |= (1UL << (pin * 2)); /* 01 = sortie push-pull */
    port->OSPEEDR |= (2UL << (pin * 2)); /* vitesse rapide */
}

void gpio_write(uint8_t port_pin, bool level) {
    uint8_t port_index = port_pin >> 4;
    uint8_t pin = port_pin & 0x0FU;
    GPIO_TypeDef *port = port_from_index(port_index);

    if (level) {
        port->BSRR = (1UL << pin);
    } else {
        port->BSRR = (1UL << (pin + 16));
    }
}

void gpio_delay_ms(uint32_t ms) {
    /* Boucle d'attente approximative, non calibrée précisément (pas de
     * SysTick ici) — suffisant pour les délais XSHUT (quelques ms), pas
     * pour un timing critique. À remplacer par un délai SysTick si besoin
     * ailleurs dans le firmware. */
    for (uint32_t i = 0; i < ms; i++) {
        for (volatile uint32_t j = 0; j < 42000U; j++) { }
    }
}
