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

// ===============================================
// Implementação das Funções do TAD Grafo
// ===============================================

Grafo* criaGrafo(int max_vertices, int is_digrafo, int is_ponderado) {
    Grafo *novo_grafo = (Grafo*) malloc(sizeof(Grafo));
    if (novo_grafo == NULL) {
        perror("Erro ao alocar Grafo");
        return NULL;
    }

    novo_grafo->vertices = (Vertice*) calloc(max_vertices, sizeof(Vertice));
    if (novo_grafo->vertices == NULL) {
        perror("Erro ao alocar array de vertices");
        free(novo_grafo);
        return NULL;
    }
    
    for (int i = 0; i < max_vertices; i++) {
        novo_grafo->vertices[i].label = NULL;
        novo_grafo->vertices[i].lista_adj = NULL;
    }

    novo_grafo->num_vertices_atual = 0;
    novo_grafo->max_vertices_alocados = max_vertices;
    novo_grafo->is_digrafo = is_digrafo;
    novo_grafo->is_ponderado = is_ponderado;
    return novo_grafo;
}

int insereAresta(Grafo *G, int u, int v, float w) { 
    if (G == NULL || u < 0 || u >= G->num_vertices_atual || v < 0 || v >= G->num_vertices_atual) {
        return 0;
    }

    // verifica se a aresta já existe para evitar duplicatas
    NoAdj *temp = G->vertices[u].lista_adj;
    while (temp != NULL) {
        if (temp->vertice_destino == v) {
            return -1; // aresta já existe
        }
        temp = temp->prox;
    }

    // insere a aresta u -> v
    NoAdj *novo_no_u = (NoAdj*) malloc(sizeof(NoAdj));
    if (novo_no_u == NULL) return 0;
    novo_no_u->vertice_destino = v;
    novo_no_u->peso = w;
    novo_no_u->prox = G->vertices[u].lista_adj;
    G->vertices[u].lista_adj = novo_no_u;

    // se o grafo não é dígrafo, insere também v -> u
    if (!G->is_digrafo) {
        NoAdj *novo_no_v = (NoAdj*) malloc(sizeof(NoAdj));
        if (novo_no_v == NULL) {
            // se falhar aqui, idealmente desfazer a inserção u->v
            // por simplicidade, assumindo que malloc não falha consecutivamente
            return 0;
        }
        novo_no_v->vertice_destino = u;
        novo_no_v->peso = w;
        novo_no_v->prox = G->vertices[v].lista_adj;
        G->vertices[v].lista_adj = novo_no_v;
    }
    return 1; // inserção bem-sucedida 

}

float menorCaminhoMedio(Grafo *G) { 
    if (G == NULL || G->num_vertices_atual == 0) return -1.0;

    double soma_distancias_totais = 0.0;
    long long num_pares_validos = 0;

    for (int i = 0; i < G->num_vertices_atual; i++) {
        dijkstra(G, i); // calcula os menores caminhos a partir do vértice i [cite: 30]
        for (int j = 0; j < G->num_vertices_atual; j++) {
            if (i != j && G->vertices[j].distancia != FLT_MAX) { // se j é alcançável de i
                soma_distancias_totais += G->vertices[j].distancia;
                num_pares_validos++;
            }
        }
    }

    if (num_pares_validos == 0) return 0.0; // nenhuma conexão entre pares

    return (float)(soma_distancias_totais / num_pares_validos); 
}

// função para carregar o grafo do arquivo .edges
int carregaGrafoDeArquivo(Grafo *G, const char *nome_arquivo) { // [cite: 49]
    FILE *file = fopen(nome_arquivo, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo do grafo");
        return 0;
    }

    // para o formato .edges, geralmente é apenas lista de pares de vértices.
    // o MovieLens user2tags pode ter IDs numéricos grandes.
    // precisamos de um mapeamento de ID externo (do arquivo) para ID interno (0...N-1).
    // usaremos um array dinâmico simples para mapeamento por enquanto, mas um hashmap seria melhor.
    typedef struct {
        char *id_str; // ID do vértice como string (ou numérico se for só int)
        int internal_id; // ID interno no seu array de vértices
    } VerticeMap;

    VerticeMap *mapa_vertices = NULL;
    int num_mapeados = 0;
    int capacidade_mapa = 100; // capacidade inicial para o mapa

    mapa_vertices = (VerticeMap*) malloc(capacidade_mapa * sizeof(VerticeMap));
    if (!mapa_vertices) {
        fprintf(stderr, "Erro de alocação para mapa_vertices.\n");
        fclose(file);
        return 0;
    }

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (line[0] == '#' || line[0] == '%') { // ignora linhas de comentário
            continue;
        }

        int u_id, v_id; // IDs do arquivo
        // assumindo que os IDs são inteiros
        if (sscanf(line, "%d %d", &u_id, &v_id) == 2) {
            // converte IDs para strings para usar como labels
            char u_label[32], v_label[32];
            sprintf(u_label, "%d", u_id);
            sprintf(v_label, "%d", v_id);

            int u_internal_id = -1, v_internal_id = -1;

            // mapeia o ID externo para o ID interno
            // esta busca linear é ineficiente para muitos vértices, mas funciona.
            // uma hash table (e.g. `uthash` ou implementação própria) seria ideal.
            for (int i = 0; i < num_mapeados; i++) {
                if (strcmp(mapa_vertices[i].id_str, u_label) == 0) {
                    u_internal_id = mapa_vertices[i].internal_id;
                }
                if (strcmp(mapa_vertices[i].id_str, v_label) == 0) {
                    v_internal_id = mapa_vertices[i].internal_id;
                }
                if (u_internal_id != -1 && v_internal_id != -1) break; // encontrou ambos
            }

            // insere vértice se não existir e mapeia
            if (u_internal_id == -1) {
                if (insereVertice(G, u_label) == 1) { // 1 indica sucesso na inserção
                    u_internal_id = G->num_vertices_atual - 1; // novo ID interno
                    if (num_mapeados == capacidade_mapa) {
                        capacidade_mapa += 100; // aumenta capacidade do mapa
                        mapa_vertices = (VerticeMap*) realloc(mapa_vertices, capacidade_mapa * sizeof(VerticeMap));
                        if (!mapa_vertices) {
                            fprintf(stderr, "Erro de realocação para mapa_vertices.\n");
                            fclose(file);
                            return 0;
                        }
                    }
                    mapa_vertices[num_mapeados].id_str = strdup(u_label);
                    mapa_vertices[num_mapeados].internal_id = u_internal_id;
                    num_mapeados++;
                } else {
                    fprintf(stderr, "Erro ao inserir vértice %s.\n", u_label);
                    continue;
                }
            }

            if (v_internal_id == -1) {
                if (insereVertice(G, v_label) == 1) {
                    v_internal_id = G->num_vertices_atual - 1;
                     if (num_mapeados == capacidade_mapa) {
                        capacidade_mapa += 100; // aumenta capacidade do mapa
                        mapa_vertices = (VerticeMap*) realloc(mapa_vertices, capacidade_mapa * sizeof(VerticeMap));
                        if (!mapa_vertices) {
                            fprintf(stderr, "Erro de realocação para mapa_vertices.\n");
                            fclose(file);
                            return 0;
                        }
                    }
                    mapa_vertices[num_mapeados].id_str = strdup(v_label);
                    mapa_vertices[num_mapeados].internal_id = v_internal_id;
                    num_mapeados++;
                } else {
                    fprintf(stderr, "Erro ao inserir vértice %s.\n", v_label);
                    continue;
                }
            }
            
            // finalmente, insere a aresta com peso 1 (não ponderado)
            // se o arquivo tiver 3 colunas (u v w), ajuste para ler o peso
            insereAresta(G, u_internal_id, v_internal_id, 1.0);
        }
    }

    // libera a memória do mapa de vértices
    for (int i = 0; i < num_mapeados; i++) {
        free(mapa_vertices[i].id_str);
    }
    free(mapa_vertices);
    
    fclose(file);
    return 1;
}



