#include "motors_pwm.h"
#include "stm32f4xx.h"

#define PWM_PERIOD 1000U /* résolution permil directe : ARR = 999 */

/* TIM3 CH1=PA6, CH2=PA7, CH3=PB0, CH4=PB1 (AF2) — voir docs/hardware.md */
static volatile uint32_t *const CCR[4] = {
    &TIM3->CCR1, &TIM3->CCR2, &TIM3->CCR3, &TIM3->CCR4,
};

void motors_pwm_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    /* PA6, PA7 en alternate function AF2 = TIM3 */
    for (int pin = 6; pin <= 7; pin++) {
        GPIOA->MODER &= ~(3UL << (pin * 2));
        GPIOA->MODER |= (2UL << (pin * 2));
        GPIOA->AFR[0] |= (2UL << (pin * 4));
    }
    /* PB0, PB1 en alternate function AF2 = TIM3 */
    for (int pin = 0; pin <= 1; pin++) {
        GPIOB->MODER &= ~(3UL << (pin * 2));
        GPIOB->MODER |= (2UL << (pin * 2));
        GPIOB->AFR[0] |= (2UL << (pin * 4));
    }

    /* APB1 timer clock = 84 MHz (APB1 42MHz x2, règle RM0090 §7.2) */
    TIM3->PSC = 2; /* 84MHz / 3 = 28MHz ; 28e6/1000 ≈ 28kHz (proche des 24kHz visés) */
    TIM3->ARR = PWM_PERIOD - 1;

    /* PWM mode 1 sur les 4 canaux, preload activé */
    TIM3->CCMR1 = (6U << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE
                | (6U << TIM_CCMR1_OC2M_Pos) | TIM_CCMR1_OC2PE;
    TIM3->CCMR2 = (6U << TIM_CCMR2_OC3M_Pos) | TIM_CCMR2_OC3PE
                | (6U << TIM_CCMR2_OC4M_Pos) | TIM_CCMR2_OC4PE;

    TIM3->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;

    for (int i = 0; i < 4; i++) { *CCR[i] = 0; }

    TIM3->CR1 |= TIM_CR1_ARPE;
    TIM3->EGR |= TIM_EGR_UG;
    TIM3->CR1 |= TIM_CR1_CEN;
}

void motors_set(motor_id_t motor, uint16_t duty_permil) {
    if (duty_permil > PWM_PERIOD) duty_permil = PWM_PERIOD;
    *CCR[motor] = duty_permil;
}

void motors_stop_all(void) {
    for (int i = 0; i < 4; i++) { *CCR[i] = 0; }
}
