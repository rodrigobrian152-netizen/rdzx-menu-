#include "zdrx.h"
#include "offsets.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <atomic>

bool mem_abrir(pid_t);
bool mem_base(const char *);
void esp_coletar(void);
void esp_montar(zdrx_frame *);
void aim_tick(void);
bool calibrar_lista(int *);
bool calibrar_jogador(int *);

extern pid_t     g_pid;
extern uintptr_t g_base;

static std::atomic<bool> g_rodando{true};
static bool g_servidor = false;

static void status(const char *txt) {
    FILE *f = fopen("/data/local/tmp/zdrx.status", "w");
    if (!f) return;
    fprintf(f, "%s\n", txt);
    fclose(f);
}

static pid_t achar_pid(const char *pkg) {
    FILE *f = popen("ps -A -o PID,NAME 2>/dev/null", "r");
    if (!f) return -1;
    char linha[256];
    pid_t pid = -1;
    while (fgets(linha, sizeof(linha), f)) {
        if (strstr(linha, pkg)) { pid = (pid_t)atoi(linha); break; }
    }
    pclose(f);
    return pid;
}

static bool vivo(pid_t pid) {
    return kill(pid, 0) == 0;
}

static void servidor(void) {
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) return;
    int um = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &um, sizeof(um));
    struct sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_port = htons(ZDRX_PORTA);
    a.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (bind(srv, (struct sockaddr *)&a, sizeof(a)) < 0) { close(srv); return; }
    listen(srv, 1);
    while (g_rodando) {
        int c = accept(srv, nullptr, nullptr);
        if (c < 0) continue;
        while (g_rodando) {
            zdrx_frame f;
            esp_coletar();
            esp_montar(&f);
            if (send(c, &f, sizeof(f), 0) <= 0) break;
            usleep(33000);
        }
        close(c);
    }
    close(srv);
}

/* uma passada de calibracao; devolve true quando lista e jogador saem validos */
static bool calibrar(pid_t pid) {
    int rel[2] = {0, 0};
    if (!calibrar_lista(rel)) {
        char b[96];
        snprintf(b, sizeof(b), "calibrando: lista sem candidato (%d regioes)", rel[0]);
        status(b);
        return false;
    }
    char b[96];
    snprintf(b, sizeof(b), "calibrando: lista ok (%d entidades), procurando jogador", rel[1]);
    status(b);

    if (!calibrar_jogador(rel)) {
        snprintf(b, sizeof(b), "calibrando: jogador sem candidato (%d)", rel[0]);
        status(b);
        off_limpar();
        return false;
    }
    off_salvar();
    snprintf(b, sizeof(b), "rodando: lista=%lx jogador=%lx",
             (unsigned long)off::LISTA_ENTIDADES, (unsigned long)off::JOGADOR_LOCAL);
    status(b);
    printf("%s\n", b);
    return true;
}

static int modo_auto(void) {
    if (!g_servidor) { std::thread(servidor).detach(); g_servidor = true; }

    for (;;) {
        status("aguardando jogo");
        pid_t pid = -1;
        while ((pid = achar_pid("com.dts.freefireth")) <= 0) sleep(2);
        status("jogo encontrado, anexando");

        if (!mem_abrir(pid)) { sleep(2); continue; }
        ptrace(PTRACE_ATTACH, pid, 0, 0);
        int st = 0;
        waitpid(pid, &st, 0);
        ptrace(PTRACE_DETACH, pid, 0, 0);

        while (vivo(pid) && !mem_base("libil2cpp.so")) {
            status("aguardando libil2cpp");
            sleep(1);
        }
        if (!vivo(pid)) continue;

        if (!off_carregar() || !calibrar(pid)) {
            off_limpar();
            int falhas = 0;
            while (vivo(pid) && falhas < 40) {
                if (calibrar(pid)) break;
                falhas++;
                sleep(1);
            }
            if (falhas >= 40) {
                status("calibracao falhou, tentando de novo em 10s");
                sleep(10);
                continue;
            }
        }

        while (vivo(pid)) {
            aim_tick();
            usleep(16000);
        }
        status("jogo fechou");
        off_limpar();
    }
}

int main(int argc, char **argv) {
    core_init();
    if (argc > 1 && !strcmp(argv[1], "parar")) { g_rodando = false; status("parado"); return 0; }
    return modo_auto();
}
