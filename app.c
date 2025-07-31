#include <stdio.h>
#include <stdlib.h>
#include "grafo.h" // Inclui a interface do seu TAD de grafo

int main() {
    // 1. cria o grafo
    // o movieLens user2tags é um grafo bipartido entre usuários e tags.
    // pode ser considerado não dirigido se a relação "usuário X usou tag Y" for simétrica.
    // não ponderado por padrão no arquivo .edges, mas pode ser se os dados permitirem um peso.
    // vamos considerar não dirigido e não ponderado para este exemplo.

    Grafo *meu_grafo = criaGrafo(100, 0, 0); // capacidade inicial 100 vértices, NÃO DIGRAFO, NÃO PONDERADO
    if (meu_grafo == NULL) {
        fprintf(stderr, "Erro ao criar o grafo.\n");
        return 1;
    }

    // 2. carrega os dados do MovieLens
    // certifique-se de que 'ia-movielens-user2tags-10m.edges' esteja no mesmo diretório
    // ou forneça o caminho completo.

    printf("Carregando grafo do arquivo ia-movielens-user2tags-10m.edges...\n");
    if (!carregaGrafoDeArquivo(meu_grafo, "ia-movielens-user2tags-10m.edges")) { 
        fprintf(stderr, "Erro ao carregar o grafo do arquivo.\n");
        // liberaGrafo(meu_grafo);
        return 1;
    }
    printf("Grafo carregado com sucesso!\n");
}