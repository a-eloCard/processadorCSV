#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#ifndef __IO__
#define __IO__

struct arq_csv {
	FILE *arquivo;
	char *tipos; // tipos de cada coluna
	unsigned long lin; // numero de linhas
	unsigned long col; // numero de colunas
	unsigned short  *sizes; // tamanhos das maiores palavras
	char ***matriz;
};
// lista para dar free nas memorias criadas durante o arquivo
struct item {
	char *valor;
	struct item *proximo;
};

struct lista {
	struct item *head;
	struct item *cauda;
	int tamanho;
};

struct elemento_col {
	unsigned long pos;
	char *elemento;
};
/*-------------------LISTA SIMPLES-------------------*/
struct lista *inicializa_lista ();

struct item *cria_item (char *media);

void insere_na_lista (char *media, struct lista *l);

void libera_lista (struct lista *l);
/*---------------------------------------------------*/

/*---------------------PARA A STRUCT-----------------*/
unsigned long int conta_linha (FILE *file);

unsigned long int conta_coluna (FILE *file);

void cria_matriz (struct arq_csv *csv, unsigned long lin, unsigned long col);

struct arq_csv *abre (char *path);

char *separa (const char *string);

void menu ();

void preenche_matriz (struct arq_csv *csv, struct lista *l);

int verifica_valido (struct arq_csv *csv);

int eh_numerica (const char *string);

void atribui_struct (struct arq_csv *csv);

/*----------------------------------------------------*/
/*--------------------PARA MOSTRAR--------------------*/
void sumario (struct arq_csv csv);

int tam_indice (unsigned long num);

void mostra_da_opcao (struct arq_csv csv);

void mostrar (char ***matriz, unsigned long lin, unsigned long col, unsigned short *v_sizes, unsigned long *v_aux);
/*-----------------------------------------------------*/

void descarta_dados (struct arq_csv *csv, char ***matriz, unsigned long lin, unsigned long col);

/*-------------------------PARA FILTRAR-----------------*/
char *var_filtrar ();

void igual (struct arq_csv *csv, char *vetor_lin, long int var_pos, unsigned long *cont_linha, const char *valor);

void maior (struct arq_csv *csv, char *vetor_lin, long int var_pos, unsigned long *cont_linha, const char *valor);

void maior_igual (struct arq_csv *csv, char *vetor_lin, long int var_pos, unsigned long *cont_linha, const char *valor);

void menor (struct arq_csv *csv, char *vetor_lin, long int var_pos, unsigned long *cont_linha, const char *valor);

void menor_igual (struct arq_csv *csv, char *vetor_lin, long int var_pos, unsigned long *cont_linha, const char *valor);

void diferente (struct arq_csv *csv, char *vetor_lin, long int var_pos, unsigned long *cont_linha, const char *valor);

void filtro (char *(var_filtrar)(void), struct arq_csv *csv);
/*-------------------------------------------------------*/

int compara_num (const void *a, const void *b);

int compara_string (const void *a, const void *b);

void descricao (struct arq_csv *csv);

void libera_matriz (char ***m, unsigned long lin, unsigned col);

int compara_struct_string_Asc (const void *a, const void *b);

int compara_struct_string_Desc (const void *a, const void *b);

int compara_struct_numerico_Asc (const void *a, const void *b);

int compara_struct_numerico_Desc (const void *a, const void *b);

int gravar_arq_csv (const char *arquivo, char ***matriz, unsigned long lin, unsigned col);

char ***criar_matriz_func (unsigned long lin, unsigned long col);


void ordenacao (struct arq_csv *csv);

void selecao (struct arq_csv *csv);

//----------------DADOS FALTANTES-------------------------------------*/
void listar_com_nulo (struct arq_csv *csv);

void substituir_nulo_media (struct arq_csv *csv, struct lista *l);

void substituir_prox (struct arq_csv *csv);

void excluir_faltantes (struct arq_csv *csv);

void dados_faltantes (struct arq_csv *csv, struct lista *l);

/*-------------------GRAVAR EM DISCO----------------------------------*/
void gravar_em_disco (struct arq_csv *csv);

void fecha (struct arq_csv *csv);

#endif
