/*
 * Sistema de Gerenciamento de Eventos Criticos - Cidade Inteligente
 * Implementacao com Arvore AVL
 *
 * Estrutura central: cada no da AVL representa um evento critico urbano.
 * Chave primaria de ordenacao: ID do evento.
 * Balanceamento automatico garantido por rotacoes simples e duplas.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <locale.h>

/* =========================================================
 * ENUMS E STRUCTS
 * ========================================================= */

/* Tipos de eventos monitorados pela cidade */
typedef enum {
    ACIDENTE_TRANSITO    = 1,
    FALHA_SEMAFORO       = 2,
    INTERRUPCAO_ENERGIA  = 3,
    ALAGAMENTO           = 4,
    INCENDIO             = 5
} TipoEvento;

/* Status possivel de cada evento */
typedef enum {
    ATIVO     = 0,
    RESOLVIDO = 1
} StatusEvento;

/* Estrutura para armazenar data e hora do registro */
typedef struct {
    int dia, mes, ano;
    int hora, minuto, segundo;
} DataHora;

/*
 * No principal da AVL.
 * ID e a chave de ordenacao.
 * Altura e usada para calcular o fator de balanceamento.
 */
typedef struct Evento {
    int           ID;
    TipoEvento    Tipo;
    int           Severidade;   /* 1 (baixa) a 5 (critica) */
    DataHora      Timestamp;
    char          Regiao[50];
    StatusEvento  Status;
    int           Altura;       /* altura do no na AVL */
    struct Evento *esq;
    struct Evento *dir;
} Evento;

/* Contador global de rotacoes realizadas durante a execucao */
static int totalRotacoes = 0;

/* Gerador de IDs unicos: incrementado a cada novo evento criado */
static int proximoID = 1;

/* =========================================================
 * FUNCOES AUXILIARES GERAIS
 * ========================================================= */

/* Retorna o maior entre dois inteiros */
static int maxi(int a, int b) {
    return (a > b) ? a : b;
}

/* Retorna a altura de um no; no nulo tem altura -1 */
static int alturaNo(Evento *no) {
    if (no == NULL) return -1;
    return no->Altura;
}

/* Recalcula a altura de um no com base nos filhos */
static void atualizarAltura(Evento *no) {
    if (no != NULL)
        no->Altura = 1 + maxi(alturaNo(no->esq), alturaNo(no->dir));
}

/* Calcula o fator de balanceamento: altura(esq) - altura(dir) */
static int fatorBalanceamento(Evento *no) {
    if (no == NULL) return 0;
    return alturaNo(no->esq) - alturaNo(no->dir);
}

/* Converte o enum TipoEvento para string legivel */
static const char *tipoStr(TipoEvento tipo) {
    switch (tipo) {
        case ACIDENTE_TRANSITO:   return "Acidente de Transito";
        case FALHA_SEMAFORO:      return "Falha em Semaforo";
        case INTERRUPCAO_ENERGIA: return "Interrupcao de Energia";
        case ALAGAMENTO:          return "Alagamento";
        case INCENDIO:            return "Incendio";
        default:                  return "Desconhecido";
    }
}

/* Converte o enum StatusEvento para string legivel */
static const char *statusStr(StatusEvento s) {
    return (s == ATIVO) ? "Ativo" : "Resolvido";
}

/* Captura o timestamp atual do sistema operacional */
static DataHora timestampAtual(void) {
    time_t agora = time(NULL);
    struct tm *t  = localtime(&agora);
    DataHora dh;
    dh.dia     = t->tm_mday;
    dh.mes     = t->tm_mon + 1;
    dh.ano     = t->tm_year + 1900;
    dh.hora    = t->tm_hour;
    dh.minuto  = t->tm_min;
    dh.segundo = t->tm_sec;
    return dh;
}

/* Imprime os dados completos de um evento na tela */
static void imprimirEvento(Evento *ev) {
    if (ev == NULL) {
        printf("  Evento nao encontrado.\n");
        return;
    }
    printf("  +----------------------------------+\n");
    printf("  | ID:         %-20d |\n", ev->ID);
    printf("  | Tipo:       %-20s |\n", tipoStr(ev->Tipo));
    printf("  | Severidade: %-20d |\n", ev->Severidade);
    printf("  | Regiao:     %-20s |\n", ev->Regiao);
    printf("  | Status:     %-20s |\n", statusStr(ev->Status));
    printf("  | Registro:   %02d/%02d/%04d %02d:%02d:%02d    |\n",
           ev->Timestamp.dia, ev->Timestamp.mes, ev->Timestamp.ano,
           ev->Timestamp.hora, ev->Timestamp.minuto, ev->Timestamp.segundo);
    printf("  +----------------------------------+\n");
}

/* Descarta caracteres restantes no buffer de entrada */
static void limparBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* =========================================================
 * ROTACOES AVL
 * ========================================================= */

/*
 * Rotacao simples a direita (caso Esquerda-Esquerda).
 * Usado quando o no esta desbalanceado pela subarvore esquerda
 * e o filho esquerdo tambem pesa para a esquerda.
 */
static Evento *rotacaoDireita(Evento *y) {
    Evento *x  = y->esq;
    Evento *T2 = x->dir;

    /* realiza a rotacao */
    x->dir = y;
    y->esq = T2;

    /* atualiza alturas: primeiro y (desceu), depois x (subiu) */
    atualizarAltura(y);
    atualizarAltura(x);

    totalRotacoes++;
    return x; /* nova raiz do subconjunto */
}

/*
 * Rotacao simples a esquerda (caso Direita-Direita).
 * Simetrico ao anterior.
 */
static Evento *rotacaoEsquerda(Evento *x) {
    Evento *y  = x->dir;
    Evento *T2 = y->esq;

    y->esq = x;
    x->dir = T2;

    atualizarAltura(x);
    atualizarAltura(y);

    totalRotacoes++;
    return y;
}

/*
 * Rebalanceia um no apos insercao ou remocao.
 * Verifica o fator de balanceamento e aplica a rotacao correta:
 *   FB > 1  -> subarvore esquerda pesada (rotacao direita ou dupla ED)
 *   FB < -1 -> subarvore direita pesada  (rotacao esquerda ou dupla DE)
 */
static Evento *rebalancear(Evento *no) {
    atualizarAltura(no);
    int fb = fatorBalanceamento(no);

    /* Caso Esquerda-Esquerda: rotacao simples a direita */
    if (fb > 1 && fatorBalanceamento(no->esq) >= 0)
        return rotacaoDireita(no);

    /* Caso Esquerda-Direita: rotacao dupla (esq->esquerda, depois direita) */
    if (fb > 1 && fatorBalanceamento(no->esq) < 0) {
        no->esq = rotacaoEsquerda(no->esq);
        return rotacaoDireita(no);
    }

    /* Caso Direita-Direita: rotacao simples a esquerda */
    if (fb < -1 && fatorBalanceamento(no->dir) <= 0)
        return rotacaoEsquerda(no);

    /* Caso Direita-Esquerda: rotacao dupla (dir->direita, depois esquerda) */
    if (fb < -1 && fatorBalanceamento(no->dir) > 0) {
        no->dir = rotacaoDireita(no->dir);
        return rotacaoEsquerda(no);
    }

    /* No ja esta balanceado */
    return no;
}

/* =========================================================
 * OPERACOES PRINCIPAIS DA AVL
 * ========================================================= */

/*
 * Aloca e inicializa um novo no de evento.
 * Retorna NULL se a alocacao falhar.
 */
Evento *criarEvento(TipoEvento tipo, int severidade, const char *regiao) {
    Evento *novo = (Evento *) malloc(sizeof(Evento));
    if (novo == NULL) {
        printf("Erro: falha ao alocar memoria para o evento.\n");
        return NULL;
    }
    novo->ID         = proximoID++; /* ID unico gerado automaticamente */
    novo->Tipo       = tipo;
    novo->Severidade = severidade;
    strncpy(novo->Regiao, regiao, 49);
    novo->Regiao[49] = '\0';
    novo->Status     = ATIVO;
    novo->Timestamp  = timestampAtual();
    novo->Altura     = 0;
    novo->esq        = NULL;
    novo->dir        = NULL;
    return novo;
}

/*
 * Insere um evento na AVL.
 * IDs duplicados sao rejeitados.
 * Apos a insercao, o rebalanceamento e aplicado no caminho de volta.
 */
Evento *inserir(Evento *arv, Evento *novo) {
    /* caso base: arvore vazia, novo vira raiz deste subconjunto */
    if (arv == NULL) return novo;

    if (novo->ID < arv->ID) {
        arv->esq = inserir(arv->esq, novo);
    } else if (novo->ID > arv->ID) {
        arv->dir = inserir(arv->dir, novo);
    } else {
        /* ID duplicado: nao insere e libera o no alocado */
        printf("Aviso: evento com ID %d ja existe na arvore.\n", novo->ID);
        free(novo);
        return arv;
    }

    /* rebalanceia no caminho de volta */
    return rebalancear(arv);
}

/*
 * Retorna o no com o menor ID a partir de um dado no.
 * Utilizado para encontrar o sucessor in-ordem durante a remocao.
 */
static Evento *menorNo(Evento *no) {
    while (no->esq != NULL)
        no = no->esq;
    return no;
}

/*
 * Remove um evento da AVL pelo ID.
 * Restricao: apenas eventos com status RESOLVIDO podem ser removidos.
 * O parametro 'removido' indica (por referencia) se a remocao ocorreu.
 * Rebalanceamento e aplicado apos cada remocao.
 */
Evento *remover(Evento *arv, int id, int *removido) {
    if (arv == NULL) {
        /* chegou em no nulo sem encontrar o ID */
        printf("Evento de ID %d nao encontrado.\n", id);
        *removido = 0;
        return NULL;
    }

    if (id < arv->ID) {
        arv->esq = remover(arv->esq, id, removido);
    } else if (id > arv->ID) {
        arv->dir = remover(arv->dir, id, removido);
    } else {
        /* encontrou o no com o ID buscado */
        if (arv->Status == ATIVO) {
            printf("Erro: evento %d esta Ativo. Resolva-o antes de remover.\n", id);
            *removido = 0;
            return arv; /* arvore permanece inalterada */
        }

        *removido = 1;

        /* No com nenhum ou apenas um filho */
        if (arv->esq == NULL || arv->dir == NULL) {
            Evento *filho = (arv->esq != NULL) ? arv->esq : arv->dir;
            free(arv);
            return filho; /* pode ser NULL se era folha */
        }

        /*
         * No com dois filhos:
         * Copia os dados do sucessor in-ordem (menor da subarvore direita)
         * e remove o sucessor recursivamente.
         */
        Evento *suc = menorNo(arv->dir);
        arv->ID         = suc->ID;
        arv->Tipo       = suc->Tipo;
        arv->Severidade = suc->Severidade;
        arv->Status     = suc->Status;
        arv->Timestamp  = suc->Timestamp;
        strncpy(arv->Regiao, suc->Regiao, 49);

        int aux = 0;
        arv->dir = remover(arv->dir, suc->ID, &aux);
    }

    return rebalancear(arv);
}

/*
 * Busca um evento pelo ID usando a propriedade da BST.
 * Complexidade O(log n) por conta do balanceamento AVL.
 */
Evento *buscarPorID(Evento *arv, int id) {
    if (arv == NULL)        return NULL;
    if (id == arv->ID)      return arv;
    if (id < arv->ID)       return buscarPorID(arv->esq, id);
    return buscarPorID(arv->dir, id);
}

/* =========================================================
 * CONSULTAS AVANCADAS
 * ========================================================= */

/*
 * Lista eventos ATIVOS cuja severidade esteja no intervalo [minSev, maxSev].
 * Percorre a arvore em-ordem (esq -> raiz -> dir) para exibir em ordem de ID.
 * Nao ha poda eficiente pois severidade nao e a chave; percorre tudo.
 */
static int listarPorSeveridade(Evento *arv, int minSev, int maxSev) {
    if (arv == NULL) return 0;
    int count = 0;
    count += listarPorSeveridade(arv->esq, minSev, maxSev);
    if (arv->Status == ATIVO &&
        arv->Severidade >= minSev &&
        arv->Severidade <= maxSev) {
        imprimirEvento(arv);
        count++;
    }
    count += listarPorSeveridade(arv->dir, minSev, maxSev);
    return count;
}

/*
 * Exibe todos os eventos ATIVOS de uma regiao especifica.
 * Percurso em-ordem garante ordem crescente de ID.
 */
static int relatorioPorRegiao(Evento *arv, const char *regiao) {
    if (arv == NULL) return 0;
    int count = 0;
    count += relatorioPorRegiao(arv->esq, regiao);
    if (arv->Status == ATIVO && strcmp(arv->Regiao, regiao) == 0) {
        imprimirEvento(arv);
        count++;
    }
    count += relatorioPorRegiao(arv->dir, regiao);
    return count;
}

/*
 * Lista eventos cujo ID esteja no intervalo [idMin, idMax].
 * Aplica poda: so desce para a esquerda se pode existir ID >= idMin,
 * e so desce para a direita se pode existir ID <= idMax.
 */
static int buscarIntervaloID(Evento *arv, int idMin, int idMax) {
    if (arv == NULL) return 0;
    int count = 0;
    if (arv->ID > idMin)
        count += buscarIntervaloID(arv->esq, idMin, idMax);
    if (arv->ID >= idMin && arv->ID <= idMax) {
        imprimirEvento(arv);
        count++;
    }
    if (arv->ID < idMax)
        count += buscarIntervaloID(arv->dir, idMin, idMax);
    return count;
}

/* Exibe todos os eventos em ordem crescente de ID */
static void exibirTodos(Evento *arv) {
    if (arv == NULL) return;
    exibirTodos(arv->esq);
    imprimirEvento(arv);
    exibirTodos(arv->dir);
}

/* =========================================================
 * OPERACOES DE ATUALIZACAO
 * ========================================================= */

/*
 * Altera o status de um evento de ATIVO para RESOLVIDO.
 * Nao quebra a consistencia da arvore pois o ID permanece igual.
 */
static void alterarStatus(Evento *arv, int id) {
    Evento *ev = buscarPorID(arv, id);
    if (ev == NULL) {
        printf("Evento %d nao encontrado.\n", id);
        return;
    }
    if (ev->Status == RESOLVIDO) {
        printf("Evento %d ja esta marcado como Resolvido.\n", id);
        return;
    }
    ev->Status = RESOLVIDO;
    printf("Evento %d marcado como Resolvido com sucesso.\n", id);
}

/*
 * Atualiza a severidade de um evento ATIVO.
 * Rejeitado se o evento estiver Resolvido ou a severidade for invalida.
 */
static void atualizarSeveridade(Evento *arv, int id, int novaSev) {
    if (novaSev < 1 || novaSev > 5) {
        printf("Severidade invalida! Informe um valor entre 1 e 5.\n");
        return;
    }
    Evento *ev = buscarPorID(arv, id);
    if (ev == NULL) {
        printf("Evento %d nao encontrado.\n", id);
        return;
    }
    if (ev->Status == RESOLVIDO) {
        printf("Evento %d esta Resolvido. Nao e possivel alterar a severidade.\n", id);
        return;
    }
    ev->Severidade = novaSev;
    printf("Severidade do evento %d atualizada para %d.\n", id, novaSev);
}

/* =========================================================
 * METRICAS DA ARVORE
 * ========================================================= */

/* Conta o total de nos na arvore */
static int contarNos(Evento *arv) {
    if (arv == NULL) return 0;
    return 1 + contarNos(arv->esq) + contarNos(arv->dir);
}

/* Conta quantos eventos estao com status ATIVO */
static int contarAtivos(Evento *arv) {
    if (arv == NULL) return 0;
    int c = (arv->Status == ATIVO) ? 1 : 0;
    return c + contarAtivos(arv->esq) + contarAtivos(arv->dir);
}

/* Soma os fatores de balanceamento absolutos de todos os nos */
static double somaFB(Evento *arv) {
    if (arv == NULL) return 0.0;
    int fb = fatorBalanceamento(arv);
    if (fb < 0) fb = -fb; /* abs manual */
    return (double)fb + somaFB(arv->esq) + somaFB(arv->dir);
}

/* Exibe o relatorio completo de metricas da arvore AVL */
static void exibirMetricas(Evento *arv) {
    int total   = contarNos(arv);
    int ativos  = contarAtivos(arv);
    int altura  = alturaNo(arv);
    double fbMed = (total > 0) ? somaFB(arv) / total : 0.0;

    printf("\n  ===== METRICAS DA ARVORE AVL =====\n");
    printf("  Altura total da arvore:          %d\n",    altura);
    printf("  Total de nos (eventos):          %d\n",    total);
    printf("  Eventos ativos:                  %d\n",    ativos);
    printf("  Eventos resolvidos:              %d\n",    total - ativos);
    printf("  Fator de balanceamento medio:    %.4f\n",  fbMed);
    printf("  Total de rotacoes realizadas:    %d\n",    totalRotacoes);
    printf("  ==================================\n");
}

/* =========================================================
 * LIBERACAO DE MEMORIA
 * ========================================================= */

/* Percurso pos-ordem para liberar todos os nos da arvore */
static void liberarArvore(Evento *arv) {
    if (arv == NULL) return;
    liberarArvore(arv->esq);
    liberarArvore(arv->dir);
    free(arv);
}

/* =========================================================
 * SUBMENUS
 * ========================================================= */

static Evento *submenuCadastros(Evento *arv) {
    int sub;
    printf("\n  -- CADASTROS --\n");
    printf("  1 - Inserir novo evento\n");
    printf("  2 - Remover evento (somente Resolvido)\n");
    printf("  Opcao: ");
    if (scanf("%d", &sub) != 1) { limparBuffer(); return arv; }
    limparBuffer();

    if (sub == 1) {
        int tipo, sev;
        char regiao[50];

        printf("  Tipo:\n");
        printf("    1 - Acidente de Transito\n");
        printf("    2 - Falha em Semaforo\n");
        printf("    3 - Interrupcao de Energia\n");
        printf("    4 - Alagamento\n");
        printf("    5 - Incendio\n");
        printf("  Escolha: ");
        if (scanf("%d", &tipo) != 1 || tipo < 1 || tipo > 5) {
            printf("  Tipo invalido.\n");
            limparBuffer();
            return arv;
        }
        limparBuffer();

        printf("  Severidade (1 a 5): ");
        if (scanf("%d", &sev) != 1 || sev < 1 || sev > 5) {
            printf("  Severidade invalida.\n");
            limparBuffer();
            return arv;
        }
        limparBuffer();

        printf("  Regiao da cidade: ");
        if (scanf(" %49[^\n]", regiao) != 1) {
            limparBuffer();
            return arv;
        }

        Evento *novo = criarEvento((TipoEvento)tipo, sev, regiao);
        if (novo != NULL) {
            int idGerado = novo->ID; /* guarda antes de inserir */
            arv = inserir(arv, novo);
            printf("  Evento inserido com sucesso. ID gerado: %d\n", idGerado);
        }

    } else if (sub == 2) {
        int id, removido = 0;
        printf("  ID do evento a remover: ");
        if (scanf("%d", &id) != 1) { limparBuffer(); return arv; }
        limparBuffer();
        arv = remover(arv, id, &removido);
        if (removido)
            printf("  Evento %d removido com sucesso.\n", id);

    } else {
        printf("  Opcao invalida.\n");
    }
    return arv;
}

static void submenuConsultas(Evento *arv) {
    int sub;
    printf("\n  -- CONSULTAS --\n");
    printf("  1 - Buscar evento por ID\n");
    printf("  2 - Listar ativos por intervalo de severidade\n");
    printf("  3 - Buscar por intervalo de ID\n");
    printf("  4 - Exibir todos os eventos\n");
    printf("  Opcao: ");
    if (scanf("%d", &sub) != 1) { limparBuffer(); return; }
    limparBuffer();

    if (sub == 1) {
        int id;
        printf("  ID: ");
        if (scanf("%d", &id) != 1) { limparBuffer(); return; }
        limparBuffer();
        imprimirEvento(buscarPorID(arv, id));

    } else if (sub == 2) {
        int minS, maxS;
        printf("  Severidade minima: ");
        if (scanf("%d", &minS) != 1) { limparBuffer(); return; }
        printf("  Severidade maxima: ");
        if (scanf("%d", &maxS) != 1) { limparBuffer(); return; }
        limparBuffer();
        if (minS > maxS || minS < 1 || maxS > 5) {
            printf("  Intervalo invalido. Use valores entre 1 e 5.\n");
            return;
        }
        printf("\n  Eventos ativos com severidade entre %d e %d:\n", minS, maxS);
        int found = listarPorSeveridade(arv, minS, maxS);
        if (found == 0) printf("  Nenhum evento encontrado.\n");

    } else if (sub == 3) {
        int idMin, idMax;
        printf("  ID minimo: ");
        if (scanf("%d", &idMin) != 1) { limparBuffer(); return; }
        printf("  ID maximo: ");
        if (scanf("%d", &idMax) != 1) { limparBuffer(); return; }
        limparBuffer();
        if (idMin > idMax) {
            printf("  Intervalo invalido.\n");
            return;
        }
        printf("\n  Eventos com ID entre %d e %d:\n", idMin, idMax);
        int found = buscarIntervaloID(arv, idMin, idMax);
        if (found == 0) printf("  Nenhum evento encontrado.\n");

    } else if (sub == 4) {
        printf("\n  Todos os eventos (ordem crescente de ID):\n");
        if (arv == NULL) printf("  Arvore vazia.\n");
        else exibirTodos(arv);

    } else {
        printf("  Opcao invalida.\n");
    }
}

static void submenuRelatorios(Evento *arv) {
    int sub;
    printf("\n  -- RELATORIOS --\n");
    printf("  1 - Relatorio de eventos ativos por regiao\n");
    printf("  Opcao: ");
    if (scanf("%d", &sub) != 1) { limparBuffer(); return; }
    limparBuffer();

    if (sub == 1) {
        char regiao[50];
        printf("  Nome da regiao: ");
        if (scanf(" %49[^\n]", regiao) != 1) { limparBuffer(); return; }
        printf("\n  Eventos ativos na regiao '%s':\n", regiao);
        int found = relatorioPorRegiao(arv, regiao);
        if (found == 0) printf("  Nenhum evento ativo nessa regiao.\n");

    } else {
        printf("  Opcao invalida.\n");
    }
}

static void submenuAtualizacoes(Evento *arv) {
    int sub;
    printf("\n  -- ATUALIZACOES --\n");
    printf("  1 - Marcar evento como Resolvido\n");
    printf("  2 - Atualizar severidade de evento ativo\n");
    printf("  Opcao: ");
    if (scanf("%d", &sub) != 1) { limparBuffer(); return; }
    limparBuffer();

    if (sub == 1) {
        int id;
        printf("  ID do evento: ");
        if (scanf("%d", &id) != 1) { limparBuffer(); return; }
        limparBuffer();
        alterarStatus(arv, id);

    } else if (sub == 2) {
        int id, sev;
        printf("  ID do evento: ");
        if (scanf("%d", &id) != 1) { limparBuffer(); return; }
        printf("  Nova severidade (1 a 5): ");
        if (scanf("%d", &sev) != 1) { limparBuffer(); return; }
        limparBuffer();
        atualizarSeveridade(arv, id, sev);

    } else {
        printf("  Opcao invalida.\n");
    }
}

/* =========================================================
 * FUNCAO PRINCIPAL
 * ========================================================= */

int main(void) {
    setlocale(LC_ALL, "");

    Evento *arv = NULL;

    /*
     * Eventos iniciais inseridos fora de ordem para demonstrar
     * que a AVL rebalanceia automaticamente.
     */
    arv = inserir(arv, criarEvento(ACIDENTE_TRANSITO,    3, "Zona Norte"));
    arv = inserir(arv, criarEvento(ALAGAMENTO,           5, "Zona Sul"));
    arv = inserir(arv, criarEvento(INCENDIO,             4, "Centro"));
    arv = inserir(arv, criarEvento(FALHA_SEMAFORO,       2, "Zona Leste"));
    arv = inserir(arv, criarEvento(INTERRUPCAO_ENERGIA,  1, "Zona Oeste"));
    arv = inserir(arv, criarEvento(ACIDENTE_TRANSITO,    5, "Centro"));
    arv = inserir(arv, criarEvento(ALAGAMENTO,           3, "Zona Norte"));
    arv = inserir(arv, criarEvento(INCENDIO,             4, "Zona Sul"));
    arv = inserir(arv, criarEvento(FALHA_SEMAFORO,       2, "Zona Leste"));
    arv = inserir(arv, criarEvento(INTERRUPCAO_ENERGIA,  5, "Centro"));

    int opcao;
    while (1) {
        printf("\n================================================\n");
        printf("   SISTEMA DE EVENTOS CRITICOS - CIDADE SMART\n");
        printf("================================================\n");
        printf("  1 - Cadastros\n");
        printf("  2 - Consultas\n");
        printf("  3 - Relatorios\n");
        printf("  4 - Atualizacoes\n");
        printf("  5 - Metricas da Arvore AVL\n");
        printf("  0 - Sair\n");
        printf("  Opcao: ");

        if (scanf("%d", &opcao) != 1) {
            limparBuffer();
            printf("  Entrada invalida. Digite um numero.\n");
            continue;
        }
        limparBuffer();

        if (opcao == 0) break;

        switch (opcao) {
            case 1: arv = submenuCadastros(arv);  break;
            case 2: submenuConsultas(arv);        break;
            case 3: submenuRelatorios(arv);        break;
            case 4: submenuAtualizacoes(arv);      break;
            case 5: exibirMetricas(arv);           break;
            default: printf("  Opcao invalida. Tente novamente.\n"); break;
        }
    }

    liberarArvore(arv);
    printf("\nSistema encerrado. Memoria liberada.\n");
    return 0;
}
