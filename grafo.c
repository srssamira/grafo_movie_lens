#include "grafo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> // Para INT_MAX ou valores de infinito
#include <float.h>  // Para FLT_MAX

// estrutura para os nós da lista de adjacências
typedef struct NoAdj {
    int vertice_destino;
    float peso;
    struct NoAdj *prox;
} NoAdj;

// estrutura para um vértice
typedef struct Vertice {
    char *label; // label do vértice
    NoAdj *lista_adj; // lista de adjacências com encadeamento dinâmico
    
    // campos auxiliares para algoritmos (dijkstra, BFS/DFS para componentes)
    float distancia;    // distância para o nó inicial (dijkstra)
    int predecessor;    // predecessor no caminho mais curto (dijkstra)
    int visitado;       // flag para controle de visitação (BFS/DFS)
} Vertice;

// estrutura principal do grafo (TAD opaco)
struct grafo {
    Vertice *vertices; // array de vértices 
    int num_vertices_atual; // num de vértices atualmente no grafo
    int max_vertices_alocados; // capacidade atual do array de vértices
    int is_digrafo; // 1 se for dígrafo, 0 caso contrário
    int is_ponderado; // 1 se for ponderado, 0 caso contrário
};

// ===============================================
// funcoes auxiliares (privadas)
// ===============================================

// funcoes para fila de Prioridade (Min-Heap) para dijkstra
// uma implementação mais robusta usaria um heap binário.
// aqui, uma implementação simplificada (array ordenado) para demonstração.
// para N=1000+, uma min-heap é essencial para desempenho.

typedef struct {
    ItemFila *itens;
    int capacidade;
    int tamanho;
} FilaPrioridade;

FilaPrioridade* criaFilaPrioridade(int capacidade) {
    FilaPrioridade *fp = (FilaPrioridade*) malloc(sizeof(FilaPrioridade));
    if (!fp) return NULL;
    fp->itens = (ItemFila*) malloc(sizeof(ItemFila) * capacidade);
    if (!fp->itens) { free(fp); return NULL; }
    fp->capacidade = capacidade;
    fp->tamanho = 0;
    return fp;
}

void liberaFilaPrioridade(FilaPrioridade *fp) {
    if (fp) {
        free(fp->itens);
        free(fp);
    }
}

void insereFilaPrioridade(FilaPrioridade *fp, ItemFila item) {
    if (fp->tamanho == fp->capacidade) {
        // realocacao se necessário, ou erro
        return;
    }
    fp->itens[fp->tamanho++] = item;

    // simplesmente insere e reordena (ineficiente para N grande)
    // em uma min-heap, seria um "heapify up"
    for (int i = fp->tamanho - 1; i > 0 && fp->itens[i].distancia < fp->itens[(i-1)/2].distancia; i = (i-1)/2) {
        ItemFila temp = fp->itens[i];
        fp->itens[i] = fp->itens[(i-1)/2];
        fp->itens[(i-1)/2] = temp;
    }
}

ItemFila extraiMinFilaPrioridade(FilaPrioridade *fp) {
    ItemFila min_item = {-1, FLT_MAX}; // valor sentinela
    if (fp->tamanho == 0) return min_item;

    min_item = fp->itens[0];
    fp->itens[0] = fp->itens[--fp->tamanho];

    // heapify down
    int idx = 0;
    while (1) {
        int left_child = 2 * idx + 1;
        int right_child = 2 * idx + 2;
        int smallest = idx;

        if (left_child < fp->tamanho && fp->itens[left_child].distancia < fp->itens[smallest].distancia) {
            smallest = left_child;
        }
        if (right_child < fp->tamanho && fp->itens[right_child].distancia < fp->itens[smallest].distancia) {
            smallest = right_child;
        }

        if (smallest != idx) {
            ItemFila temp = fp->itens[idx];
            fp->itens[idx] = fp->itens[smallest];
            fp->itens[smallest] = temp;
            idx = smallest;
        } else {
            break;
        }
    }
    return min_item;
}

int filaPrioridadeVazia(FilaPrioridade *fp) {
    return fp->tamanho == 0;
}