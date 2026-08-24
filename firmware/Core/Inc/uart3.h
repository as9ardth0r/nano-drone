/**
 * uart3.h — pilote USART3 minimal (registre, bloquant en écriture,
 * réception par polling avec buffer de ligne). PB10=TX, PB11=RX (AF7).
 * Relié au module BLE HM-10 (voir docs/hardware.md) — 9600 bauds par
 * défaut sur le HM-10.
 */
#ifndef UART3_H
#define UART3_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void uart3_init(uint32_t baudrate);

/* Poll non-bloquant : à appeler à chaque tour de boucle principale.
 * Accumule les octets reçus dans un buffer interne ; quand un '\n' est
 * reçu, copie la ligne (sans le '\n') dans `line_out` (taille max
 * max_len, terminée par '\0') et retourne true. Sinon retourne false
 * sans bloquer. */
bool uart3_poll_line(char *line_out, size_t max_len);

#endif /* UART3_H */
