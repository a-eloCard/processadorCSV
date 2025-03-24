#include "io.h"

int main (int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Uso: ./csvreader <funcionarios.csv>\n");
	}
	if (!argv[1])
		return 1;
	
	char *arquivo = argv[1];
	
	struct arq_csv *csv = abre(arquivo);
	
	if (!csv) {
		fecha(csv);
		return 1;
	}
	
	struct lista *lista = inicializa_lista();
	preenche_matriz (csv, lista);
	if (verifica_valido(csv) == 0) {
		printf("Arquivo inválido!\n");
		libera_lista(lista);
		fecha(csv);
		return 1;
	}
	atribui_struct(csv);
	char *(*filtrar)(void);
	
	filtrar = &var_filtrar;
	int opcao = 0;
	while (opcao != 9) {
		menu();
		printf("Digite sua opcao: ");
		scanf("%d", &opcao);
		getchar();

		switch (opcao) {
			case 1: 
				sumario(*csv);
				break;
			case 2:
				mostra_da_opcao(*csv);
				break;
			case 3:
				filtro(filtrar, csv);
				break;
			case 4:
				descricao(csv);
				break;
			case 5:
				ordenacao(csv);
				break;
			case 6:
				selecao(csv);
				break;
			case 7:
				dados_faltantes(csv, lista); 
				break;
			case 8:
				gravar_em_disco(csv);
				break;
			case 9:
				break;
			default:
				printf("Opção inválida!\n");
		}
	}
	libera_lista(lista);
	fecha(csv);
	return 0;
}
