#pragma once
#include <cstdint>
#include <cstddef>
#include <sys/types.h>

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

/* --- acesso a memoria do processo alvo (mem.cpp) --- */
extern pid_t     g_pid;
extern int       g_mem;
extern uintptr_t g_base;
extern uintptr_t g_fim;

bool    mem_abrir(pid_t pid);
bool    mem_base(const char *modulo);
ssize_t mem_ler(uintptr_t addr, void *dst, size_t n);
ssize_t mem_escrever(uintptr_t addr, const void *src, size_t n);
bool    mem_mapeado(uintptr_t p);
size_t  mem_regioes(uintptr_t *ini, uintptr_t *fim, size_t max);

template <typename T>
inline bool ler(uintptr_t addr, T *saida) {
    return mem_ler(addr, saida, sizeof(T)) == (ssize_t)sizeof(T);
}

template <typename T>
inline bool escrever(uintptr_t addr, T valor) {
    return mem_escrever(addr, &valor, sizeof(T)) == (ssize_t)sizeof(T);
}
