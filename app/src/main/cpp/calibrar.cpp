#include "zdrx.h"
#include "offsets.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

bool mem_mapeado(uintptr_t);
size_t mem_regioes(uintptr_t *, uintptr_t *, size_t);
ssize_t mem_ler(uintptr_t, void *, size_t);

#define MAX_REGIOES 64
#define MAX_CAND    256

static bool coordenada_ok(float v) {
    if (!isfinite(v)) return false;
    float m = fabsf(v);
    return m > 1.0f && m < 500000.0f;
}

/* le ate 24 entidades a partir do candidato e devolve quantas parecem reais */
static int validar_lista(uintptr_t p) {
    if (!mem_mapeado(p)) return 0;
    int bons = 0;
    for (int i = 0; i < 24; i++) {
        uintptr_t e = p + (uintptr_t)i * off::PASSO_ENT;
        int32_t id = 0;
        float x = 0, y = 0, z = 0;
        if (mem_ler(e + off::ENT_ID, &id, 4) != 4) break;
        if (id <= 0 || id > 0x7FFFFFFF) {
            if (i == 0) return 0;
            break;
        }
        if (mem_ler(e + off::ENT_X, &x, 4) != 4) break;
        if (mem_ler(e + off::ENT_Y, &y, 4) != 4) break;
        if (mem_ler(e + off::ENT_Z, &z, 4) != 4) break;
        if (!coordenada_ok(x) || !coordenada_ok(y) || fabsf(z) > 200000.0f) break;
        bons++;
    }
    return bons;
}

static bool validar_jogador(uintptr_t p) {
    if (!mem_mapeado(p)) return false;
    float yaw = 0, pitch = 0;
    if (mem_ler(p + off::CAMPO_YAW, &yaw, 4) != 4) return false;
    if (mem_ler(p + off::CAMPO_PITCH, &pitch, 4) != 4) return false;
    if (!isfinite(yaw) || !isfinite(pitch)) return false;
    if (fabsf(yaw) > 360.0f || fabsf(pitch) > 90.0f) return false;
    if (fabsf(yaw) < 0.001f && fabsf(pitch) < 0.001f) return false;
    return true;
}

/* varre as regioes gravaveis atras de globais com cara de lista de entidades */
bool calibrar_lista(int *relatorio) {
    uintptr_t ini[MAX_REGIOES], fim[MAX_REGIOES];
    size_t nr = mem_regioes(ini, fim, MAX_REGIOES);
    if (!nr) return false;

    uintptr_t cand[MAX_CAND];
    int nc = 0;
    for (size_t r = 0; r < nr && nc < MAX_CAND; r++) {
        size_t tam = (size_t)(fim[r] - ini[r]);
        uint8_t *buf = (uint8_t *)malloc(tam);
        if (!buf) continue;
        if (mem_ler(ini[r], buf, tam) != (ssize_t)tam) { free(buf); continue; }
        for (size_t o = 0; o + 8 <= tam && nc < MAX_CAND; o += 8) {
            uintptr_t v;
            memcpy(&v, buf + o, 8);
            if (v < 0x10000000ull || v > 0x00007fffffffffffull) continue;
            if (validar_lista(v) >= 2) cand[nc++] = v;
        }
        free(buf);
    }
    if (relatorio) relatorio[0] = nc;
    if (!nc) return false;

    uintptr_t melhor = 0;
    int nota = 0;
    for (int i = 0; i < nc; i++) {
        int v = validar_lista(cand[i]);
        if (v > nota) { nota = v; melhor = cand[i]; }
    }
    if (!melhor || nota < 2) return false;
    off::LISTA_ENTIDADES = melhor;
    if (relatorio) relatorio[1] = nota;
    return true;
}

/* acha a struct do jogador local pelas faixas de yaw/pitch */
bool calibrar_jogador(int *relatorio) {
    if (!off::LISTA_ENTIDADES) return false;
    int nota_lista = validar_lista(off::LISTA_ENTIDADES);
    if (nota_lista < 1) return false;

    float lx = 0, ly = 0, lz = 0;
    if (mem_ler(off::LISTA_ENTIDADES + off::ENT_X, &lx, 4) != 4) return false;
    mem_ler(off::LISTA_ENTIDADES + off::ENT_Y, &ly, 4);
    mem_ler(off::LISTA_ENTIDADES + off::ENT_Z, &lz, 4);

    uintptr_t ini[MAX_REGIOES], fim[MAX_REGIOES];
    size_t nr = mem_regioes(ini, fim, MAX_REGIOES);
    int nc = 0;
    for (size_t r = 0; r < nr && nc < MAX_CAND; r++) {
        size_t tam = (size_t)(fim[r] - ini[r]);
        uint8_t *buf = (uint8_t *)malloc(tam);
        if (!buf) continue;
        if (mem_ler(ini[r], buf, tam) != (ssize_t)tam) { free(buf); continue; }
        for (size_t o = 0; o + 8 <= tam && nc < MAX_CAND; o += 8) {
            uintptr_t v;
            memcpy(&v, buf + o, 8);
            if (v < 0x10000000ull || v > 0x00007fffffffffffull) continue;
            if (validar_jogador(v)) { nc++; break; }
        }
        free(buf);
    }
    if (relatorio) relatorio[0] = nc;
    if (!nc) return false;

    /* preferencia: o candidato cuja posicao bate com a primeira entidade da lista */
    uintptr_t melhor = 0;
    float melhor_d = 1e18f;
    for (size_t r = 0; r < nr && !melhor; r++) {
        size_t tam = (size_t)(fim[r] - ini[r]);
        uint8_t *buf = (uint8_t *)malloc(tam);
        if (!buf) continue;
        if (mem_ler(ini[r], buf, tam) != (ssize_t)tam) { free(buf); continue; }
        for (size_t o = 0; o + 8 <= tam; o += 8) {
            uintptr_t v;
            memcpy(&v, buf + o, 8);
            if (v < 0x10000000ull || v > 0x00007fffffffffffull) continue;
            if (!validar_jogador(v)) continue;
            float x = 0, y = 0, z = 0;
            mem_ler(v + off::CAMPO_POS + 0, &x, 4);
            mem_ler(v + off::CAMPO_POS + 4, &y, 4);
            mem_ler(v + off::CAMPO_POS + 8, &z, 4);
            float d = fabsf(x - lx) + fabsf(y - ly) + fabsf(z - lz);
            if (d < melhor_d) { melhor_d = d; melhor = v; }
        }
        free(buf);
    }
    if (!melhor) return false;
    off::JOGADOR_LOCAL = melhor;
    if (relatorio) relatorio[1] = (int)melhor_d;
    return true;
}
