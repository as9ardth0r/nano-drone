/**
 * command_link.h — parsing du protocole de commande BLE + logique de
 * repli sécuritaire. Indépendant du matériel (pas d'inclusion STM32,
 * pas d'accès direct à l'UART) pour rester testable nativement et
 * comparable à sim/nanodrone_sim/command_link.py — voir
 * tests/test_command_link_c_matches_python.py.
 */
#ifndef NANODRONE_COMMAND_LINK_H
#define NANODRONE_COMMAND_LINK_H

#include <stdbool.h>
#include <stdint.h>

#define CL_FAILSAFE_TIMEOUT_S 0.5f

typedef struct {
    int32_t vx_mm_s, vy_mm_s, vz_mm_s;
} cl_command_t;

typedef struct {
    float failsafe_timeout_s;
    cl_command_t last_command;
    float last_received_at_s;
    bool has_received_ever;
} cl_link_state_t;

void cl_link_init(cl_link_state_t *link, float failsafe_timeout_s);

/* Parse une ligne terminée par '\0' (le '\n' final, s'il est présent,
 * est ignoré). Retourne false si le format ou le checksum est invalide
 * — ne modifie alors pas *out. */
bool cl_parse_command(const char *line, cl_command_t *out);

/* Met à jour l'état du lien si `line` est une trame valide. Retourne
 * true si l'état a été mis à jour. */
bool cl_on_line_received(cl_link_state_t *link, const char *line, float now_s);

/* Vitesse désirée courante, avec repli sécuritaire (0,0,0) si le lien
 * est silencieux depuis plus de failsafe_timeout_s. */
cl_command_t cl_get_desired_velocity(const cl_link_state_t *link, float now_s);

#endif /* NANODRONE_COMMAND_LINK_H */
