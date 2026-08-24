#include "command_link.h"
#include <string.h>

static uint8_t checksum_of(const char *payload, int len) {
    uint8_t value = 0;
    for (int i = 0; i < len; i++) {
        value ^= (uint8_t)payload[i];
    }
    return value;
}

static bool parse_hex_byte(const char *s, uint8_t *out) {
    uint8_t result = 0;
    for (int i = 0; i < 2; i++) {
        char c = s[i];
        uint8_t nibble;
        if (c >= '0' && c <= '9') nibble = (uint8_t)(c - '0');
        else if (c >= 'A' && c <= 'F') nibble = (uint8_t)(c - 'A' + 10);
        else if (c >= 'a' && c <= 'f') nibble = (uint8_t)(c - 'a' + 10);
        else return false;
        result = (uint8_t)((result << 4) | nibble);
    }
    *out = result;
    return true;
}

/* Parse un entier signé décimal à partir de *s, avance *s après le
 * dernier chiffre consommé. Retourne false si aucun chiffre trouvé. */
static bool parse_int(const char **s, int32_t *out) {
    const char *p = *s;
    bool negative = false;
    if (*p == '-') { negative = true; p++; }

    if (*p < '0' || *p > '9') return false;

    int32_t value = 0;
    while (*p >= '0' && *p <= '9') {
        value = value * 10 + (*p - '0');
        p++;
    }
    *out = negative ? -value : value;
    *s = p;
    return true;
}

bool cl_parse_command(const char *line, cl_command_t *out) {
    /* trouve '*' */
    const char *star = strchr(line, '*');
    if (star == NULL || line[0] != 'V') return false;

    int payload_len = (int)(star - line);
    uint8_t computed = checksum_of(line, payload_len);

    uint8_t expected;
    if (!parse_hex_byte(star + 1, &expected)) return false;
    if (computed != expected) return false;

    const char *p = line + 1; /* après le 'V' */
    int32_t vx, vy, vz;

    if (!parse_int(&p, &vx)) return false;
    if (*p != ',') return false;
    p++;
    if (!parse_int(&p, &vy)) return false;
    if (*p != ',') return false;
    p++;
    if (!parse_int(&p, &vz)) return false;
    if (p != star) return false; /* pas de caractères de trop avant '*' */

    out->vx_mm_s = vx;
    out->vy_mm_s = vy;
    out->vz_mm_s = vz;
    return true;
}

void cl_link_init(cl_link_state_t *link, float failsafe_timeout_s) {
    link->failsafe_timeout_s = failsafe_timeout_s;
    link->last_command.vx_mm_s = 0;
    link->last_command.vy_mm_s = 0;
    link->last_command.vz_mm_s = 0;
    link->last_received_at_s = 0.0f;
    link->has_received_ever = false;
}

bool cl_on_line_received(cl_link_state_t *link, const char *line, float now_s) {
    cl_command_t cmd;
    if (!cl_parse_command(line, &cmd)) return false;
    link->last_command = cmd;
    link->last_received_at_s = now_s;
    link->has_received_ever = true;
    return true;
}

cl_command_t cl_get_desired_velocity(const cl_link_state_t *link, float now_s) {
    cl_command_t zero = {0, 0, 0};
    if (!link->has_received_ever) return zero;
    if (now_s - link->last_received_at_s > link->failsafe_timeout_s) return zero;
    return link->last_command;
}
