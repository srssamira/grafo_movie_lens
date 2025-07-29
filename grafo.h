#ifndef GRAFO_H
#define GRAFO_H

typedef struct grafo Grafo;

// estrutura p um item na fila de prioridade (usada no Dijkstra)
typedef struct {
    int vertice;
    float distancia;
} ItemFila;

// funcoes de alocação e desalocação do grafo
Grafo* criaGrafo(int max_vertices, int is_digrafo, int is_ponderado);
void liberaGrafo(Grafo *G);

// funcoes de manipulacao de vertices e arestas

// insereVertice: insere um vértice no grafo. aumenta o array de vértices em 50% se estiver cheio.
// retorna 1 em caso de sucesso, 0 caso contrário.
int insereVertice(Grafo *G, char *label);

// insereAresta: insere a aresta (u, v) no grafo com peso w.
// se não for dígrafo, insere também a aresta (v, u) internamente, sem recursão.
// retorna 1 em caso de sucesso, 0 caso contrário.
int insereAresta(Grafo *G, int u, int v, float w);

// removeAresta: remove a aresta (u, v) do grafo, caso exista].
// se não for dígrafo, trata internamente sem recursão.
// retorna 1 em caso de sucesso, 0 caso contrário.
int removeAresta(Grafo *G, int u, int v);

// funcoes de cálculo de propriedades do grafo

// grau: retorna o grau do vértice v. retorna -1 se o vértice não existir (ajustado da especificação).
int grau(Grafo *G, int v);

// grauMedio: retorna o grau médio do grafo. Retorna -1 se o grafo estiver vazio.
float grauMedio(Grafo *G);

// grauMax: retorna o maior grau de todos os vértices e armazena o índice do vértice em *v.
int grauMax(Grafo *G, int *v);

// funcoes adicionais obrigatórias

// menorCaminhoMedio: calcula o menor caminho médio do grafo.
// utiliza algoritmos como dijkstra. considera pesos se o grafo for ponderado.
float menorCaminhoMedio(Grafo *G);

// calculaAssortatividade: calcula a assortatividade da rede.
float calculaAssortatividade(Grafo *G);

// funcao para carregar o grafo a partir de um arquivo de arestas (Edges List)
// necessaria para gerar o grafo a partir dos arquivos da base do repositório[cite: 49].
int carregaGrafoDeArquivo(Grafo *G, const char *nome_arquivo);

// funcao para obter o label de um vértice (útil para saída)
char* getVerticeLabel(Grafo *G, int v_idx);

// funcao para contar o número de arestas (não explicitamente pedida, mas útil para validação)
long long contaArestas(Grafo *G);

#endif