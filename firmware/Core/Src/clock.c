#include "stm32f4xx.h"
#include "clock.h"

/**
 * Configure l'horloge système : HSE (8 MHz, cristal externe standard sur
 * la plupart des cartes STM32F405) -> PLL -> 168 MHz SYSCLK.
 * Séquence standard STM32F4 (RM0090 §7.3.8) : HSE ON -> PLL config ->
 * PLL ON -> latence Flash -> prédiviseurs bus -> bascule SW sur PLL.
 */
void clock_init_168mhz_hse8mhz(void) {
    /* 1. Démarrer HSE et attendre sa stabilisation */
    RCC->CR |= RCC_CR_HSEON;
    while ((RCC->CR & RCC_CR_HSERDY) == 0) { }

    /* 2. Configurer le PLL : HSE(8MHz)/PLLM=8 = 1MHz -> *PLLN=336 = 336MHz
     *    -> /PLLP=2 = 168MHz (SYSCLK) ; PLLQ=7 -> 48MHz pour USB/RNG si besoin */
    RCC->PLLCFGR = (8U << RCC_PLLCFGR_PLLM_Pos)
                 | (336U << RCC_PLLCFGR_PLLN_Pos)
                 | (0U << RCC_PLLCFGR_PLLP_Pos)   /* 00 = /2 */
                 | RCC_PLLCFGR_PLLSRC_HSE
                 | (7U << RCC_PLLCFGR_PLLQ_Pos);

    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0) { }

    /* 3. Latence Flash pour 168 MHz @ 3.3V : 5 wait states (RM0090 Table 10) */
    FLASH->ACR = FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY_5WS;

    /* 4. Prédiviseurs bus : AHB /1 (168MHz), APB1 /4 (42MHz, max 42MHz),
     *    APB2 /2 (84MHz, max 84MHz) */
    RCC->CFGR = RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV4 | RCC_CFGR_PPRE2_DIV2;

    /* 5. Basculer SYSCLK sur le PLL et attendre la confirmation */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) { }

    SystemCoreClock = 168000000U;
}
