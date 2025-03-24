#include "io.h"
#include <errno.h>

struct lista *inicializa_lista (){
	struct lista *l = malloc(sizeof(struct lista));
	if (!l)
		return NULL;
	l->head = NULL;
	l->cauda = NULL;
	l->tamanho = 0;
	return l;
}

struct item *cria_item (char *media){
	struct item *novo_item = malloc(sizeof(struct item));
	if (!novo_item) {
		fprintf(stderr, "Erro ao alocar memória para novo item\n");
		return NULL;
	}
	novo_item->valor = media;
	novo_item->proximo = NULL;
	return novo_item;
}

void insere_na_lista (char *media, struct lista *l) {
	struct item *novo_item = cria_item(media);
	if (l->head == NULL) {
		l->head = novo_item;
		l->cauda = novo_item;
	}
	else {
		l->cauda->proximo = novo_item;
		l->cauda = novo_item;
	}
	l->tamanho++;
}

void libera_lista (struct lista *l) {
	if (l->head == NULL) {
		free(l);
		return;
	}
	struct item *aux = l->head;
	while (aux != NULL) {
		struct item *temp = aux;
		aux = aux->proximo;
		free(temp->valor);
		free(temp);
	}
	l->head = NULL;
	l->cauda = NULL;
	l->tamanho = 0;
	free(l);
}

unsigned long conta_linha (FILE *file) {
	unsigned long quantidade = 0;
	char linha[1024]; 

	while (fgets(linha, sizeof(linha), file)) {
        quantidade++;    
    }

    rewind(file);
    
    return quantidade;
}

unsigned long conta_coluna (FILE *file) {
	unsigned long quantidade = 0;
	char linha[1024];
	char *token;
	char *result;

	result = fgets(linha, 1024, file);
	if (result != linha) {
		fprintf (stderr, "erro");
		return 0;
	}

	token = strtok(linha, ",");

	while (token) {
		quantidade++;
		token = strtok(NULL, ",");
	}
	
	rewind(file);

	return quantidade;
}

struct arq_csv *abre (char *path) {
	struct arq_csv *csv = (struct arq_csv *)malloc(sizeof(struct arq_csv));
	if (!csv) {
		printf("Erro de alocação!");
		return NULL;
	}

	csv->arquivo = fopen(path, "r");

	if (!csv->arquivo) {
		printf ("Erro ao abrir o arquivo!\n");
		free(csv);
		return NULL;
	}

	csv->tipos = NULL;
	csv->lin = 0;
	csv->col = 0;
	csv->sizes = NULL;
	csv->matriz = NULL;

	return csv;
}

char* separa(const char *string){
	char *pos_virgula;

	if (!string) return 0;

	pos_virgula = strchr(string, ',');
	if (!pos_virgula) return 0;

	*pos_virgula = '\0';
	return pos_virgula + 1;
}

void cria_matriz (struct arq_csv *csv, unsigned long lin, unsigned long col) {
	csv->lin = lin;
	csv->col = col;
	
	csv->matriz = (char***) malloc(csv->lin * sizeof(char **));
	if (!csv->matriz) {
		printf ("Erro de alocação!\n");
		return;		
	}
	
	// cria matriz e libera memória se a alocação não deu certo
	for (unsigned long i = 0; i < csv->lin; i++) {
		csv->matriz[i] = (char**) malloc(csv->col * sizeof(char *));
		if (!csv->matriz[i]) {
			printf ("Erro de alocação!\n");
			if (i > 0) {
				for (unsigned long j = 0; j < i; j++) { 
					free(csv->matriz[j]);
				}
			}
			free(csv->matriz);
			return;
		}
	}

	for (unsigned long i = 0; i < csv->lin; i++) {
		for (unsigned long j = 0; j < csv->col; j++) 
			csv->matriz[i][j] = NULL;
	}

}

void menu () {
	printf("1) Sumário do Arquivo\n");
	printf("2) Mostrar\n");
	printf("3) Filtros\n");
	printf("4) Descrição dos Dados\n");
	printf("5) Ordenação\n");
	printf("6) Seleção\n");
	printf("7) Dados Faltantes\n");
	printf("8) Salvar Dados\n");
	printf("9) Fim\n");
}

void preenche_matriz (struct arq_csv *csv, struct lista *l) {
	char linha[1024];
	char *copia, *proximo;

	cria_matriz(csv, conta_linha(csv->arquivo), conta_coluna(csv->arquivo));
	
	unsigned long i, j;
	i = 0;
	while (fgets(linha, 1024, csv->arquivo) != NULL && i < csv->lin) {
		if (linha[strlen(linha) - 1] == '\n')
			linha[strlen(linha) - 1] = '\0';
		copia = strdup(linha);
		proximo = separa(copia);
	
		j = 0;
		while (proximo != NULL && j < csv->col) {
			
			csv->matriz[i][j] = copia;
			copia = proximo;
			proximo = separa(copia);
			j++;
		}
		csv->matriz[i][j] = copia;
		i++;
	}
	
	for (unsigned long i = 0; i < csv->lin; i++)
		insere_na_lista(csv->matriz[i][0], l);

}

// verificar se o arquivo passado é varido
int verifica_valido (struct arq_csv *csv){
	for (unsigned long i = 0; i < csv->lin; i++) {
		for (unsigned long j = 0; j < csv->col; j++){
			if (csv->matriz[i][j] == NULL)
				return 0;
		}
	}

	return 1;
}

// Se for numerico retorna 1;
// Se for string retorna 0;
int eh_numerica (const char *string) {
	if (strcmp(string, "") == 0)
		return 0;
	if (*string == '\0') {
		return 0;
	}
	int ponto = 0;

	while (*string != '\0') {
		// função retorna 0 se for um numero diferente de decimal
		if (!isdigit(*string)) {
			if (*string == '.' && !ponto)
				ponto = 1;
			else 
				return 0;
		}
		string++;
	}
	return 1; // É verdadeiro se todos os caracteres forem digitos.
}

// Atribui se é numerica ou não e conta o tamanho de cada string.
void atribui_struct (struct arq_csv *csv) {
	if (!csv->matriz)
		return;
	csv->tipos = malloc(csv->col * sizeof(char));

	if (!csv->tipos) {
		printf ("Erro de alocação!\n");
		return;
	}

	csv->sizes = malloc(csv->col * sizeof(unsigned short));
	
	if (!csv->matriz) {
		printf ("Erro de alocação!\n");
		return;
	}
	
	unsigned long j, i;
	j = 0;
	// quando for 1 é numérico e quando for 0 é string
	while (j < csv->col) {
		i = 0;
		csv->tipos[j] = 0;
		while (i < csv->lin) {
			if(strcmp(csv->matriz[i][j], "") != 0){
				if (eh_numerica(csv->matriz[i][j])) {
					csv->tipos[j] =  1;
					break;
				}
			}
			i++;
		}
		j++;
	}

	unsigned short maior;
	unsigned short aux;

	for (j = 0; j < csv->col; j++) {
		maior = strlen(csv->matriz[0][j]);
		for (i = 0; i < csv->lin; i++){
			// Se for vazio o maior vai ser o tamanho do NaN
			if (strcmp(csv->matriz[i][j], "") == 0 && maior < 3)
				maior = 3;
			else {
				aux = strlen(csv->matriz[i][j]);
				if (maior < aux)
					maior = aux;
			}
		}
		csv->sizes[j] = maior;
	}
}

void sumario (struct arq_csv csv) {
	for (unsigned long j = 0; j < csv.col; j++ ) {
		printf("%s ", csv.matriz[0][j]);
		if (!csv.tipos[j])
			printf("[S]\n");
		else 
			printf("[N]\n");
	}

	printf("%ld variaveis encontradas\n\n", csv.col);

	printf("Pressione ENTER para continuar\n");

	getchar();
}

//verifica a quantidade de digitos da linha atual
int tam_indice (unsigned long num){
	int tam = 0;
	if (num == 0)
		return 1;

	while (num != 0) {
		tam++;
		num = num / 10;
	}
	return tam;
}

void mostra_da_opcao (struct arq_csv csv){
	unsigned long *vetor_aux = (unsigned long *) malloc(csv.lin * sizeof(unsigned long));
	for(unsigned long i = 0; i < csv.lin; i++) {
		vetor_aux[i] = i;
	}

	mostrar (csv.matriz, csv.lin, csv.col, csv.sizes, vetor_aux);
	free(vetor_aux);
	printf("Pressione ENTER para continuar\n");
	getchar();	
}

void mostrar (char ***matriz, unsigned long lin, unsigned long col, unsigned short *v_sizes, unsigned long *v_aux) {
	unsigned short cont;
	// Pega o maior tamanho possivel de indice (o ultimo);
	unsigned short tam_indice_maior = tam_indice(lin - 2);
	if (lin <= 10 ) {
		for (unsigned long i = 0; i < lin; i++) {
			if (v_aux[i] > 0){
				printf("%ld", v_aux[i] - 1);
				for (int k = 0; k < tam_indice_maior - tam_indice(v_aux[i]-1) + 1; k++)
					printf(" ");
			}
			else {
				for (int k = 0; k < tam_indice_maior + 1; k++)
					printf(" ");
			}
				
			for (unsigned long j = 0; j < col; j++) {
				if (strcmp(matriz[i][j], "") != 0)
					cont = v_sizes[j] - strlen(matriz[i][j]) + 1;
				else 
					cont = v_sizes[j] - 2;
				for (int k = 0; k < cont; k++) {
					printf(" ");
				}
				if (strcmp(matriz[i][j], "") == 0)
					printf("%s ", "NaN");
				else 
					printf("%s ", matriz[i][j]);
			}
			printf ("\n");
		}

		printf("\n[%ld rows x %ld columns]\n\n", lin - 1, col);

		return;
	}
	
	for (unsigned long i = 0; i <= 5; i++) {
		if (i > 0){
			printf("%ld", v_aux[i] - 1);
			for (int k = 0; k < tam_indice_maior - tam_indice(v_aux[i]-1) + 1; k++)
				printf(" ");
		}
		else {
			for (int k = 0; k < tam_indice_maior + 1; k++)
				printf(" ");
		}
			
		for (unsigned long j = 0; j < col; j++) {
			if (strcmp(matriz[i][j], "") != 0)
				cont = v_sizes[j] - strlen(matriz[i][j]) + 1;
			else 
				cont = v_sizes[j] - 2;
			for (int k = 0; k < cont; k++) {
				printf(" ");
			}
			if (strcmp(matriz[i][j], "") == 0)
				printf("%s ", "NaN");
			else 
				printf("%s ", matriz[i][j]);
		}
		printf ("\n");
	}

	printf("...");
	for (int k = 0; k < tam_indice_maior - 3; k++)
		printf(" ");
	
	for (unsigned long j = 0; j < col; j++){
		cont = v_sizes[j] - 2;
		for (int k = 0; k < cont; k++) {
			printf(" ");
		}
		printf(" ...");
	}

	printf("\n");


	for (unsigned long i = lin - 5; i < lin; i++) {
		printf("%ld", v_aux[i] - 1);
		for (int k = 0; k < tam_indice_maior - tam_indice(v_aux[i]-1) + 1; k++)
			printf(" ");
		for (unsigned long j = 0; j < col; j++) {
			if (strcmp(matriz[i][j], "") != 0)
				cont = v_sizes[j] - strlen(matriz[i][j]) + 1;
			else 
				cont = v_sizes[j] - 2;
			for (int k = 0; k < cont; k++) {
				printf(" ");
			}
			if (strcmp(matriz[i][j], "") == 0)
				printf("%s ", "NaN");
			else 
				printf("%s ", matriz[i][j]);
		}
		printf ("\n");
	}

	printf("\n[%ld rows x %ld columns]\n\n", lin - 1, col);
}

char *var_filtrar () {
	char *fil = (char *)malloc(3 * sizeof(char));


	printf("Escolha um filtro ( == > >= < <= != ): ");
	scanf ("%s", fil);
	getchar();
	
	return fil;
}

void igual (struct arq_csv *csv, char *vetor_lin, long int var_pos, unsigned long *cont_linha, const char *valor) {
	// CASO NUMERICO
	if (csv->tipos[var_pos] == 1) {
		double *vetor_comp = (double *)malloc((csv->lin - 1) * sizeof(double));
		double num_valor = atof(valor);
		if (!vetor_comp) {
			printf("Erro de alocação!\n");
			return;
		}

		// passar os numeros para o vetor em float;
		unsigned long k = 0;
		for (unsigned long i = 1; i < csv->lin; i++) {
			vetor_comp[k] = atof(csv->matriz[i][var_pos]);
			k++;
		}
	
		for (unsigned long i = 0; i < csv->lin - 1; i++) {
			if (vetor_comp[i] == num_valor){
				(*cont_linha)++;
				vetor_lin[i + 1] = 1;
			}
		}

		free(vetor_comp);
		return;
	}

	for (unsigned long i = 1; i < csv->lin; i++){
		if (strcmp(csv->matriz[i][var_pos], valor) == 0){
			(*cont_linha)++;
			vetor_lin[i] = 1;
		}
	}
}

void maior (struct arq_csv *csv, char *vetor_lin, long int var_pos, unsigned long *cont_linha, const char *valor) {
	// CASO NUMERICO
	if (csv->tipos[var_pos] == 1) {
		double *vetor_comp = (double *)malloc((csv->lin - 1) * sizeof(double));
		double num_valor = atof(valor);
		if (!vetor_comp) {
			printf("Erro de alocação!\n");
			return;
		}

		// passar os numeros para o vetor em float;
		unsigned long k = 0;
		for (unsigned long i = 1; i < csv->lin; i++) {
			vetor_comp[k] = atof(csv->matriz[i][var_pos]);
			k++;
		}
	
		for (unsigned long i = 0; i < csv->lin - 1; i++) {
			if (vetor_comp[i] >  num_valor){
				(*cont_linha)++;
				vetor_lin[i + 1] = 1;
			}
		}

		free(vetor_comp);
		return;
	}
	
	//NAO NUMERICO
	for (unsigned long i = 1; i < csv->lin; i++){
		if (strcmp(csv->matriz[i][var_pos], valor) > 0){
			(*cont_linha)++;
			vetor_lin[i] = 1;
		}
	}
}

void maior_igual (struct arq_csv *csv, char *vetor_lin, long int var_pos, unsigned long *cont_linha, const char *valor) {
	// CASO NUMERICO
	if (csv->tipos[var_pos] == 1) {
		double *vetor_comp = (double *)malloc((csv->lin - 1) * sizeof(double));
		double num_valor = atof(valor);
		if (!vetor_comp) {
			printf("Erro de alocação!\n");
			return;
		}

		// passar os numeros para o vetor em float;
		unsigned long k = 0;
		for (unsigned long i = 1; i < csv->lin; i++) {
			vetor_comp[k] = atof(csv->matriz[i][var_pos]);
			k++;
		}
	
		for (unsigned long i = 0; i < csv->lin - 1; i++) {
			if (vetor_comp[i] >=  num_valor){
				(*cont_linha)++;
				vetor_lin[i + 1] = 1;
			}
		}

		free(vetor_comp);
		return;
	}
	
	//NAO NUMERICO
	for (unsigned long i = 1; i < csv->lin; i++){
		if (strcmp(csv->matriz[i][var_pos], valor) > 0 || strcmp(csv->matriz[i][var_pos], valor) == 0){
			(*cont_linha)++;
			vetor_lin[i] = 1;
		}
	}
}

void menor (struct arq_csv *csv, char *vetor_lin, long int var_pos, unsigned long *cont_linha, const char *valor) {
	// CASO NUMERICO
	if (csv->tipos[var_pos] == 1) {
		double *vetor_comp = (double *)malloc((csv->lin - 1) * sizeof(double));
		double num_valor = atof(valor);
		if (!vetor_comp) {
			printf("Erro de alocação!\n");
			return;
		}

		// passar os numeros para o vetor em float;
		unsigned long k = 0;
		for (unsigned long i = 1; i < csv->lin; i++) {
			vetor_comp[k] = atof(csv->matriz[i][var_pos]);
			k++;
		}
	
		for (unsigned long i = 0; i < csv->lin - 1; i++) {
			if (vetor_comp[i] <  num_valor){
				(*cont_linha)++;
				vetor_lin[i + 1] = 1;
			}
		}

		free(vetor_comp);
		return;
	}
	
	//NAO NUMERICO
	for (unsigned long i = 1; i < csv->lin; i++){
		if (strcmp(csv->matriz[i][var_pos], valor) < 0){
			(*cont_linha)++;
			vetor_lin[i] = 1;
		}
	}
}

void menor_igual (struct arq_csv *csv, char *vetor_lin, long int var_pos, unsigned long *cont_linha, const char *valor) {
	// CASO NUMERICO
	if (csv->tipos[var_pos] == 1) {
		double *vetor_comp = (double *)malloc((csv->lin - 1) * sizeof(double));
		double num_valor = atof(valor);
		if (!vetor_comp) {
			printf("Erro de alocação!\n");
			return;
		}

		// passar os numeros para o vetor em float;
		unsigned long k = 0;
		for (unsigned long i = 1; i < csv->lin; i++) {
			vetor_comp[k] = atof(csv->matriz[i][var_pos]);
			k++;
		}
	
		for (unsigned long i = 0; i < csv->lin - 1; i++) {
			if (vetor_comp[i] <=  num_valor){
				(*cont_linha)++;
				vetor_lin[i + 1] = 1;
			}
		}

		free(vetor_comp);
		return;
	}
	
	//NAO NUMERICO
	for (unsigned long i = 1; i < csv->lin; i++){
		if (strcmp(csv->matriz[i][var_pos], valor) < 0 || strcmp(csv->matriz[i][var_pos], valor) == 0){
			(*cont_linha)++;
			vetor_lin[i] = 1;
		}
	}

}

void diferente (struct arq_csv *csv, char *vetor_lin, long int var_pos, unsigned long *cont_linha, const char *valor) {
	// CASO NUMERICO
	if (csv->tipos[var_pos] == 1) {
		double *vetor_comp = (double *)malloc((csv->lin - 1) * sizeof(double));
		double num_valor = atof(valor);
		if (!vetor_comp) {
			printf("Erro de alocação!\n");
			return;
		}

		// passar os numeros para o vetor em float;
		unsigned long k = 0;
		for (unsigned long i = 1; i < csv->lin; i++) {
			vetor_comp[k] = atof(csv->matriz[i][var_pos]);
			k++;
		}
	
		for (unsigned long i = 0; i < csv->lin - 1; i++) {
			if (vetor_comp[i] != num_valor){
				(*cont_linha)++;
				vetor_lin[i + 1] = 1;
			}
		}

		free(vetor_comp);
		return;
	}

	for (unsigned long i = 1; i < csv->lin; i++){
		if (strcmp(csv->matriz[i][var_pos], valor) != 0){
			(*cont_linha)++;
			vetor_lin[i] = 1;
		}
	}

}

void filtro (char *(var_filtrar)(void), struct arq_csv *csv){
	char variavel[51];
	char valor[51];
	char gravar[2];
	char nome_arquivo[101];
	char descarte[2];
	long int var_pos;
	unsigned long cont_linha;


	printf("Entre com a variavel: ");
	scanf ("%s", variavel);
	getchar();
	
	var_pos = - 1;
	for (unsigned long j = 0; j < csv->col; j++) {
		if (strcmp(csv->matriz[0][j], variavel) == 0) {
			var_pos = j;
			break;
		}
	}
	
	if (var_pos == -1) {
		printf("Variável não encontrada na tabela.\n");
		return;
	}

	// pega qual filtro vai ser utilizado;
	char *fil = var_filtrar();

	printf("Digite um valor: ");
	scanf ("%s", valor);
	getchar();
	
	// Vetor que vai guardar se vai precisar da linha ou nao
	char *vetor_lin = (char *)malloc(csv->lin * sizeof(char));
	if (!vetor_lin) {
		printf("Erro de alocação!\n");
		free(fil);
		return;
	}

	for (unsigned long i = 0; i < csv->lin; i++) {
		vetor_lin[i] = 0;
	}
	
	// sempre precisa do cabeçalho;
	vetor_lin[0] = 1;
	

	// contar quantas linhas serão necessarias
	cont_linha = 0;

/*-----------------------------COMPARAÇÃO-----------------------------------*/
	if (strcmp(fil, "==") == 0)
		igual(csv, vetor_lin, var_pos, &cont_linha, valor);
	else if (strcmp(fil, ">") == 0)
		maior(csv, vetor_lin, var_pos, &cont_linha, valor);
	else if (strcmp(fil, ">=") == 0)
		maior_igual(csv, vetor_lin, var_pos, &cont_linha, valor);
	else if (strcmp(fil, "<") == 0)
		menor(csv, vetor_lin, var_pos, &cont_linha, valor); 
	else if (strcmp(fil, "<=") == 0)
		menor_igual(csv, vetor_lin, var_pos, &cont_linha, valor);
	else if (strcmp(fil, "!=") == 0)
		diferente(csv, vetor_lin, var_pos, &cont_linha, valor);
	else{
		printf("Opção escolhida de filtro é inválida!\n");
		free(fil);
		free(vetor_lin);
		return;
	}
		
/*--------------------------------------------------------------------------*/
	if (cont_linha == 0) {
		printf("Não possui valor ou não encontrado!\n");
		free(fil);
		free(vetor_lin);
		return;
	}

	// para contar com o cabeçalho
	cont_linha++;

	char ***m = criar_matriz_func (cont_linha, csv->col);
	if (!m) {
		printf("Erro de alocação!\n");
		return;
	}
	
	unsigned long k = 0;
	unsigned long i = 0;
	while (i < csv->lin && k < cont_linha) {
		if (vetor_lin[i] == 1) {
			for (unsigned long j = 0; j < csv->col; j++) {
				m[k][j] = csv->matriz[i][j];
			}
			k++;
		}
		i++;
	}
	// vetor para guardar os indices
	unsigned long *vetor_aux = (unsigned long *)malloc(cont_linha * sizeof(unsigned long));
	if (!vetor_aux) {
		printf("Erro de alocação!\n");
		free(fil);
		free(vetor_lin);
		libera_matriz(m, cont_linha, csv->col);
		return;
	}


	i = 0;
	k = 0;
	while (i < csv->lin && k < cont_linha) {
		if (vetor_lin[i] == 1) {
			vetor_aux[k] = i;
			k++;
		}
		i++;
	}
	mostrar(m, cont_linha, csv->col, csv->sizes, vetor_aux);
	
	printf("Deseja gravar um arquivo com os dados ordenados? [S|N] ");
	scanf("%s", gravar);
	getchar();

	// Se escolheu S grava um novo arquivo.
	if (strcmp(gravar, "S") == 0) {
		printf("\nEntre com o nome do arquivo: ");
		scanf("%s", nome_arquivo);
		getchar();

		if (gravar_arq_csv(nome_arquivo, m, cont_linha, csv->col) == 1)
			printf("Arquivo gravado com sucesso\n");
		else
			printf("Erro ao gravar o arquivo!\n");
	}

	else if (strcmp(gravar, "N") != 0 && strcmp(gravar, "S") != 0) {
		printf("Opção escolhida é inválida.\n");
		libera_matriz(m, cont_linha, csv->col);
		free(vetor_aux);
		free(vetor_lin);
		free(fil);
		return;
	}

	printf("\nDeseja descartar os dados originais? [S|N]: ");
	scanf("%s", descarte);
	getchar();

	if (strcmp(descarte, "S") == 0) 
		descarta_dados (csv, m, cont_linha, csv->col);
	
	else if (strcmp(descarte, "S") != 0 && strcmp(descarte, "N") != 0){
		printf("Opção escolhida é inválida.\n");
		libera_matriz(m, cont_linha, csv->col);
		free(vetor_aux);
		free(vetor_lin);
		free(fil);
		return;
	}

	libera_matriz(m, cont_linha, csv->col);
	free(vetor_aux);
	free(vetor_lin);
	free(fil);
		
	printf("\nPressione ENTER para continuar\n");
	getchar();

}

int compara_num (const void *a, const void *b) {
	const float *aa = (const float *)a;
	const float *bb = (const float *)b;

	if (*aa < *bb) return -1;
	if (*aa > *bb) return 1;
	return 0;
}

int compara_string (const void *a, const void *b) {
	return strcmp(*(const char **)a, *(const char **)b);
}

void descricao (struct arq_csv *csv) {
	char variavel[50];
	long int var_pos;
	printf("Entre com a variavel: ");
	scanf ("%s", variavel);
	getchar();

	var_pos = - 1;
	for (unsigned long j = 0; j < csv->col; j++) {
		if (strcmp(csv->matriz[0][j], variavel) == 0) {
			var_pos = j;
			break;
		}
	}

	if (var_pos == -1) {
		printf("Variável não encontrada na tabela.\n");
		return;
	}
	
	unsigned long total_dados = 0;

	for (unsigned long i = 1; i < csv->lin; i++) {
		if (strcmp(csv->matriz[i][var_pos], "") != 0)
			total_dados++;
	}

	printf("Contador: %ld\n", total_dados);

	/*---------------------NUMERICOS-------------------------*/
	if (csv->tipos[var_pos] == 1) {
		float *vetor = (float *)malloc(total_dados * sizeof(float));
		if (!vetor){
			printf("Erro de alocação!\n");
			return;
		}

		unsigned long k = 1;
		unsigned long i = 0;
		while (i < total_dados) {
			if (strcmp(csv->matriz[k][var_pos], "") != 0) {
				vetor[i] = atof(csv->matriz[k][var_pos]);
				i++;
			}
			k++;
		}
		
		// CALCULAR A MEDIA
		float soma = 0;

		for (unsigned long i = 0; i < total_dados; i++)
			soma += vetor[i];

		float media = soma / total_dados;

		printf("Media: %.1f\n", media);

		// CALCULA DESVIO
		float desvio;
		float soma_quadrado = 0;
		for (unsigned long i = 0; i < total_dados; i++)
			soma_quadrado += pow(vetor[i] - media, 2);
		
		desvio = sqrt(soma_quadrado / (total_dados));

		printf("Desvio: %.1f\n", desvio);

		// CALCULA MEDIANA
		qsort(vetor, total_dados, sizeof(float), compara_num);
		float mediana;
		unsigned long meio = total_dados / 2;
		if (total_dados % 2 != 0) {
			mediana = vetor[meio];
		}
		else {
			mediana = (vetor[meio - 1] + vetor[meio]) / 2;
		}
		
		printf("Mediana: %.1f\n", mediana); 

		// CALCULAR A MODA
		float moda = vetor[0];
		int cont_moda = 1;
		float atual = vetor[0];
		int cont_atual = 0;

		i = 1;
		while (i < total_dados) {
			if (vetor[i] == moda) {
				cont_moda++;
				i++;
			}
			else if(vetor[i] == atual) {
				cont_atual++;
				i++;
			}
			else {
				atual = vetor[i];
				cont_atual = 1;
				i++;
			}
			if (cont_atual > cont_moda) {
				moda = atual;
				cont_moda = cont_atual;
			}
		}

		printf("Moda: %.1f %d vezes\n", moda, cont_moda);
		
		// IMPRIMIR MIN, MAX E VALORES UNICOS

		printf("Min.: %.1f\n", vetor[0]);
		printf("Max.: %.1f\n", vetor[total_dados - 1]);

		printf("Valores unicos: [%.1f", vetor[0]);
		for (unsigned long i = 1; i < total_dados; i++) {
			if (vetor[i-1] != vetor[i])
				printf(", %.1f", vetor[i]);
		}
		printf("]\n");
		free(vetor);
		printf("\nPressione ENTER para continuar\n");
		getchar();
		return;
	}
	/*---------------------NAO-NUMERICOS-------------------------*/
	char **vetor = (char **)malloc(total_dados * sizeof(char *));
	if (!vetor) {
		printf("Erro de alocação!\n");
		return;
	}

	unsigned long k = 1;
	unsigned long i = 0;
	while (i < total_dados) {
		if (strcmp(csv->matriz[k][var_pos], "") != 0){
			vetor[i] = csv->matriz[k][var_pos];
			i++;
		}
		k++;
	}
	
	// ACHAR A MODA
	qsort(vetor, total_dados, sizeof(char *), compara_string);
	
	char *moda = vetor[0];
	int cont_moda = 1;
	char *atual = vetor[0];
	int cont_atual = 0;
	i = 1;
	while (i < total_dados) {
		if (strcmp(vetor[i], moda) == 0) {
			cont_moda++;
			i++;
		}
		else if (strcmp(vetor[i], atual) == 0) {
			cont_atual++;
			i++;
		}
		else  {
			atual = vetor[i];
			cont_atual = 1;
			i++;
		}
		if (cont_atual > cont_moda) {
			moda = atual;
			cont_moda = cont_atual;
		}
	}

	printf("Moda    : %s %d vezes\n", moda, cont_moda);

	// VALORES UNICOS
	printf("Valores unicos: ['%s'", vetor[0]);
	for (unsigned long i = 1; i < total_dados; i++) {
		if (strcmp(vetor[i-1], vetor[i]) != 0)
			printf(", '%s'", vetor[i]);
	}
	printf("]\n");
	free(vetor);
	printf("\nPressione ENTER para continuar\n");
	getchar();
}

void libera_matriz (char ***m, unsigned long lin, unsigned col) {
	
	if (m != NULL) {
		for (unsigned long i = 0; i < lin; i++) {
			if (m[i]) {
				for (unsigned long j = 0; j < col; j++) {
					m[i][j] = NULL;
				}
				free(m[i][0]);
				free(m[i]);
			}
		}
		free(m);
	}
}

int compara_struct_string_Asc (const void *a, const void *b) {
	struct elemento_col *aa = (struct elemento_col *)a;
	struct elemento_col *bb = (struct elemento_col *)b;

	if (strcmp(aa->elemento,"") == 0)
		return -1;
	if (strcmp(bb->elemento,"") == 0)
		return 1;
	return strcmp(aa->elemento, bb->elemento);
}

int compara_struct_string_Desc (const void *a, const void *b) {
	return -1 * (compara_struct_string_Asc(a,b));
}

int compara_struct_numerico_Asc (const void *a, const void *b) {
	struct elemento_col *aa = (struct elemento_col *)a;
	struct elemento_col *bb = (struct elemento_col *)b;
	
	// converter string para float
	if (strcmp(aa->elemento, "") == 0)
		return -1;
	if (strcmp(bb->elemento, "") == 0)
		return 1;

	float num_aa = atof(aa->elemento);
	float num_bb = atof(bb->elemento);

	if (num_aa > num_bb)
		return 1;
	else if (num_bb > num_aa)
		return -1;
	else 
		return 0;
}

int compara_struct_numerico_Desc (const void *a, const void *b) {
	return -1 * (compara_struct_numerico_Asc(a, b));
}

int gravar_arq_csv (const char *arquivo, char ***matriz, unsigned long lin, unsigned col) {
	FILE *csv_arq = fopen(arquivo, "w");
	if (!csv_arq) {
		printf("Erro ao abrir o arquivo!\n");
		return 0;
	}
	
	for (unsigned long i = 0; i < lin; i++) {
		for (unsigned long j = 0; j < col; j++) {
			//escreve elemento no arquivo;
			fprintf(csv_arq, "%s", matriz[i][j]);
			
			//Não colocar virgula no final da linha;
			if (j < col - 1) 
				fputc(',', csv_arq);
		}
			fputc('\n', csv_arq);
	}

	fclose(csv_arq);
	return 1;
}

// Libera a outra matriz e cria outra, passa os dados da nova para antiga;
void descarta_dados (struct arq_csv *csv, char ***matriz, unsigned long lin, unsigned long col) {
	libera_matriz(csv->matriz, csv->lin, csv->col);
	cria_matriz(csv, lin, col);
	for (unsigned long i = 0; i < lin; i++){
		for (unsigned long j = 0; j < col; j++)
			csv->matriz[i][j] = matriz[i][j];
	}
}

char ***criar_matriz_func (unsigned long lin, unsigned long col) {
	char ***m = (char ***)malloc(lin * sizeof(char**));
	if (!m) {
		printf("Erro de alocação!: %s\n", strerror (errno));
		return NULL;
	}

	for (unsigned long i = 0; i < lin; i++) {
		m[i] = (char**) malloc(col * sizeof(char *));
		if (!m[i]) {
			if (i > 0) {
				for (unsigned long j = 0; j < i; j++) {
					free(m[j]);
				}
			}
			free(m);
			return NULL;
		}
	}
	return m;
}

void ordenacao (struct arq_csv *csv) {
	char variavel[50];
	char opcao[2];
	char gravar[2];
	char descarte[2];
	char nome_arquivo[101];
	long int var_pos;
	
	struct elemento_col *vetor = (struct elemento_col *)malloc((csv->lin - 1) * sizeof(struct elemento_col));
	if (!vetor){
		printf("Erro de alocação!\n");
		return;
	}

	printf("Entre com a variavel: ");
	scanf ("%s", variavel);
	getchar();

	var_pos = - 1;
	for (unsigned long j = 0; j < csv->col; j++) {
		if (strcmp(csv->matriz[0][j], variavel) == 0) {
			var_pos = j;
			break;
		}
	}

	if (var_pos == -1) {
		printf("Variável não encontrada na tabela.\n");
		free(vetor);
		return;
	}
	
	printf("Selecione uma opcao [A]scendente ou [D]escrescente: ");
	scanf("%s", opcao);
	getchar();

	// Passa a coluna selecionada para o vetor
	unsigned long k = 1;
	for (unsigned long i = 0; i < csv->lin - 1; i++) {
		vetor[i].pos = k;
		vetor[i].elemento = csv->matriz[k][var_pos];
		k++;
	}
		
	
	// FAZER CASO NAO NUMERICO
	if (csv->tipos[var_pos] == 0) { 
		//Ordenar vetor;
		if (strcmp(opcao, "A") == 0) {
			qsort(vetor, (csv->lin - 1), sizeof(struct elemento_col), compara_struct_string_Asc);
		}
		else if (strcmp(opcao, "D") == 0){
			qsort(vetor, (csv->lin - 1), sizeof(struct elemento_col), compara_struct_string_Desc);
		}
		else {
			printf("Opção inválida!");
			free(vetor);
			return;
		}
	}
	
	// CASO NUMERICO
	else {
		// Ordenar vetor;	
		if (strcmp(opcao, "A") == 0) {
			qsort(vetor, (csv->lin - 1), sizeof(struct elemento_col), compara_struct_numerico_Asc);
		}
		else if (strcmp(opcao, "D") == 0){
			qsort(vetor, (csv->lin - 1), sizeof(struct elemento_col), compara_struct_numerico_Desc);
		}
		else {
			printf("Opção inválida!");
			free(vetor);
			return;
		}
	}
	//criar nova matriz para imprimir;	
	char ***m = criar_matriz_func(csv->lin, csv->col);
	if (!m) {
		printf("Erro de alocação!\n");
		free(vetor);
		return;
	}

	//passa cabeçalho para nova matriz
	for (unsigned long j = 0; j < csv->col; j++) {
		m[0][j] = csv->matriz[0][j];
	}

	// preencher o resto da matriz
	k = 0;
	for (unsigned long i = 1; i < csv->lin; i++) {
		for (unsigned long j = 0; j < csv->col; j++) {
			m[i][j] = csv->matriz[vetor[k].pos][j];
		}
		k++;
	}
	// Vetor auxiliar para imprimir os indices
	unsigned long *vetor_aux = (unsigned long *) malloc(csv->lin * sizeof(unsigned long));
	if (!vetor_aux){
		printf("Erro de alocação!\n");
		free(vetor);
		libera_matriz(m, csv->lin, csv->col);
		return;
	}

	vetor_aux[0] = 0;
	k = 0;
	for (unsigned long i = 1; i < csv->lin; i++){
		vetor_aux[i] = vetor[k].pos;
		k++;
	}

	mostrar(m, csv->lin, csv->col, csv->sizes, vetor_aux);
	
	printf("Deseja gravar um arquivo com os dados ordenados? [S|N] ");
	scanf("%s", gravar);
	getchar();

	// Se escolheu S grava um novo arquivo.
	if (strcmp(gravar, "S") == 0) {
		printf("\nEntre com o nome do arquivo: ");
		scanf("%s", nome_arquivo);
		if (gravar_arq_csv(nome_arquivo, m, csv->lin, csv->col) == 1)
			printf("Arquivo gravado com sucesso\n");
		else
			printf("Erro ao gravar o arquivo!\n");
	}

	else if (strcmp(gravar, "N") != 0 && strcmp(gravar, "S") != 0) {
		printf("Opção escolhida é inválida.\n");
		free(vetor_aux);
		free(vetor);
		libera_matriz(m, csv->lin, csv->col);
		return;
	}

	printf("\nDeseja descartar os dados originais? [S|N]: ");
	scanf("%s", descarte);
	getchar();

	if (strcmp(descarte, "S") == 0) 
		descarta_dados (csv, m, csv->lin, csv->col);
	
	else if (strcmp(descarte, "S") != 0 && strcmp(descarte, "N") != 0){
		printf("Opção escolhida é inválida.\n");
		free(vetor_aux);
		free(vetor);
		libera_matriz(m, csv->lin, csv->col);
		return;
	}

	libera_matriz(m, csv->lin, csv->col);
	free(vetor_aux);
	free(vetor);
		
	printf("\nPressione ENTER para continuar\n");
	getchar();
}

void selecao (struct arq_csv *csv){
	char variaveis[1024];
	char gravar[2];
	char nome_arquivo[101];
	// Vetor que guarda se vai ou nao guardar a coluna;
	char *vetor = (char *)malloc(csv->col * sizeof(char));
	char *token;
	unsigned long cont_var = 0;

	//Zerar todo o vetor;
	for (unsigned long i = 0; i < csv->col; i++)
		vetor[i] = 0;
		

	printf("Entre com a variaveis que deseja selecionar (separadas por espaço): ");
	fgets(variaveis, 1024, stdin);

	if (variaveis[strlen(variaveis) - 1] == '\n')
		variaveis[strlen(variaveis) - 1] = '\0';

	token = strtok(variaveis, " ");
	while (token != NULL) {
		for (unsigned long j = 0; j < csv->col; j++){
			if (strcmp(token, csv->matriz[0][j]) == 0) {
				vetor[j] = 1;
				cont_var++;
				break;
			}
		}
		token = strtok(NULL, " ");
	}
	
	if (cont_var == 0) {
		printf("Variável não encontrada na tabela.\n");
		free(vetor);
		return;
	}

	// Cria a matriz para as colunas selecionadas
	char ***m = criar_matriz_func(csv->lin, cont_var);

	// pega as colunas selecionas e coloca em uma matriz;
	unsigned long j = 0;
	unsigned long k = 0;
	while (j < cont_var && k < csv->col) {
		if (vetor[k] == 1) {
			for (unsigned long i = 0; i < csv->lin; i++) {
				m[i][j] = csv->matriz[i][k];
			}
			j++;
		}
		k++;
	}

	// vetor para armazenar o maior tamanho da palavra das colunas;
	unsigned short *v_tam = (unsigned short *)malloc(cont_var * sizeof(unsigned short));

	unsigned long i = 0;
	for (unsigned long j = 0; j < csv->col; j++) {
		if (vetor[j] == 1) {
			v_tam[i] = csv->sizes[j];
			i++;
		}
	}
	
	// Vetor para armazenar indices para impressao
	unsigned long *v_aux = (unsigned long *) malloc(csv->lin * sizeof(unsigned long));
	for(unsigned long i = 0; i < csv->lin; i++) {
		v_aux[i] = i;
	}

	mostrar(m, csv->lin, cont_var, v_tam, v_aux);


	printf("Deseja gravar um arquivo com os dados ordenados? [S|N] ");
	scanf("%s", gravar);
	getchar();

	// Se escolheu S grava um novo arquivo.
	if (strcmp(gravar, "S") == 0) {
		printf("Entre com o nome do arquivo: ");
		scanf("%s", nome_arquivo);
		if (gravar_arq_csv(nome_arquivo, m, csv->lin, cont_var) == 1)
			printf("Arquivo gravado com sucesso\n");
	}
	
	else if (strcmp(gravar, "S") != 0 && strcmp(gravar, "N") != 0) {
		printf("Opção escolhida é inválida.\n");
		libera_matriz(m, csv->lin, cont_var);
		free(vetor);
		free(v_tam);
		free(v_aux);
		return;
	}
	
	libera_matriz(m, csv->lin, cont_var);
	free(vetor);
	free(v_tam);
	free(v_aux);
	
	printf("\nPressione ENTER para continuar\n");
	getchar();
}

void listar_com_nulo (struct arq_csv *csv) {
	char gravar[2];
	char descarte[2];
	char nome_arquivo[100];
	
	unsigned long cont_nulo;

	// Numero de linhas que possui NaN, começa com 1 por causa do cabeçalho
	cont_nulo = 1;
	for (unsigned long i = 1; i < csv->lin; i++) {
		for(unsigned long j = 0; j < csv->col; j++) {
			if (strcmp(csv->matriz[i][j], "") == 0) {
				cont_nulo++;
				break;
			}
		}
	}

	// olhar o caso em q nao tem nan
	if (cont_nulo == 1) {
		printf("Empty DataFrame\n");
		printf("[%s", csv->matriz[0][0]);
		for (unsigned long j = 1; j < csv->col; j++)
			printf(", %s", csv->matriz[0][j]);
		printf("]\n");
		printf("Index: []\n\n");
		
		return;
	
	}
	// criar vetor com a quatidade das linhas que possui NaN para guardar os indices
	unsigned long *vetor_ind = (unsigned long *)malloc(cont_nulo * sizeof(unsigned long));

	// passar cada valor de linha para o vetor
	vetor_ind[0] = 0;
	unsigned long k = 1;
	for (unsigned long i = 1; i < csv->lin; i++) {
		for(unsigned long j = 0; j < csv->col; j++) {
			if (strcmp(csv->matriz[i][j], "") == 0) {
				vetor_ind[k] = i;
				k++;
				break;
			}
		}
	}


	// Criar matriz com a quantidade dessas linhas
	char ***m = criar_matriz_func(cont_nulo, csv->col);


	// preencher a matriz
	for (unsigned long i = 0; i < cont_nulo; i++) {
		for (unsigned long j = 0; j < csv->col; j++) {
			m[i][j] = csv->matriz[vetor_ind[i]][j];
		}
	}
	
	mostrar(m, cont_nulo, csv->col, csv->sizes, vetor_ind);
	
	printf("Deseja gravar um arquivo com os dados alterados? [S|N] ");
	scanf("%s", gravar);

	// Se escolheu S grava um novo arquivo.
	if (strcmp(gravar, "S") == 0) {
		printf("\nEntre com o nome do arquivo: ");
		scanf("%s", nome_arquivo);
		if (gravar_arq_csv(nome_arquivo, m, cont_nulo, csv->col) == 1)
			printf("Arquivo gravado com sucesso\n");
		else
			printf("Erro ao gravar o arquivo!\n");
	}

	else if (strcmp(gravar, "N") != 0 && strcmp(gravar, "S") != 0) {
		printf("Opção escolhida é inválida.\n");
		libera_matriz(m, cont_nulo, csv->col);
		free(vetor_ind);
		return;
	}

	printf("\nDeseja descartar os dados originais? [S|N]: ");
	scanf("%s", descarte);

	if (strcmp(descarte, "S") == 0) 
		descarta_dados (csv, m, cont_nulo, csv->col);
	
	else if (strcmp(descarte, "N") != 0 && strcmp(descarte, "S") != 0) {
		printf("Opção escolhida é inválida.\n");
		libera_matriz(m, cont_nulo, csv->col);
		free(vetor_ind);
		return;
	}
	libera_matriz(m, cont_nulo, csv->col);
	free(vetor_ind);
}

void substituir_nulo_media (struct arq_csv *csv, struct lista *l){
	char *copia;
	double soma;	
	unsigned long total_dados;
	for (unsigned long j = 0; j < csv->col; j++){
		soma = 0;
		total_dados = 0;
		if (csv->tipos[j] == 1){
			for (unsigned long i = 1; i < csv->lin; i++) {
				if (strcmp(csv->matriz[i][j], "") != 0)
					total_dados++;
			}
			
			if (total_dados == csv->lin - 1) {
				printf("Empty DataFrame\n");
				printf("[%s", csv->matriz[0][0]);
				for (unsigned long j = 1; j < csv->col; j++)
					printf(", %s", csv->matriz[0][j]);
				printf("]\n");
				printf("Index: []\n\n");
				
				return;
			}

			double *vetor = (double *)malloc(total_dados * sizeof(double));
			if (!vetor) {
				printf("Erro ao alocar memória!\n");
				return;
			}
			unsigned long k = 1;
			unsigned long i = 0;
			while (i < total_dados) {
				if (strcmp(csv->matriz[k][j], "") != 0) {
					vetor[i] = atof(csv->matriz[k][j]);
					i++;
				}
				k++;
			}
			//fazer a soma dos elementos e achar a media
			for (unsigned long i = 0; i < total_dados; i++)
				soma += vetor[i];
			
			char buffer[257];

			double media = soma / total_dados;
			
			sprintf(buffer, "%.2f", media);
			
			if(buffer[strlen(buffer) - 1] == '\n')
				buffer[strlen(buffer) - 1] = '\0';
			
			copia = strdup(buffer);
			
			for (unsigned long i = 1; i < csv->lin; i++) {
				if (strcmp(csv->matriz[i][j], "") == 0) {
					csv->matriz[i][j] = copia;
				}
			}

			//inserir na lista para dar free;
			insere_na_lista(copia, l);
			free(vetor);
		}

	}
}

void substituir_prox (struct arq_csv *csv) {
	unsigned long i, k;
	// i controla a linha e k controla o que vai ser substituido;
	for (unsigned long j = 0; j < csv->col; j++) {
		i = 1;
		k = i + 1;
		while (i < csv->lin - 1) {
			if (strcmp(csv->matriz[i][j], "") == 0) {
				if (strcmp(csv->matriz[k][j], "") != 0) {
					csv->matriz[i][j] = csv->matriz[k][j];
					i++;
					k = i + 1;
				}
				else 
					k++;
			}
			else {
				i++;
				k++;
			}
			// nenhum outro valor valido encontrado
			if (k == csv->lin - 1 && strcmp(csv->matriz[k][j], "") == 0)
				break;
		}
	}
}

void excluir_faltantes (struct arq_csv *csv) {
	char variaveis[1024];

	// Vetor que guarda se vai ou nao mudar a coluna;
	char *vetor = (char *)malloc(csv->col * sizeof(char));
	char *token;
	unsigned long cont_var = 0;

	//Zerar todo o vetor;
	for (unsigned long i = 0; i < csv->col; i++)
		vetor[i] = 0;
		
	printf("Entre com a variaveis que deseja selecionar (separadas por espaço): ");
	fgets(variaveis, 1024, stdin);

	if (variaveis[strlen(variaveis) - 1] == '\n')
		variaveis[strlen(variaveis) - 1] = '\0';

	token = strtok(variaveis, " ");
	while (token != NULL) {
		for (unsigned long j = 0; j < csv->col; j++){
			if (strcmp(token, csv->matriz[0][j]) == 0) {
				vetor[j] = 1;
				cont_var++;
				break;
			}
		}
		token = strtok(NULL, " ");
	}
	
	if (cont_var == 0) {
		printf("Variável não encontrada na tabela.\n");
		free(vetor);
		return;
	}
	
	unsigned long *vetor_selec = (unsigned long *)malloc(cont_var * sizeof(unsigned long));

	unsigned long i = 0;
	for (unsigned long j = 0; j < csv->col; j++) {
		if (vetor[j] == 1){
			vetor_selec[i] = j;
			i++;
		}
	}

	char *vetor_lin = (char *)malloc(csv->lin * sizeof(char));

	for (unsigned long i = 0; i < csv->lin; i++)
		vetor_lin[i] = 0;

	unsigned long cont_nao_nulo = 0;
	unsigned long j;
	for (unsigned long i = 0; i < csv->lin; i++) {
		for (j = 0; j < cont_var; j++){
			if (strcmp(csv->matriz[i][vetor_selec[j]], "") == 0)
				break;
		}
		if (j == cont_var) {
			cont_nao_nulo++;
			vetor_lin[i] = 1;
		}
	}

	if (cont_nao_nulo == csv->lin) {
		printf("Empty DataFrame\n");
		printf("[%s", csv->matriz[0][0]);
		for (unsigned long j = 1; j < csv->col; j++)
			printf(", %s", csv->matriz[0][j]);
		printf("]\n");
		printf("Index: []\n\n");
		free(vetor);
		free(vetor_lin);
		free(vetor_selec);
		return;
		
	}

	char ***m = criar_matriz_func(cont_nao_nulo, csv->col);
	
	unsigned long k = 0;
	i = 0;
	while (i < csv->lin && k < cont_nao_nulo) {
		if (vetor_lin[i] == 1){
			for(unsigned long j = 0; j < csv->col; j++){
				m[k][j] = csv->matriz[i][j];
			}
			k++;
		}
		i++;
	}
	
	descarta_dados(csv, m, cont_nao_nulo, csv->col);
	libera_matriz(m, cont_nao_nulo, csv->col);
	free(vetor);
	free(vetor_selec);
	free(vetor_lin);

}

void dados_faltantes (struct arq_csv *csv, struct lista *l) {
	int opcao;

	opcao = 0;
	while (opcao != 5){
		printf("     1) Listar registros com NaN\n");
		printf("     2) Substituir pela media\n");
		printf("     3) Substituir pelo proximo valor valido\n");
		printf("     4) Remover registros com NaN\n");
		printf("     5) Voltar ao menu principal\n");
		
		printf("Digite sua opcao: ");
		scanf("%d", &opcao);
		getchar();

		switch (opcao) {
			case 1:
				listar_com_nulo(csv);
				break;
			case 2:
				substituir_nulo_media(csv, l);
				break;
			case 3:
				substituir_prox(csv);
				break;
			case 4:
				excluir_faltantes(csv);
				break;
			case 5:
				break;
			default:
				printf("Opção inválida!\n");
		}
	}
	printf("\nPressione ENTER para continuar\n");
	getchar();
}

void gravar_em_disco (struct arq_csv *csv) {
	char arquivo[101];
	
	printf("Entre com o nome do arquivo: ");
	scanf("%s", arquivo);
	getchar();

	FILE *csv_arq = fopen(arquivo, "w");
	if (!csv_arq) {
		printf("Erro ao abrir o arquivo!\n");
		return;
	}

	
	for (unsigned long i = 0; i < csv->lin; i++) {
		for (unsigned long j = 0; j < csv->col; j++) {
			//escreve elemento no arquivo;
			fprintf(csv_arq, "%s", csv->matriz[i][j]);
			
			//Não colocar virgula no final da linha;
			if (j < csv->col - 1) 
				fputc(',', csv_arq);
		}
			fputc('\n', csv_arq);
	}

	fclose(csv_arq);
	printf("Arquivo gravado com sucesso\n");
	printf("\nPressione ENTER para continuar\n");
	getchar();
}

void fecha (struct arq_csv *csv) {
	if (!csv)

		return;
	if (csv->arquivo)
		fclose(csv->arquivo);
		
	if (csv->matriz != NULL) {
		for (unsigned long i = 0; i < csv->lin; i++) {
			if (csv->matriz[i]) {
				
				for (unsigned long j = 0; j < csv->col; j++) {
					csv->matriz[i][j] = NULL;
					free(csv->matriz[i][j]);
				}
		
				free(csv->matriz[i]);
			}
		}
		free(csv->matriz);
	}
	
	free(csv->tipos);
	free(csv->sizes);
	free(csv);
}
