#include "zdrx.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/uio.h>

pid_t     g_pid  = -1;
int       g_mem  = -1;
uintptr_t g_base = 0;
uintptr_t g_fim  = 0;

static bool mapa(const char *modulo, uintptr_t *ini, uintptr_t *fim) {
    char p[64];
    snprintf(p, sizeof(p), "/proc/%d/maps", g_pid);
    FILE *f = fopen(p, "r");
    if (!f) return false;
    char linha[512];
    bool achou = false;
    *fim = 0;
    while (fgets(linha, sizeof(linha), f)) {
        uintptr_t a, b;
        char perm[8] = {0};
        char nome[256] = {0};
        if (sscanf(linha, "%lx-%lx %7s %*s %*s %*s %255s", &a, &b, perm, nome) < 3) continue;
        if (!strstr(nome, modulo)) continue;
        if (!achou) { *ini = a; achou = true; }
        if (b > *fim) *fim = b;
    }
    fclose(f);
    return achou;
}

bool mem_abrir(pid_t pid) {
    g_pid = pid;
    char p[64];
    snprintf(p, sizeof(p), "/proc/%d/mem", pid);
    g_mem = open(p, O_RDWR);
    return g_mem >= 0;
}

bool mem_base(const char *modulo) {
    uintptr_t ini = 0;
    if (!mapa(modulo, &ini, &g_fim)) return false;
    g_base = ini;
    return g_fim > g_base;
}

ssize_t mem_ler(uintptr_t addr, void *dst, size_t n) {
    struct iovec lo = { dst, n };
    struct iovec re = { (void *)addr, n };
    ssize_t r = process_vm_readv(g_pid, &lo, 1, &re, 1, 0);
    if (r == (ssize_t)n) return r;
    if (g_mem < 0) return -1;
    return pread(g_mem, dst, n, (off_t)addr);
}

ssize_t mem_escrever(uintptr_t addr, const void *src, size_t n) {
    struct iovec lo = { (void *)src, n };
    struct iovec re = { (void *)addr, n };
    ssize_t r = process_vm_writev(g_pid, &lo, 1, &re, 1, 0);
    if (r == (ssize_t)n) return r;
    if (g_mem < 0) return -1;
    return pwrite(g_mem, src, n, (off_t)addr);
}

template <typename T>
bool ler(uintptr_t addr, T *saida) {
    return mem_ler(addr, saida, sizeof(T)) == (ssize_t)sizeof(T);
}

template <typename T>
bool escrever(uintptr_t addr, T valor) {
    return mem_escrever(addr, &valor, sizeof(T)) == (ssize_t)sizeof(T);
}

/* o endereco esta mapeado e legivel? */
bool mem_mapeado(uintptr_t p) {
    if (p < 0x1000) return false;
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/maps", g_pid);
    FILE *f = fopen(path, "r");
    if (!f) return false;
    char linha[512];
    bool ok = false;
    while (fgets(linha, sizeof(linha), f)) {
        uintptr_t a, b;
        char perm[8] = {0};
        if (sscanf(linha, "%lx-%lx %7s", &a, &b, perm) < 3) continue;
        if (p >= a && p < b) { ok = (perm[0] == 'r'); break; }
    }
    fclose(f);
    return ok;
}

/* regioes gravaveis e nao executaveis dentro do modulo = onde ficam as globais */
size_t mem_regioes(uintptr_t *ini, uintptr_t *fim, size_t max) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/maps", g_pid);
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    char linha[512];
    size_t n = 0;
    while (fgets(linha, sizeof(linha), f) && n < max) {
        uintptr_t a, b;
        char perm[8] = {0};
        char nome[256] = {0};
        if (sscanf(linha, "%lx-%lx %7s %*s %*s %*s %255s", &a, &b, perm, nome) < 3) continue;
        if (!strstr(nome, "libil2cpp")) continue;
        if (perm[0] != 'r' || perm[1] != 'w' || perm[2] == 'x') continue;
        if (b - a < 8) continue;
        ini[n] = a; fim[n] = b; n++;
    }
    fclose(f);
    return n;
}
