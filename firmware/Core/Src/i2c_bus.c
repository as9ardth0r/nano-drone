#include "i2c_bus.h"
#include "stm32f4xx.h"

#define I2C_TIMEOUT_LOOPS 100000U

static i2c_status_t wait_flag(volatile uint32_t *reg, uint32_t mask, uint32_t loops) {
    while ((*reg & mask) == 0) {
        if (--loops == 0) return I2C_ERR_TIMEOUT;
    }
    return I2C_OK;
}

void i2c1_init(void) {
    /* Horloges : GPIOB + I2C1 */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    /* PB6/PB7 en alternate function open-drain, AF4 = I2C1 */
    GPIOB->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7);
    GPIOB->MODER |= (2U << GPIO_MODER_MODER6_Pos) | (2U << GPIO_MODER_MODER7_Pos);
    GPIOB->OTYPER |= GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7;
    GPIOB->OSPEEDR |= (3U << GPIO_OSPEEDR_OSPEED6_Pos) | (3U << GPIO_OSPEEDR_OSPEED7_Pos);
    GPIOB->PUPDR |= (1U << GPIO_PUPDR_PUPD6_Pos) | (1U << GPIO_PUPDR_PUPD7_Pos); /* pull-up */
    GPIOB->AFR[0] |= (4U << GPIO_AFRL_AFSEL6_Pos) | (4U << GPIO_AFRL_AFSEL7_Pos);

    I2C1->CR1 = I2C_CR1_SWRST;
    I2C1->CR1 = 0;

    /* APB1 = 42 MHz (voir clock.c) */
    I2C1->CR2 = 42U;
    /* 400 kHz mode rapide, duty 16/9 : formule standard RM0090 §27.6.8 */
    I2C1->CCR = I2C_CCR_FS | I2C_CCR_DUTY | (35U & I2C_CCR_CCR_Msk);
    I2C1->TRISE = 13U;

    I2C1->CR1 |= I2C_CR1_PE;
}

static i2c_status_t start_and_address(uint8_t addr7, uint8_t read_bit) {
    I2C1->CR1 |= I2C_CR1_START;
    if (wait_flag(&I2C1->SR1, I2C_SR1_SB, I2C_TIMEOUT_LOOPS) != I2C_OK) return I2C_ERR_TIMEOUT;

    I2C1->DR = (uint8_t)((addr7 << 1) | read_bit);
    uint32_t loops = I2C_TIMEOUT_LOOPS;
    while ((I2C1->SR1 & I2C_SR1_ADDR) == 0) {
        if (I2C1->SR1 & I2C_SR1_AF) { I2C1->SR1 &= ~I2C_SR1_AF; return I2C_ERR_NACK; }
        if (--loops == 0) return I2C_ERR_TIMEOUT;
    }
    (void)I2C1->SR1; (void)I2C1->SR2; /* clear ADDR (lecture SR1 puis SR2) */
    return I2C_OK;
}

i2c_status_t i2c1_write(uint8_t addr7, const uint8_t *data, size_t len) {
    i2c_status_t st = start_and_address(addr7, 0);
    if (st != I2C_OK) return st;

    for (size_t i = 0; i < len; i++) {
        if (wait_flag(&I2C1->SR1, I2C_SR1_TXE, I2C_TIMEOUT_LOOPS) != I2C_OK) return I2C_ERR_TIMEOUT;
        I2C1->DR = data[i];
    }
    if (wait_flag(&I2C1->SR1, I2C_SR1_BTF, I2C_TIMEOUT_LOOPS) != I2C_OK) return I2C_ERR_TIMEOUT;
    I2C1->CR1 |= I2C_CR1_STOP;
    return I2C_OK;
}

i2c_status_t i2c1_read(uint8_t addr7, uint8_t *data, size_t len) {
    i2c_status_t st = start_and_address(addr7, 1);
    if (st != I2C_OK) return st;

    if (len == 1) {
        I2C1->CR1 &= ~I2C_CR1_ACK;
        I2C1->CR1 |= I2C_CR1_STOP;
        if (wait_flag(&I2C1->SR1, I2C_SR1_RXNE, I2C_TIMEOUT_LOOPS) != I2C_OK) return I2C_ERR_TIMEOUT;
        data[0] = (uint8_t)I2C1->DR;
        I2C1->CR1 |= I2C_CR1_ACK;
        return I2C_OK;
    }

    I2C1->CR1 |= I2C_CR1_ACK;
    for (size_t i = 0; i < len; i++) {
        if (i == len - 2) {
            /* NACK sur le dernier octet, STOP juste avant sa réception —
             * séquence standard fin-de-lecture I2C STM32 (RM0090 §27.3.3) */
            I2C1->CR1 &= ~I2C_CR1_ACK;
            I2C1->CR1 |= I2C_CR1_STOP;
        }
        if (wait_flag(&I2C1->SR1, I2C_SR1_RXNE, I2C_TIMEOUT_LOOPS) != I2C_OK) return I2C_ERR_TIMEOUT;
        data[i] = (uint8_t)I2C1->DR;
    }
    I2C1->CR1 |= I2C_CR1_ACK;
    return I2C_OK;
}

i2c_status_t i2c1_write_read(uint8_t addr7, const uint8_t *reg, size_t reg_len,
                              uint8_t *data, size_t data_len) {
    i2c_status_t st = i2c1_write(addr7, reg, reg_len);
    if (st != I2C_OK) return st;
    return i2c1_read(addr7, data, data_len);
}
