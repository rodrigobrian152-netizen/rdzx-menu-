#include "offsets.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

namespace off {
    uintptr_t LISTA_ENTIDADES = 0;
    uintptr_t JOGADOR_LOCAL   = 0;
}

static const char *ARQ = "/data/local/tmp/zdrx.off";

void off_limpar(void) {
    off::LISTA_ENTIDADES = 0;
    off::JOGADOR_LOCAL = 0;
}

bool off_carregar(void) {
    FILE *f = fopen(ARQ, "r");
    if (!f) return false;
    char chave[32];
    unsigned long v;
    while (fscanf(f, "%31s %lx", chave, &v) == 2) {
        if (!strcmp(chave, "lista"))    off::LISTA_ENTIDADES = (uintptr_t)v;
        if (!strcmp(chave, "jogador"))  off::JOGADOR_LOCAL = (uintptr_t)v;
    }
    fclose(f);
    return off::LISTA_ENTIDADES != 0;
}

bool off_salvar(void) {
    FILE *f = fopen(ARQ, "w");
    if (!f) return false;
    fprintf(f, "lista %lx\njogador %lx\n",
            (unsigned long)off::LISTA_ENTIDADES, (unsigned long)off::JOGADOR_LOCAL);
    fclose(f);
    return true;
}
