#pragma once
#include <cstdint>

/* preenchidos em tempo de execucao pela calibracao automatica */
namespace off {
    extern uintptr_t LISTA_ENTIDADES;
    extern uintptr_t JOGADOR_LOCAL;

    constexpr uintptr_t CAMPO_YAW   = 0x1C;
    constexpr uintptr_t CAMPO_PITCH = 0x20;
    constexpr uintptr_t CAMPO_POS   = 0x30;
    constexpr uintptr_t ENT_X       = 0x30;
    constexpr uintptr_t ENT_Y       = 0x34;
    constexpr uintptr_t ENT_Z       = 0x38;
    constexpr uintptr_t ENT_ID      = 0x08;
    constexpr uintptr_t PASSO_ENT   = 0x40;
}

void off_limpar(void);
bool off_carregar(void);
bool off_salvar(void);
