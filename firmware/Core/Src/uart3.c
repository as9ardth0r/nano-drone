#include "uart3.h"
#include "stm32f4xx.h"

#define LINE_BUF_SIZE 64

static char line_buf[LINE_BUF_SIZE];
static size_t line_len = 0;

void uart3_init(uint32_t baudrate) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

    /* PB10 = TX, PB11 = RX, AF7 = USART3 */
    GPIOB->MODER &= ~(GPIO_MODER_MODER10 | GPIO_MODER_MODER11);
    GPIOB->MODER |= (2UL << GPIO_MODER_MODER10_Pos) | (2UL << GPIO_MODER_MODER11_Pos);
    GPIOB->AFR[1] |= (7UL << GPIO_AFRH_AFSEL10_Pos) | (7UL << GPIO_AFRH_AFSEL11_Pos);
    GPIOB->OSPEEDR |= (2UL << GPIO_OSPEEDR_OSPEED10_Pos) | (2UL << GPIO_OSPEEDR_OSPEED11_Pos);

    /* APB1 = 42 MHz (voir clock.c). USARTDIV = PCLK1 / (16 * baud),
     * encodé en Q12.4 (RM0090 §19.3.4). Calcul en virgule fixe pour
     * rester exact aux bauds standards (9600, 115200...). */
    uint32_t usartdiv_x16 = (42000000UL * 2U) / baudrate; /* *2 puis /2 plus bas : évite l'arrondi grossier */
    uint32_t mantissa = usartdiv_x16 / 32U;
    uint32_t fraction = (usartdiv_x16 / 2U) % 16U;
    USART3->BRR = (mantissa << 4) | (fraction & 0xFU);

    USART3->CR1 = USART_CR1_TE | USART_CR1_RE;
    USART3->CR1 |= USART_CR1_UE;

    line_len = 0;
}

bool uart3_poll_line(char *line_out, size_t max_len) {
    if ((USART3->SR & USART_SR_RXNE) == 0) {
        return false; /* rien de nouveau, ne bloque pas */
    }

    char c = (char)(USART3->DR & 0xFF);

    if (c == '\n' || c == '\r') {
        if (line_len == 0) return false; /* ligne vide (CRLF), rien à remonter */
        size_t copy_len = (line_len < max_len - 1) ? line_len : max_len - 1;
        for (size_t i = 0; i < copy_len; i++) { line_out[i] = line_buf[i]; }
        line_out[copy_len] = '\0';
        line_len = 0;
        return true;
    }

    if (line_len < LINE_BUF_SIZE - 1) {
        line_buf[line_len++] = c;
    } else {
        /* ligne trop longue (trame corrompue ou bruit sur le lien) :
         * on réinitialise plutôt que de laisser déborder silencieusement */
        line_len = 0;
    }
    return false;
}
