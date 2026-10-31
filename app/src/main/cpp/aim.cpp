#include "zdrx.h"
#include "offsets.h"
#include <cmath>
#include <cstring>

ssize_t mem_ler(uintptr_t, void *, size_t);
template <typename T> bool escrever(uintptr_t, T);

void aim_tick(void) {
    if (!gate(F_AIM)) return;
    uintptr_t eu = off::JOGADOR_LOCAL;
    if (!eu || !off::LISTA_ENTIDADES) return;

    float px = 0, py = 0, pz = 0, yaw = 0, pitch = 0;
    if (mem_ler(eu + off::CAMPO_POS + 0, &px, 4) != 4) return;
    mem_ler(eu + off::CAMPO_POS + 4, &py, 4);
    mem_ler(eu + off::CAMPO_POS + 8, &pz, 4);
    mem_ler(eu + off::CAMPO_YAW, &yaw, 4);
    mem_ler(eu + off::CAMPO_PITCH, &pitch, 4);

    float fov = (float)core_get(F_FOV);
    float melhor = fov * fov;
    float tx = 0, ty = 0, tz = 0;
    bool achou = false;

    for (int i = 0; i < 100; i++) {
        uintptr_t e = off::LISTA_ENTIDADES + (uintptr_t)i * off::PASSO_ENT;
        int32_t id = 0;
        float x = 0, y = 0, z = 0;
        if (mem_ler(e + off::ENT_ID, &id, 4) != 4 || id <= 0) break;
        if (mem_ler(e + off::ENT_X, &x, 4) != 4) break;
        if (mem_ler(e + off::ENT_Y, &y, 4) != 4) break;
        if (mem_ler(e + off::ENT_Z, &z, 4) != 4) break;

        float dx = x - px, dy = y - py, dz = z - pz;
        float d2 = dx * dx + dy * dy + dz * dz;
        if (d2 < melhor) { melhor = d2; tx = x; ty = y; tz = z; achou = true; }
    }
    if (!achou) return;

    float dx = tx - px, dy = ty - py, dz = tz - pz;
    float alvo_yaw = atan2f(dy, dx) * 57.29578f;
    float alvo_pitch = -atan2f(dz, sqrtf(dx * dx + dy * dy)) * 57.29578f;

    float dyaw = alvo_yaw - yaw;
    while (dyaw > 180.0f) dyaw -= 360.0f;
    while (dyaw < -180.0f) dyaw += 360.0f;
    float dpitch = alvo_pitch - pitch;

    float suave = (float)core_get(F_SMOOTH);
    if (suave < 1.0f) suave = 1.0f;
    float dist = sqrtf(dyaw * dyaw + dpitch * dpitch);

    float ny, np;
    if (dist < 0.6f) { ny = alvo_yaw; np = alvo_pitch; }
    else { ny = yaw + dyaw / suave; np = pitch + dpitch / suave; }

    escrever(eu + off::CAMPO_YAW, ny);
    escrever(eu + off::CAMPO_PITCH, np);
}
