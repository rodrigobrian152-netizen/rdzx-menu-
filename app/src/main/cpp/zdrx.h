#pragma once
#include <cstdint>
#include <cstddef>

#define ZDRX_MAGIC        0x5A44
#define ZDRX_PORTA        27015
#define ZDRX_MAX_ALVOS    68
#define ZDRX_TETO_FLOAT   280
#define ZDRX_IDADE_MS     2500
#define ZDRX_LERP_MS      200

enum {
    F_ESP = 0, F_AIM = 1, F_FOV = 2, F_SMOOTH = 3, F_WALL = 4, F_SPEED = 5,
    F_NORECOIL = 6, F_FASTLAND = 7, F_ANTENA = 8, F_BONE = 9, F_TRIGGER = 10,
    F_SILENT = 11, F_NOSPREAD = 12, F_GHOST = 13, F_JUMP = 14, F_NOFALL = 15,
    F_RADAR = 16, F_MAGIC = 17, F_FASTFIRE = 18, F_HEADSHOT = 19, F_AMMO = 20,
    G_BLOCO_A = 21, G_BLOCO_B = 22, G_BLOCO_C = 23, G_BLOCO_E = 24,
    G_BLOCO_F = 25, G_BLOCO_G = 26, G_BLOCO_H = 27,
    ZDRX_FEAT_N = 28
};

typedef struct { int32_t id; int64_t valor; } zdrx_feat;
typedef struct { float x, y, z; int32_t id; } zdrx_alvo;

typedef struct {
    uint16_t magic;
    uint16_t versao;
    float    px, py, pz, yaw, pitch, roll;
    uint16_t n;
    zdrx_alvo alvos[ZDRX_MAX_ALVOS];
} zdrx_frame;

/* estado global do alvo (equivalente a DAT_001b1a48) */
extern int32_t  g_alvo_id;
extern int32_t  g_alvo_estado;
extern uint64_t g_alvo_janela;

void     core_init(void);
int64_t  core_get(int id);
void     core_set(int id, int64_t v);
int      gate(int id);
