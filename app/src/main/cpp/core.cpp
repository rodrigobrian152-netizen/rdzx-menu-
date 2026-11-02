#include "zdrx.h"
#include <cstring>

static int64_t g_feat[ZDRX_FEAT_N];

/* valores iniciais: gates dos blocos desligados, defaults neutros */
static const zdrx_feat TABELA[] = {
    { F_ESP, 0 }, { F_AIM, 0 }, { F_FOV, 120 }, { F_SMOOTH, 3 },
    { F_WALL, 0 }, { F_SPEED, 0 }, { F_NORECOIL, 0 }, { F_FASTLAND, 0 },
    { F_ANTENA, 0 }, { F_BONE, 0 }, { F_TRIGGER, 0 }, { F_SILENT, 0 },
    { F_NOSPREAD, 0 }, { F_GHOST, 0 }, { F_JUMP, 0 }, { F_NOFALL, 0 },
    { F_RADAR, 1 }, { F_MAGIC, 0 }, { F_FASTFIRE, 0 }, { F_HEADSHOT, 0 },
    { F_AMMO, 0 }, { G_BLOCO_A, 0 }, { G_BLOCO_B, 0 }, { G_BLOCO_C, 0 },
    { G_BLOCO_E, 1 }, { G_BLOCO_F, 0 }, { G_BLOCO_G, 0 }, { G_BLOCO_H, 1 }
};

int32_t  g_alvo_id = 0;
int32_t  g_alvo_estado = 0;
uint64_t g_alvo_janela = 0;

void core_init(void) {
    memset(g_feat, 0, sizeof(g_feat));
    for (size_t i = 0; i < sizeof(TABELA) / sizeof(TABELA[0]); i++)
        g_feat[TABELA[i].id] = TABELA[i].valor;
}

int64_t core_get(int id) {
    return (id >= 0 && id < ZDRX_FEAT_N) ? g_feat[id] : 0;
}

void core_set(int id, int64_t v) {
    if (id >= 0 && id < ZDRX_FEAT_N) g_feat[id] = v;
}

int gate(int id) {
    return core_get(id) != 0;
}
