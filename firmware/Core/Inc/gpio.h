#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include <stdbool.h>

/* Encodage compact port/pin : (port << 4) | pin, port A=0, B=1, C=2... */
#define GPIO_PIN(port_letter, pin_num) ((uint8_t)((((port_letter) - 'A') << 4) | (pin_num)))

void gpio_init_output(uint8_t port_pin);
void gpio_write(uint8_t port_pin, bool level);
void gpio_delay_ms(uint32_t ms);

#endif /* GPIO_H */
