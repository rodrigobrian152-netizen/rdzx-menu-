#include "zdrx.h"
#include "offsets.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <unistd.h>

bool mem_mapeado(uintptr_t);
ssize_t mem_ler(uintptr_t, void *, size_t);

typedef struct {
    int32_t  id;
    float    x, y, z;
    float    dx, dy, dz;
    uint64_t t_visao;
    uint64_t t_ini;
} ent;

static ent g_ent[128];
static int g_n = 0;

static uint64_t agora_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ull + (uint64_t)ts.tv_nsec / 1000000ull;
}

/* anda a lista ate achar id 0 (a calibracao nao depende de contagem) */
void esp_coletar(void) {
    g_n = 0;
    if (!off::LISTA_ENTIDADES) return;
    uint64_t agora = agora_ms();

    for (int i = 0; i < 100 && g_n < 128; i++) {
        uintptr_t e = off::LISTA_ENTIDADES + (uintptr_t)i * off::PASSO_ENT;
        int32_t id = 0;
        float x = 0, y = 0, z = 0;
        if (mem_ler(e + off::ENT_ID, &id, 4) != 4 || id <= 0) break;
        if (mem_ler(e + off::ENT_X, &x, 4) != 4) break;
        if (mem_ler(e + off::ENT_Y, &y, 4) != 4) break;
        if (mem_ler(e + off::ENT_Z, &z, 4) != 4) break;
        if (fabsf(x) < 1.0f && fabsf(y) < 1.0f) continue;

        ent *slot = nullptr;
        for (int k = 0; k < g_n; k++) if (g_ent[k].id == id) { slot = &g_ent[k]; break; }
        if (!slot) { slot = &g_ent[g_n++]; memset(slot, 0, sizeof(*slot)); slot->id = id; slot->t_ini = agora; }
        slot->dx = slot->x; slot->dy = slot->y; slot->dz = slot->z;
        slot->x = x; slot->y = y; slot->z = z;
        slot->t_visao = agora;
    }
}

void esp_montar(zdrx_frame *f) {
    memset(f, 0, sizeof(*f));
    f->magic = ZDRX_MAGIC;
    f->versao = 3;

    uintptr_t eu = off::JOGADOR_LOCAL;
    if (eu) {
        mem_ler(eu + off::CAMPO_POS + 0, &f->px, 4);
        mem_ler(eu + off::CAMPO_POS + 4, &f->py, 4);
        mem_ler(eu + off::CAMPO_POS + 8, &f->pz, 4);
        mem_ler(eu + off::CAMPO_YAW, &f->yaw, 4);
        mem_ler(eu + off::CAMPO_PITCH, &f->pitch, 4);
    }

    uint64_t agora = agora_ms();
    int saida = 0;
    for (int i = 0; i < g_n && saida < ZDRX_MAX_ALVOS; i++) {
        ent *e = &g_ent[i];
        uint64_t idade = agora - e->t_visao;
        if (idade >= ZDRX_IDADE_MS) continue;

        float x = e->x, y = e->y, z = e->z;
        uint64_t dur = e->t_visao - e->t_ini;
        if (dur > 0 && dur < 500) {
            float t = (float)idade / (float)dur;
            float lim = (float)ZDRX_LERP_MS / 1000.0f;
            if (t > lim) t = lim;
            x = e->x + (e->x - e->dx) * t;
            y = e->y + (e->y - e->dy) * t;
            z = e->z + (e->z - e->dz) * t;
        }
        f->alvos[saida].x = x / 1000.0f;
        f->alvos[saida].y = y / 1000.0f;
        f->alvos[saida].z = z / 1000.0f;
        f->alvos[saida].id = e->id;
        saida++;
    }
    f->n = (uint16_t)saida;
}
