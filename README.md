# Sistema de Gerenciamento de Eventos Críticos — Cidade Inteligente

Sistema desenvolvido em linguagem C para monitoramento e gerenciamento de eventos críticos urbanos em tempo real, utilizando uma **Árvore AVL** como estrutura de dados central.

---

## Descrição Geral do Sistema

Cidades inteligentes precisam reagir rapidamente a ocorrências como acidentes, falhas de infraestrutura e desastres naturais. Este sistema simula o núcleo de gerenciamento desses eventos, priorizando eficiência mesmo com alto volume de dados.

Cada evento registrado possui:

| Campo | Descrição |
|---|---|
| **ID** | Identificador único gerado automaticamente (autoincremento) |
| **Tipo** | Categoria do evento (acidente, alagamento, incêndio, etc.) |
| **Severidade** | Nível de 1 (baixa) a 5 (crítica) |
| **Região** | Área da cidade onde ocorreu |
| **Status** | `Ativo` ou `Resolvido` |
| **Timestamp** | Data e hora exatas do registro |

### Por que Árvore AVL?

A chave de ordenação é o **ID do evento**. A AVL garante que inserções, remoções e buscas ocorram sempre em **O(log n)**, mantendo a árvore balanceada automaticamente através de rotações simples e duplas. Isso é fundamental para um sistema de resposta rápida, onde o volume de eventos cresce continuamente.

### Funcionalidades

**Cadastros**
- Inserção de novos eventos com ID gerado automaticamente
- Remoção de eventos — restrita a eventos com status `Resolvido`, seguida de rebalanceamento

**Consultas**
- Busca direta por ID em O(log n)
- Listagem de eventos ativos por intervalo de severidade
- Listagem de eventos por intervalo de ID (com poda de subárvores)
- Exibição de todos os eventos em ordem crescente de ID (percurso em-ordem)

**Relatórios**
- Eventos ativos de uma região específica, ordenados por ID

**Atualizações**
- Alteração de status (`Ativo` → `Resolvido`)
- Atualização de severidade de eventos ativos

**Métricas da Árvore AVL**
- Altura total da árvore
- Total de nós e quantidade de eventos ativos/resolvidos
- Fator de balanceamento médio
- Número total de rotações realizadas desde o início da execução

---

## Instruções de Compilação

### Pré-requisitos

- Compilador **GCC** instalado
- Sistema operacional **Linux**, **macOS** ou **Windows** (com MinGW ou WSL)
- Alternativamente, pode ser utilizada a IDE **Dev-C++** (já inclui o GCC via MinGW)

### Compilando

No terminal, dentro da pasta do projeto, execute:

```bash
gcc -Wall -Wextra -o eventos_cidade Gerenciamento_de_Eventos.cpp -lm
```

| Flag | Função |
|---|---|
| `-Wall -Wextra` | Ativa avisos de compilação para maior segurança |
| `-o eventos_cidade` | Define o nome do executável gerado |
| `-lm` | Liga a biblioteca matemática (usada internamente) |

### Executando

```bash
./eventos_cidade
```

No Windows (MinGW):

```bash
eventos_cidade.exe
```

### Compilando pelo Dev-C++

1. Abra o Dev-C++ e vá em **Arquivo → Novo → Projeto**
2. Selecione **Console Application** e linguagem **C**
3. Adicione o arquivo `Gerenciamento_de_Eventos.cpp` ao projeto (ou cole o código no editor)
4. Vá em **Ferramentas → Opções do Compilador** e certifique-se de que o GCC está configurado
5. Pressione **F9** (ou **Executar → Compilar & Executar**) para compilar e rodar

### Exemplo de sessão

```
================================================
   SISTEMA DE EVENTOS CRITICOS - CIDADE SMART
================================================
  1 - Cadastros
  2 - Consultas
  3 - Relatorios
  4 - Atualizacoes
  5 - Metricas da Arvore AVL
  0 - Sair
  Opcao: 5

  ===== METRICAS DA ARVORE AVL =====
  Altura total da arvore:          3
  Total de nos (eventos):          10
  Eventos ativos:                  10
  Eventos resolvidos:              0
  Fator de balanceamento medio:    0.2000
  Total de rotacoes realizadas:    2
  ==================================
```

---

## Estrutura do Código

```
eventos_cidade.c
├── Enums e Structs          — TipoEvento, StatusEvento, DataHora, Evento (nó AVL)
├── Funções Auxiliares       — altura, fator de balanceamento, impressão, timestamp
├── Rotações AVL             — rotacaoDireita, rotacaoEsquerda, rebalancear
├── Operações da AVL         — criarEvento, inserir, remover, buscarPorID
├── Consultas Avançadas      — severidade, região, intervalo de ID
├── Atualizações             — alterarStatus, atualizarSeveridade
├── Métricas                 — contarNos, contarAtivos, somaFB, exibirMetricas
└── main / Submenus          — interface textual interativa
```

---

## Requisitos Técnicos Atendidos

- [x] Linguagem C pura
- [x] Alocação dinâmica de memória (`malloc` / `free`)
- [x] Uso de `struct`, `enum` e ponteiros
- [x] Árvore AVL implementada do zero (sem bibliotecas externas)
- [x] Rotações simples e duplas com rebalanceamento automático
- [x] Remoção restrita a eventos `Resolvidos`
- [x] Liberação completa de memória ao encerrar (`liberarArvore`)
