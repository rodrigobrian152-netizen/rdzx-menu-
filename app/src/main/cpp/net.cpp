#include "zdrx.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <unistd.h>
#include <time.h>

/* porta fiel dos blocos A..I do nProcessC2S.
 * Sem hook de pacote os blocos rodam sobre o buffer que o chamador entregar;
 * a assinatura segue a original: retorna 1 quando enfileirou modificacao. */

static inline int32_t zigzag(int32_t v)  { return (v << 1) ^ (v >> 31); }
static inline int32_t unzig(int32_t v)   { return (v >> 1) ^ -(v & 1); }

static uint64_t agora_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ull + (uint64_t)ts.tv_nsec / 1000000ull;
}

static int g_dumps = 0;
static int g_cont_b = 0;
static int g_cont_c = 0;

/* A — decode + hexdump, limite de 16 dumps */
int net_bloco_a(const uint8_t *b, size_t n) {
    if (!gate(G_BLOCO_A) || g_dumps >= 16 || n < 5) return 0;
    g_dumps++;
    printf("[A] tipo=%02x %02x %02x %02x %02x len=%zu\n", b[0], b[1], b[2], b[3], b[4], n);
    return 0;
}

/* B — trava 3 campos do jogador */
static int32_t g_travado[3];
int net_bloco_b(const uint8_t *b, size_t n) {
    if (!gate(G_BLOCO_B) || n < 0x40) return 0;
    int32_t v1 = unzig(*(int32_t *)(b + 0x20));
    int32_t v2 = unzig(*(int32_t *)(b + 0x30));
    int32_t v3 = unzig(*(int32_t *)(b + 0x40));
    if (v1 == g_travado[0] && v2 == g_travado[1] && v3 == g_travado[2]) return 0;
    g_travado[0] = v1; g_travado[1] = v2; g_travado[2] = v3;
    g_cont_b++;
    if (g_cont_b < 4 || g_cont_b % 200 == 0)
        printf("[B] campos travados %d %d %d\n", v1, v2, v3);
    return 1;
}

/* C — normaliza 64 caracteres hex */
int net_bloco_c(uint8_t *b, size_t n) {
    if (!gate(G_BLOCO_C) || n < 0x40) return 0;
    for (size_t i = 0; i < 0x40; i++) {
        char c = (char)b[i];
        int hex = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
        if (!hex) b[i] = '0';
    }
    g_cont_c++;
    if (g_cont_c < 4) printf("[C] normalizado\n");
    return 1;
}

/* E — aimbot: correcao da mira no pacote */
float g_mira_x = 0, g_mira_y = 0;
static uint64_t g_mira_t = 0;
static int g_correcoes = 0;

int net_bloco_e(const uint8_t *b, size_t n) {
    if (!gate(G_BLOCO_E) || g_alvo_id == 0 || g_alvo_estado != 1) return 0;
    if (n < 0x31) return 0;
    int32_t id_reg = *(int32_t *)(b + 0x08);
    if (id_reg != g_alvo_id) return 0;

    uint64_t agora = agora_ms();
    if (g_mira_t == 0) g_mira_t = agora;

    float dx = 0.0f, dy = 0.0f;
    memcpy(&dx, b + 0x30, 4);
    memcpy(&dy, b + 0x34, 4);

    float dist = sqrtf(dx * dx + dy * dy);
    float dt = (float)(agora - g_mira_t) / 1000.0f;
    if (dt > 0.5f) dt = 0.5f;
    float taxa = (float)core_get(F_SMOOTH);
    float passo = (taxa > 0.0f ? 1000.0f / taxa : 0.0f) * dt;

    if (passo < dist && dist >= 1.0f) {
        g_mira_x += passo * (dx / dist);
        g_mira_y += passo * (dy / dist);
    } else {
        g_mira_x = dx;
        g_mira_y = dy;
    }
    g_mira_t = agora;
    g_correcoes++;
    if (g_correcoes < 7 || g_correcoes % 100 == 0)
        printf("[E] mira %d -> %.2f %.2f (d=%.1f)\n", g_correcoes, g_mira_x, g_mira_y, dist);
    return 1;
}

/* F — comandos 3D/4D junto ao alvo */
int net_bloco_f(const uint8_t *b, size_t n) {
    if (!gate(G_BLOCO_F) || g_alvo_id == 0 || n < 0x81) return 0;
    int32_t x = unzig(*(int32_t *)(b + 0x20));
    int32_t y = unzig(*(int32_t *)(b + 0x30));
    int32_t z = unzig(*(int32_t *)(b + 0x40));
    printf("[F] comando %d %d %d\n", x, y, z);
    return 1;
}

/* G — normalizacao numerica */
int net_bloco_g(const uint8_t *b, size_t n) {
    if (!gate(G_BLOCO_G) || n < 0x11) return 0;
    int32_t campo = *(int32_t *)(b + 0x0C);
    int32_t novo = campo > 9999 ? campo - 10000 : 0;
    printf("[G] campo %d -> %d\n", campo, novo);
    return 1;
}

/* H — trava de alvo: le o id da quinta entidade e grava em g_alvo_id */
int net_bloco_h(const uint8_t *b, size_t n) {
    if (!gate(G_BLOCO_H) || n < 0x50) return 0;
    int32_t id = *(int32_t *)(b + 0x08 * 4);
    if (id == 0) return 0;
    if (g_alvo_id == 0) {
        g_alvo_estado = 2;
        g_alvo_janela = agora_ms() + 60000ull;
        printf("[H] alvo travado %d\n", id);
    }
    g_alvo_id = id;
    return 1;
}

/* I — feature extra */
int net_bloco_i(const uint8_t *b, size_t n) {
    if (!gate(G_BLOCO_A) || n < 0x40) return 0;
    printf("[I] extra v7=%d v1=%d v3=%d\n",
           *(int32_t *)(b + 0x38), *(int32_t *)(b + 0x08), *(int32_t *)(b + 0x18));
    return 1;
}

int net_c2s(uint8_t *b, size_t n) {
    if (n < 8) return 0;
    int m = 0;
    m |= net_bloco_a(b, n);
    m |= net_bloco_b(b, n);
    m |= net_bloco_c(b, n);
    m |= net_bloco_e(b, n);
    m |= net_bloco_f(b, n);
    m |= net_bloco_g(b, n);
    m |= net_bloco_h(b, n);
    m |= net_bloco_i(b, n);
    return m;
}
