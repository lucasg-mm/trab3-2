//Lucas Gabriel Mendes Miranda, 10265892

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define TAM_REG_CAB 19  //tamanho fixo do registro de cabeçalho
#define TAM_REG_PAD 85  //tamanho fixo do registro padrão
#define NUM_REG_MAX 5000  //número máximo de registros no arquivo de dados
#define NUM_VER_MAX 10000  //número máximo de vértices no grafo
#define INFINITO 99990  //para utilizar no algoritmo de Dijkstra (vetor de distâncias)
#define PHI "-2"  //para utilizar no algoritmo de Dijkstra (vetor de antecessores - anterior indefinido)

/**
 * Uma lista de adjacência é composta por duas partes:
 * 1) Vetor;
 * 2) Lista sequencial.
 */

/**
 * Representa um vértice do grafo. Na lista de adjacências, pode ser parte do
 * vetor ou da lista linear.
 */
typedef struct vertice {
    char nomeCidade[100]; //nome da cidade do vértice
    char estado[3]; //estado onde se localiza a cidade

    /**
     * Só é usado no contexto de lista sequencial. Corresponde à distância
     * entre o cabeça da lista (que é um elemento da parte vetor) até a cidade
     * representada pelo vértice. 
     */
    int pesoAresta;

    /**
     * Só é usado no contexto de lista sequencial. Corresponde ao tempo de viagem
     * entre o cabeça da lista (que é um elemento da parte vetor) até a cidade
     * representada pelo vértice. 
     */
    char tempoViagem[100];

    int numVertices;  //número de vértices do grafo (só é preenchido no primeiro elemento da parte vetor da lista de adjacência)

    struct vertice* conectado; //aponta para um vértice conectado na lista de adjacências (pode ser adjacente ou não)
} VERTICE;

/**
 * Esse é o tipo armazenado pela fila. Corresponde a um elemento no conjunto de
 * vértices cuja menor distância em relação ao vértice de origem ainda não foi 
 * determinada definitivamente.
 */
typedef struct peso {
    int distancia; //menor distância até agora
    int indiceCorresponde; //índice correspondente no vetor onde ficam armazenadas as menores distâncias.
} PESO;

/**
 * Definição do tipo fila. Nesse programa, toda inserção ocorre de maneira ordenada.
 * Corresponde a uma fila de elementos no conjunto de vértices cuja menor distância
 * em relação ao vértice de origem ainda não foi determinada definitivamente.
 */
typedef struct fila {
    int qtd; //quantidade de elementos na fila
    PESO dados[NUM_VER_MAX]; //vetor da fila.
} Fila;

/**
 * Representa um registro no arquivo de dados, como especificado na parte anterior
 * do trabalho.
 */
typedef struct registro_padrao { //modelo de um registro padrão (tamanho fixo)
    //campos do arquivo de dados:
    char estadoOrigem[3];
    char estadoDestino[3];
    int distancia;
    char cidadeOrigem[100];
    char cidadeDestino[100];
    char tempoViagem[100]; //indica o tempo de viagem entre as duas localidades (aceita valores nulos)

    //indica o número de arestas que o grafo gerado terá:
    int numReg;
} REGISTRO_PADRAO;

//Função disponibilizada pelo Matheus:

void scan_quote_string(char *str) {

    /*
     *	Use essa função para ler um campo string delimitado entre aspas (").
     *	Chame ela na hora que for ler tal campo. Por exemplo:
     *
     *	A entrada está da seguinte forma:
     *		nomeDoCampo "MARIA DA SILVA"
     *
     *	Para ler isso para as strings já alocadas str1 e str2 do seu programa, você faz:
     *		scanf("%s", str1); // Vai salvar nomeDoCampo em str1
     *		scan_quote_string(str2); // Vai salvar MARIA DA SILVA em str2 (sem as aspas)
     *
     */

    char R;

    while ((R = getchar()) != EOF && isspace(R)); // ignorar espaços, \r, \n...

    if (R == 'N' || R == 'n') { // campo NULO
        getchar();
        getchar();
        getchar(); // ignorar o "ULO" de NULO.
        strcpy(str, ""); // copia string vazia
    } else if (R == '\"') {
        if (scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
            strcpy(str, "");
        }
        getchar(); // ignorar aspas fechando
    } else if (R != EOF) { // vc tá tentando ler uma string que não tá entre aspas! Fazer leitura normal %s então...
        str[0] = R;
        scanf("%s", &str[1]);
    } else { // EOF
        strcpy(str, "");
    }
}

/**
 * Essa função preenche uma string de um dado tamanho com '\0'.
 */
void limparString(char string[], int tamanho) {
    int i;

    for (i = 0; i < tamanho; i++) {
        string[i] = '\0';
    }

    return;
}

/*
 * Essa função lê um campo de variável apontado pela posição corrente do
 * cursor associado ao descritor de arquivo passado como parâmetro. Esse
 * campo é retornado pela função como uma string.
 * Retorno:
 * campoLido - o campo que foi lido
 */
char* lerCampoVariavel(FILE* descritor) {
    char* campoLido;
    char percorre;
    int i;

    i = 0;

    //alocação:
    campoLido = (char*) calloc(100, sizeof (char));
    if (campoLido == NULL) {
        return NULL;
    }

    //lê o primeiro caractere do campo:
    percorre = fgetc(descritor);

    //preenche o campoLido:
    while ((percorre != '|') && (i < 100)) {
        campoLido[i] = percorre;
        percorre = fgetc(descritor);
        i++;
    }

    return campoLido;
}

/*
 * Essa função lê o registro apontado pelo cursor do arquivo associado 
 * ao descritor passado como parâmetro. 
 * Retorno:
 * registroLido - o registro que foi lido
 */
REGISTRO_PADRAO* lerRegistro(FILE* descritor) {
    REGISTRO_PADRAO* registroLido;
    char* campoLido;

    //aloca espaço para o registroLido:
    registroLido = (REGISTRO_PADRAO*) calloc(1, sizeof (REGISTRO_PADRAO));
    if (registroLido == NULL) {
        return NULL;
    }

    //lê os campos de tamanho fixo:
    if (fread(&(registroLido->estadoOrigem), sizeof (char), 2, descritor) != 2) {
        free(registroLido);
        return NULL;
    }
    fread(&(registroLido->estadoDestino), sizeof (char), 2, descritor);
    fread(&(registroLido->distancia), sizeof (int), 1, descritor);

    //lê os campos de tamanho variável:
    campoLido = lerCampoVariavel(descritor);
    strcpy(registroLido->cidadeOrigem, campoLido);
    free(campoLido);

    campoLido = lerCampoVariavel(descritor);
    strcpy(registroLido->cidadeDestino, campoLido);
    free(campoLido);

    campoLido = lerCampoVariavel(descritor);
    strcpy(registroLido->tempoViagem, campoLido);
    free(campoLido);

    return registroLido;
}

/*
 * Essa função transfere os dados de um arquivo binário para um vetor de
 * REGISTRO_PADRAO 
 * Retorno:
 * registros - vetor de registros gerado
 */
REGISTRO_PADRAO* lerTodosOsRegistros(char* nomeDados) {
    int i;
    char arquivoCorrompido;

    //aloca o vetor que ficará com os dados:
    REGISTRO_PADRAO* registros = (REGISTRO_PADRAO*) calloc(NUM_REG_MAX, sizeof (REGISTRO_PADRAO));
    if (registros == NULL) {
        printf("Falha na execução da funcionalidade.\n");
        return NULL;
    }
    REGISTRO_PADRAO* aux;

    //abre o arquivo:
    FILE* descritor = fopen(nomeDados, "rb");
    if (descritor == NULL) {
        printf("Falha no carregamento do arquivo.\n");
        free(registros);
        return NULL;
    }

    //checa se o arquivo está corrompido através da flag no cabeçalho:
    fread(&arquivoCorrompido, sizeof (char), 1, descritor);
    if (arquivoCorrompido == '0') {
        printf("Falha no carregamento do arquivo.\n");
        free(registros);
        fclose(descritor);
        return NULL;
    }

    fseek(descritor, TAM_REG_CAB, SEEK_SET); //muda o posição corrente para além do cabeçalho

    //lê um registro:
    aux = lerRegistro(descritor);
    for (i = 0; (i < NUM_REG_MAX) && (aux != NULL); i++) {
        //insere no vetor de registros:
        strcpy(registros[i].cidadeDestino, aux->cidadeDestino);
        strcpy(registros[i].cidadeOrigem, aux->cidadeOrigem);
        strcpy(registros[i].estadoDestino, aux->estadoDestino);
        strcpy(registros[i].estadoOrigem, aux->estadoOrigem);
        strcpy(registros[i].tempoViagem, aux->tempoViagem);
        registros[i].distancia = aux->distancia;
        free(aux);

        //avança cursor:
        fseek(descritor, TAM_REG_CAB + TAM_REG_PAD * (i + 1), SEEK_SET);

        //lê o próximo registro:
        aux = lerRegistro(descritor);
    }
    if (aux != NULL) {
        free(aux);
    }
    //fecha o arquivo:
    fclose(descritor);

    return registros;
}

/*
 * Essa função insere um vértice de maneira ordenada na parte vetor da lista de adjacencias.
 * A inserção precisa ser ordenada para viabilizar a busca binária.
 * Retorno:
 * n - tamanho do vetor, se ele já estiver cheio
 * i + 1 - índice do ítem inserido
 */
int insereOrdenadoVetor(VERTICE* verticesOrdenados, int n, char* chave, char* estado) {
    //Não pode inserir mais um elemento se n for superior ao número de vértices máximo
    if (n >= NUM_VER_MAX) {
        return n;
    }

    //acha a posição correta para inserir:
    int i;
    for (i = n - 1; (i >= 0 && (strcmp(verticesOrdenados[i].nomeCidade, chave) > 0)); i--) {
        strcpy(verticesOrdenados[i + 1].nomeCidade, verticesOrdenados[i].nomeCidade);
        strcpy(verticesOrdenados[i + 1].estado, verticesOrdenados[i].estado);
        verticesOrdenados[i + 1].conectado = verticesOrdenados[i].conectado;
    }

    //insere:
    strcpy(verticesOrdenados[i + 1].nomeCidade, chave);
    strcpy(verticesOrdenados[i + 1].estado, estado);
    verticesOrdenados[i + 1].conectado = NULL;

    return (i + 1);
}

/**
 * Essa função realiza uma busca binária simples na parte vetor da lista de adjacencias.
 * Retorno:
 * -1 - se o vértice não foi encontrado
 * meio - o índice do vértice no vetor
 */
int buscaBinaria(VERTICE* verticesOrdenados, char *chave, int tam) {
    int bottom = 0;
    int meio;
    int top = tam - 1;

    while (bottom <= top) {
        meio = (bottom + top) / 2;
        if (strcmp(verticesOrdenados[meio].nomeCidade, chave) == 0) {
            return meio;  //vértice encontrado
        } else if (strcmp(verticesOrdenados[meio].nomeCidade, chave) > 0) {
            top = meio - 1;
        } else if (strcmp(verticesOrdenados[meio].nomeCidade, chave) < 0) {
            bottom = meio + 1;
        }
    }

    return -1;  //vértice não encontrado
}

/**
 * Função para inserir ordenado um vértice na parte lista da lista de adjacências.
 */
void insereOrdenadoLista(VERTICE* verticesOrdenados, char* cidade, char* tempoViagem, int pesoAresta, char* estado) {
    VERTICE* novoVertice;
    VERTICE* anterior;
    VERTICE* cabeca;

    //alocação de memória:
    novoVertice = (VERTICE*) calloc(1, sizeof (VERTICE));
    if (novoVertice == NULL) {
        printf("Falha na execução da funcionalidade.");
        return;
    }

    //preenche o novo vértice:
    strcpy(novoVertice->nomeCidade, cidade);
    strcpy(novoVertice->tempoViagem, tempoViagem);
    novoVertice->pesoAresta = pesoAresta;
    strcpy(novoVertice->estado, estado);


    //chega até o ponto em que se deve inserir:
    cabeca = verticesOrdenados;
    verticesOrdenados = verticesOrdenados->conectado;
    anterior = verticesOrdenados;
    while ((verticesOrdenados != NULL) && (strcmp(verticesOrdenados->nomeCidade, cidade) < 0)) {
        anterior = verticesOrdenados;
        verticesOrdenados = verticesOrdenados->conectado;
    }

    if (anterior == verticesOrdenados) { //inserindo no começo, quando a lista não está vazia
        cabeca->conectado = novoVertice;
        novoVertice->conectado = verticesOrdenados;
        return;
    }

    //conecta o vértice:
    anterior->conectado = novoVertice;
    novoVertice->conectado = verticesOrdenados;

    return;
}

/**
 * Essa função administra a inserção de vértices na lista de adjacências.
 * Ela pode chamar as seguintes funções:
 * 1) insereOrdenadoVetor - que insere ordenado na parte vetor da lista
 * 2) insereOrdenadoLista - que insere ordenado na parte lista da lista
 * Retorno:
 * - numVertices: o tamanho da parte vetor após a inserção
 */
int administraInsercaoListaAdjacencia(REGISTRO_PADRAO registro, VERTICE* verticesOrdenados, int numVertices) {
    int indiceEncontrado;  //armazena o índice da variável 'cidade', se ela existir na parte vetor da lista de adjacência
    int i = 0;
    char cidade[100];  //cidade a ser inserida na parte vetor, se ela ainda não existir como vértice
    char outraCidade[100];  //cidade a ser inserida na parte lista, cuja cabeça é o vértice da cidade definida pela variável acima
    char tempo[100];  //tempo de viagem entre outraCidade e cidade
    int distancia;  //distância entre outraCidade e cidade
    char estado[3];  //estado de cidade
    char outroEstado[3];  //estado de outraCidade

    //limpa todas as strings:
    limparString(cidade, 100);
    limparString(outraCidade, 100);
    limparString(tempo, 100);
    limparString(estado, 3);
    limparString(outroEstado, 3);

    //inicializa variáveis com base em um registro ddo vetor de registros:
    strcpy(cidade, registro.cidadeOrigem);
    strcpy(outraCidade, registro.cidadeDestino);
    strcpy(tempo, registro.tempoViagem);
    distancia = registro.distancia;
    strcpy(estado, registro.estadoOrigem);
    strcpy(outroEstado, registro.estadoDestino);


    while (i < 2) {
        //verifica se um potencial vértice já existe na parte vetor e retorna seu índice:
        indiceEncontrado = buscaBinaria(verticesOrdenados, cidade, numVertices);

        if (indiceEncontrado != -1) { //encontrou 
            //insere a outra cidade  na parte lista linear:
            insereOrdenadoLista(&verticesOrdenados[indiceEncontrado], outraCidade, tempo, distancia, outroEstado);
        } else { //não encontrou 
            //insere na parte vetor:
            indiceEncontrado = insereOrdenadoVetor(verticesOrdenados, numVertices, cidade, estado); //insere ordenado
            //insere a outra cidade  na parte lista linear:
            insereOrdenadoLista(&verticesOrdenados[indiceEncontrado], outraCidade, tempo, distancia, outroEstado);
            //o número de vértices cresce, nesse caso:
            numVertices++;
        }

        //limpa todas as strings:
        limparString(cidade, 100);
        limparString(outraCidade, 100);
        limparString(estado, 3);
        limparString(outroEstado, 3);

        //repete o processo, mas com a 'outraCidade', pois queremos contabilizá-la como possível vértice também:
        strcpy(cidade, registro.cidadeDestino);
        strcpy(outraCidade, registro.cidadeOrigem);
        strcpy(estado, registro.estadoDestino);
        strcpy(outroEstado, registro.estadoOrigem);

        i++;
    }

    return numVertices; //retorna o tamanho do vetor de VERTICEs após a inserção
}

/**
 * Essa função gera uma lista de adjacências a partir de um vetor de REGISTROS_PADRAO.
 * Retorna:
 * -listaAdjacencia - a lista de adjacência gerada.
 */
VERTICE* geraListaAdjacencia(REGISTRO_PADRAO* registros) {
    VERTICE* listaAdjacencia; //lista de adjacência gerada
    int numVertices = 0;
    int j = 0;

    //aloca uma lista de adjacências com o tamanho máximo de vértices:
    listaAdjacencia = (VERTICE*) calloc(NUM_VER_MAX, sizeof (VERTICE));
    if (listaAdjacencia == NULL) { //erro na alocação de memória
        printf("Falha na execução da funcionalidade.\n");
        return NULL;
    }

    //faz a busca binária das restantes para decidir se vai inserir ou incrementar:
    while ((j < NUM_REG_MAX) && (strcmp(registros[j].cidadeDestino, "") != 0)) {
        //cada registro possui dois novos vértices em potencial:
        if (registros[j].estadoOrigem[0] != '*') { //só conta se não for um registro excluído
            numVertices = administraInsercaoListaAdjacencia(registros[j], listaAdjacencia, numVertices);
        }

        //incrementa para preservar o loop:
        j++;
    }

    //guarda o número de vértices do grafo apenas no primeiro elemento da parte de vetores
    listaAdjacencia[0].numVertices = numVertices;

    return listaAdjacencia;
}

/**
 * Essa função imprime uma lista de adjacência de acordo com a especificação do trabalho.
 */
void imprimeListaAdjacencia(VERTICE* listaAdjacencia) {
    int i = 0; //para ajudar a percorrer a parte vetor

    /**
     * Essa variável irá percorrer a lista de adjacência por inteiro. É necessária
     * para imprimir os dados.
     */
    VERTICE* percorre;

    while ((i < NUM_VER_MAX) && (listaAdjacencia[i].nomeCidade[0] != '\0')) { //enquanto não terminou de percorrer a lista...

        printf("%s %s", listaAdjacencia[i].nomeCidade, listaAdjacencia[i].estado); //imprime a parte vetor
        percorre = listaAdjacencia[i].conectado; //conecta o percorre ao primeiro elemento da parte lista

        while (percorre != NULL) { //enquanto não chegou ao final da parte lista...

            if (strcmp(percorre->tempoViagem, "\0") == 0) { //se o tempo de viagem não estiver disponível
                printf(" %s %s %d", percorre->nomeCidade, percorre->estado, percorre->pesoAresta); //imprime a parte lista sem o tempo de viagem (se imprimisse com o tempo de viagem, colocaria um espaço no lugar, fiz assim para evitar problemas com o run.codes)
                percorre = percorre->conectado; //vai para o próximo elemento da parte lista
                continue; //continua o loop
            }

            printf(" %s %s %d %s", percorre->nomeCidade, percorre->estado, percorre->pesoAresta, percorre->tempoViagem); //imprime a parte lista
            percorre = percorre->conectado; //vai para o próximo elemento da parte lista
        }

        printf("\n");

        i++;
    }
}

/**
 * Essa função libera a parte lista linear da lista de adjacência.
 */
void liberaLista(VERTICE* listaAdjacencia) {
    int i = 0; //para percorrer a parte vetor (que NÃO será liberada)
    VERTICE* percorre; //para percorrer a parte lista (que será liberada)
    VERTICE* aux; //para guardar o próximo elemento da parte lista, antes de liberar um vértice

    while ((i < NUM_VER_MAX) && (listaAdjacencia[i].nomeCidade[0] != '\0')) { //enquanto não terminou de percorrer a lista de adjacências...

        percorre = listaAdjacencia[i].conectado; //conecta o percorrer ao primeiro elemento da parte lista

        while (percorre != NULL) { //enquanto não terminou de percorreu a parte lista...
            aux = percorre->conectado; //guarda o próximo elemento de percorre no auxiliar
            free(percorre); //libera percorre
            percorre = aux; //define percorre como o seu próximo, antes de ser liberado
        }

        i++;
    }
}

//-----------Implementação de uma fila (necessária para a aplicação de Dijkstra)-------------

/**
 * Cria uma fila.
 * Retorna:
 * - A lista que acabou de ser criada
 */
Fila* criaFila() {
    Fila *aSerCriada;
    aSerCriada = (Fila*) malloc(sizeof (struct fila));
    if (aSerCriada != NULL)
        aSerCriada->qtd = 0;
    return aSerCriada;
}

/**
 * Libera uma fila.
 */
void liberaFila(Fila* aSerLiberada) {
    free(aSerLiberada);
}

/**
 * Diz se uma fila está vazia.
 * Retorna:
 * -1 - se a fila não existe;
 *  1 - se a fila não está vazia;
 *  0 - se a fila está vazia
 */
int filaVazia(Fila* aSerVerificada) {
    if (aSerVerificada == NULL)
        return -1;
    return (aSerVerificada->qtd == 0);
}

/**
 * Insere numa dada fila de maneira ordenada.
 * Retorna:
 * 0 - se a fila não existe ou estiver cheia;
 * 1 - se a inserção foi bem sucedida.
 */
int insereFila(Fila* fi, int al, int indice) {
    if (fi == NULL) //fila não existe
        return 0;
    if (fi->qtd == NUM_VER_MAX) //fila cheia
        return 0;
    int k, i = 0;

    //insere ordenado:
    while (i < fi->qtd && fi->dados[i].distancia < al)
        i++;

    for (k = fi->qtd - 1; k >= i; k--)
        fi->dados[k + 1] = fi->dados[k];

    fi->dados[i].distancia = al;
    fi->dados[i].indiceCorresponde = indice;
    fi->qtd++;
    return 1;
}

/**
 * Remove o primeiro elemento na fila, que é sempre o menor, pois todas as
 * inserções são feitas de maneira ordenada.
 * Retorna:
 * 0 - se a fila não existir, ou estiver vazia
 * indiceRemovido - o índice no vetor de caminhos mais curtos do elemento removido
 */
int removePrimeiroFila(Fila* fi) { //remove pelo nome da cidade
    int indiceRemovido = fi->dados[0].indiceCorresponde;

    if (fi == NULL) //fila não existe
        return 0;
    if (fi->qtd == 0) //fila vazia
        return 0;

    //remove ordenado:
    int k = 0;
    for (k = 0; k < fi->qtd - 1; k++)
        fi->dados[k] = fi->dados[k + 1];
    fi->qtd--;

    return indiceRemovido;
}

/**
 * Remove um elemento qualquer da fila.
 * Retorna:
 * 0 - se a remoção fracassou
 * 1 - se a remoção foi bem sucedida
 */
int removeFila(Fila* fi, int aRemover, int indiceCorrespondente) {
    if (fi == NULL) //fila não existe
        return 0;
    if (fi->qtd == 0) //fila vazia
        return 0;

    //remove:
    int k, i = 0;
    while (i < fi->qtd && (fi->dados[i].distancia != aRemover || fi->dados[i].indiceCorresponde == indiceCorrespondente))
        i++;
    if (i == fi->qtd) //elemento nao encontrado
        return 0;

    for (k = i; k < fi->qtd - 1; k++)
        fi->dados[k] = fi->dados[k + 1];
    fi->qtd--;
    return 1;
}

//-------------------------fim das rotinas de fila---------------------------------------------
/**
 * Imprime o vetor de menor caminho e o de antecessores, como especificado.
 */
void imprimeMenorCaminho(VERTICE* antecessores, int* caminhosMaisCurtos, VERTICE* listaAdjacentes, int indiceOrigem) {
    int i; //para percorrer a lista de adjacentes

    i = 0;
    while ((i < NUM_VER_MAX) && (listaAdjacentes[i].nomeCidade[0] != '\0')) { //enquanto não terminou de percorrer a lista de adjacentes...

        if (i != indiceOrigem) { //não imprime o correspondente ao índice de origem
            //imprime como especificado:
            printf("%s %s %s %s %d %s %s\n", listaAdjacentes[indiceOrigem].nomeCidade, listaAdjacentes[indiceOrigem].estado, listaAdjacentes[i].nomeCidade, listaAdjacentes[i].estado, caminhosMaisCurtos[i], antecessores[i].nomeCidade, antecessores[i].estado);
        }
        i++;

    }

    return;
}

/**
 * Algoritmo de Dijkstra. Acha o caminho mais curto de todos os vértices de um grafo
 * representado por lista de adjacência a um vértice de origem. Nesse procedimento,
 * dois vetores são gerados:
 * jaDeterminados - possui vértices cujo caminho mais curto em relação ao vértice de origem já foi
 * determinado;
 * anteriores - vetor de antecessores.
 * 
 * Esses  dois vetores são impressos no final da função. 
 */
void menorCaminhoDijkstra(VERTICE* listaAdjacencia, int indiceOrigem) {
    /**
     * Vetor de antecessores.
     */
    VERTICE* antecessores;

    /**
     * Vetor de caminhos mais curtos.
     */
    int* caminhosMaisCurtos;

    /**
     * Armazena vértices cujo caminho mais curto entre o vértice de origem ainda não foi determinado.
     */
    Fila* naoDeterminados = criaFila();

    /**
     * Indice de um elemento na lista de adjacentes (parte vetor) que corresponde a uma 
     * distância que foi retirada da fila.
     */
    int indiceRemovido;

    /**
     * Ajuda a percorrer a parte lista linear da lista de adjacência
     */
    VERTICE* percorre;

    /**
     * Índice correspondente a um vértice apontado por percorre, na parte vetor da lista de adjacência
     */
    int indiceCorrespondente;

    int i;

    //alocando espaço para os dois vetores a serem gerados no fim do proc.:
    antecessores = (VERTICE*) calloc(NUM_VER_MAX, sizeof (VERTICE));
    if (antecessores == NULL) {
        printf("Falha na execução da funcionalidade.\n");
        return;
    }

    caminhosMaisCurtos = (int*) calloc(NUM_VER_MAX, sizeof (int));
    if (caminhosMaisCurtos == NULL) {
        printf("Falha na execução da funcionalidade.\n");
        free(antecessores);
        return;
    }

    //inicializa o vetor de caminhos mais curtos:
    i = 0;
    while ((i < NUM_VER_MAX) && (listaAdjacencia[i].nomeCidade[0] != '\0')) {
        if (i != indiceOrigem) { //inicializa todos, menos os correspondentes ao vértice de origem
            caminhosMaisCurtos[i] = INFINITO;
            strcpy(antecessores[i].nomeCidade, PHI);
            strcpy(antecessores[i].estado, PHI);
        }
        i++;
    }

    /**
     * A situação inicial no algoritmo é:
     * - Nenhum vértice teve o caminho mais curto determinado, exceto a origem;
     * - A origem tem caminho mais curto zero;
     * - A origem tem antecessor indeterminado (chamaremos de PHI).
     */
    caminhosMaisCurtos[indiceOrigem] = 0;
    strcpy(antecessores[indiceOrigem].nomeCidade, PHI);
    strcpy(antecessores[indiceOrigem].estado, PHI);

    //insere todos os vértices na fila dos vertices que não tiveram o caminho mais curto determinado:
    i = 0;
    while ((i < NUM_VER_MAX) && (listaAdjacencia[i].nomeCidade[0] != '\0')) {
        insereFila(naoDeterminados, caminhosMaisCurtos[i], i);
        i++;
    }

    indiceRemovido = removePrimeiroFila(naoDeterminados); //remove a origem da fila, pois ela já teve o caminho mais curto determinado

    //enquanto a fila não estiver vazia itera no loop principal:
    i = 0;
    while (filaVazia(naoDeterminados) == 0) {
        /**
         * para todo vértice adjacente ao que acabou de ser retirado da fila, faça:
         * caminhosMaisCurtos[indiceCorrespondente] = min(caminhosMaisCurtos[indiceCorrespondente], percorre.pesoAresta + caminhosMaisCurtos[indiceCorrespondente])        
         */
        percorre = &listaAdjacencia[indiceRemovido];
        while (percorre->conectado != NULL) {
            percorre = percorre->conectado;
            indiceCorrespondente = buscaBinaria(listaAdjacencia, percorre->nomeCidade, listaAdjacencia[0].numVertices);

            if (percorre->pesoAresta + caminhosMaisCurtos[indiceRemovido] < caminhosMaisCurtos[indiceCorrespondente]) {
                //remove o antigo da fila:
                removeFila(naoDeterminados, caminhosMaisCurtos[indiceCorrespondente], indiceCorrespondente);
                //atualiza o caminhos mais curtos:
                caminhosMaisCurtos[indiceCorrespondente] = percorre->pesoAresta + caminhosMaisCurtos[indiceRemovido];
                //insere o novo na fila:
                insereFila(naoDeterminados, caminhosMaisCurtos[indiceCorrespondente], indiceCorrespondente);
                //atualiza o vetor de antecessores:
                strcpy(antecessores[indiceCorrespondente].nomeCidade, listaAdjacencia[indiceRemovido].nomeCidade);
                strcpy(antecessores[indiceCorrespondente].estado, listaAdjacencia[indiceRemovido].estado);
            }
        }

        //conclui-se determinando a menor distância definitiva de um vértice
        indiceRemovido = removePrimeiroFila(naoDeterminados); //retira esse vértice da fila de não determinados
    }

    //imprime os vetores de menor caminho e anteriores:
    imprimeMenorCaminho(antecessores, caminhosMaisCurtos, listaAdjacencia, indiceOrigem);

    //libera memória:
    free(caminhosMaisCurtos); //menores caminhos
    free(antecessores); //antecessores
    liberaFila(naoDeterminados); //fila de não determinados

    return;
}

/**
 *  Encontra o índice de um registro na lista de adjacencias.
 *  Retorna -1 se o registro não for encontrado na lista de adjacencias
 * **/
int getIndex(VERTICE* v, VERTICE* listaAdjacencia){
    for (int i = 0; i < listaAdjacencia[0].numVertices; i++){
        if (!strcmp(listaAdjacencia[i].nomeCidade, v -> nomeCidade ))
            return i;
    }
    return 0;
}
/**
 *  Retorna 1 se o vetor estiver cheio de 1, o que quer dizer que já chegou a todos os vertices (U == V)
 *  Retorna 0 se ainda tiver algum vertice não encontrado
 * **/
int isFull(int* U, int numVertices){
    for(int i = 0; i < numVertices; i++){
        if (U[i] == 0)
            return 0;
    }
    return 1;
}
/** Gera o vetor prox e o vetor mc a partir de U e da lista de Adjacencias
 * prox[i] fornece o vértice em V-U atualmente mais próximo ao vertice i em U
 * mc[i] fornece o custo da aresta (i, prox[i])
 * **/
void genProx(VERTICE** prox, int** mc, int* U, VERTICE* listaAdjacencia) {
    int numVertices = listaAdjacencia[0].numVertices;
    // Cria os vetores
    *prox = malloc(sizeof(VERTICE)* numVertices);
    *mc = malloc(sizeof(int)* numVertices);

    // Se um vertice não estiver conectado com U (ou estiver em U) a dist vai ser infinita
    for(int i = 0; i< numVertices; i++){
        (*mc)[i] = INFINITO;
    }

    // Olha em todos os vertices em U
    for(int i = 0; i < numVertices; i++){
        if (U[i]){
            // Percorre a lista encadeada e acha o a menor dist
            for (VERTICE* v = listaAdjacencia[i].conectado; v != NULL; v = v->conectado){
                // Somente para vertices que não estão em U
                if(!U[getIndex(v, listaAdjacencia)]){
                    if(v->pesoAresta < (*mc)[i]){
                        (*prox)[i] = *v;
                        (*mc)[i] = v -> pesoAresta;
                    }
                }
            }

        }
    }
}
/** Algoritimo de Prim. Acha uma Arvore Geradora Mínima (Minimum Spanning Tree) de um grafo
 * representado por uma lista de adjacência a partir de um vértice de origem. Gera a
 * lista de adjacência MSTListaAdj, que guarda a MST e a imprime no final da função.
 * Nos comentários desta função, U e V serão usados para representar, respectivamente,
 * o conjunto dos vértices que já foram incluidos na árvore em um determinado ponto no algoritmo
 * e o conjunto de todos os vertices.
 * **/
void primsMST(VERTICE* listaAdjacencia, int indiceOrigem) {

    int numVertices = listaAdjacencia[0].numVertices;

    VERTICE* prox; //prox[i] fornece o vértice em V-U atualmente mais próximo ao vertice i em U
    int* mc; //mc[i] fornece o custo da aresta (i, prox[i])

    VERTICE* MSTListaAdj; //lista de adjacência gerada
    //aloca uma lista de adjacências com o tamanho máximo de vértices:
    MSTListaAdj= (VERTICE*) calloc(NUM_VER_MAX, sizeof (VERTICE));
    if (MSTListaAdj== NULL) { //erro na alocação de memória
        printf("Falha na execução da funcionalidade.\n");
        return;
    }
    // Cria o vetor U, que vai guardar um valor booleano para cada vértice, representando se
    // o vértice ja foi incluído na arvore (1) ou não (0)
    int U[numVertices];

    // Começa sem nenhum vértice em U
    for (int i = 0; i < numVertices; i++){
        U[i] = 0;
    }
    // Adiciona todos os vértices na arvore, sem nenhuma aresta.
    for(int i = 0; i < numVertices; i++){
        strcpy(MSTListaAdj[i].nomeCidade, listaAdjacencia[i].nomeCidade);
        strcpy(MSTListaAdj[i].estado, listaAdjacencia[i].estado);
        MSTListaAdj[i].conectado = NULL;
    }

    // Adiciona o primeiro vertice a U
    U[indiceOrigem] = 1;

    VERTICE* v;
    int min;
    int dist;


    while(!isFull(U, numVertices)){ // Enquanto U != V
        //
        v = NULL;
        min = 0;
        dist = INFINITO;

        //Gera prox e mc
        genProx(&prox, &mc, U, listaAdjacencia);

        // Encontra a aresta de menor peso que conecta U a V-U percorrendo o vetor mc
        for (int i = 0; i < numVertices; i++){
            if(mc[i] < dist){
                v = &prox[i];
                min = i;
                dist = mc[i];
            }
        }
        if(v == NULL)
            return;
        // Insere a aresta na arvore
        insereOrdenadoLista(&(MSTListaAdj[min]), v -> nomeCidade, v->tempoViagem, dist, v->estado);
        // Insere o vertice em U
        U[getIndex(v, listaAdjacencia)] = 1;

        free(prox);
        free(mc);
    }

    imprimeListaAdjacencia(MSTListaAdj); //imprime a arvore

}

int main() {
    //DECLARAÇÕES DE VARIÁVEIS:
    int funcionalidade; //armazena a funcionalidade escolhida pelo usuário
    //Armazenam as várias entradas do usuário:
    char entrada1[25];
    char entrada2[25];
    char entrada3[25];
    REGISTRO_PADRAO* registrosLidos; //registros lidos do arquivo binário
    VERTICE* listaAdjacencia; //lista de adjacência gerada a partir dos registros lidos
    int indiceOrigem; //guarda o índice do vértice de origem

    //ATRIBUIÇÕES INICIAIS E CHAMADAS DE FUNÇÕES:
    limparString(entrada1, 25);
    limparString(entrada2, 25);
    limparString(entrada3, 25);

    //LÓGICA:
    scanf("%d", &funcionalidade); //obtém qual a funcionalidade será executada

    switch (funcionalidade) {
        case 9: //gera lista de adjacências e imprime
            scanf("%s", entrada1); //obtém o nome do arquivo a partir do qual o grafo será gerado
            registrosLidos = lerTodosOsRegistros(entrada1); //lê os registros do arquivo de dados por inteiro
            if (registrosLidos == NULL) {  //erro
                return 0;
            }
            listaAdjacencia = geraListaAdjacencia(registrosLidos); //faz uma lista de adjacência com os registros lidos
            if (listaAdjacencia == NULL) {  //erro
                return 0;
            }
            imprimeListaAdjacencia(listaAdjacencia); //imprime a lista

            //libera memória:
            liberaLista(listaAdjacencia); //libera parte lista
            free(listaAdjacencia); //libera parte vetor
            free(registrosLidos); //libera os registros lidos do arquivo binário
            break;

        case 10:
            //basicamente, refaz parte do caso 9, para então, aplicar Dijkstra:
            scanf("%s", entrada1); //obtém o nome do arquivo a partir do qual o grafo será gerado
            scanf("%s", entrada2); //obtém o nome do campo do vértice que será o vértice de origem
            scan_quote_string(entrada3); //obtém o valor do campo utilizado para a busca

            registrosLidos = lerTodosOsRegistros(entrada1); //lê os registros do arquivo de dados por inteiro
            if (registrosLidos == NULL) {  //erro
                return 0;
            }

            listaAdjacencia = geraListaAdjacencia(registrosLidos); //faz uma lista de adjacência com os registros lidos
            if (listaAdjacencia == NULL) {  //erro
                return 0;
            }

            indiceOrigem = buscaBinaria(listaAdjacencia, entrada3, listaAdjacencia[0].numVertices); //busca (na parte vetor da lista de adjacência) a cidade que será o vértice de origem
            if(indiceOrigem == -1){  //se não encontrou a cidade
                printf("Cidade inexistente.\n");
                return 0;
            }
            menorCaminhoDijkstra(listaAdjacencia, indiceOrigem); //calcula os menores caminhos e os imprime

            //libera memória:
            liberaLista(listaAdjacencia); //libera parte lista
            free(listaAdjacencia); //libera parte vetor
            free(registrosLidos); //libera os registros lidos do arquivo binário
            break;

        case 11:
            scanf("%s", entrada1); //obtém o nome do arquivo a partir do qual o grafo será gerado
            scanf("%s", entrada2); //obtém o nome do campo do vértice que será o vértice de origem
            scan_quote_string(entrada3); //obtém o valor do campo utilizado para a busca
            registrosLidos = lerTodosOsRegistros(entrada1); //lê os registros do arquivo de dados por inteiro
            if (registrosLidos == NULL) {  //erro
                return 0;
            }

            listaAdjacencia = geraListaAdjacencia(registrosLidos); //faz uma lista de adjacência com os registros lidos
            if (listaAdjacencia == NULL) {  //erro
                return 0;
            }

            indiceOrigem = buscaBinaria(listaAdjacencia, entrada3, listaAdjacencia[0].numVertices); //busca (na parte vetor da lista de adjacência) a cidade que será o vértice de origem
            if(indiceOrigem == -1){  //se não encontrou a cidade
                printf("Cidade inexistente.\n");
                return 0;
            }
            primsMST(listaAdjacencia, indiceOrigem); //Acha uma arvore geradora minima e imprime

            //libera memoria:
            liberaLista(listaAdjacencia); //libera parte lista
            free(listaAdjacencia); //libera parte vetor
            free(registrosLidos); //libera os registros lidos do arquivo binário
            break;
    }

    return 0;
}


