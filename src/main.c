#include "../include/AST.h"
#include "../include/LSE.h"
#include "../include/parser.h"
#include "../include/scanner.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    perror("ERROR: arquivo de entrada não informado!");
    exit(1);
  }

  Lista *lista_tokens = executar_analise_lexica(argv[1]);

  exibir_tokens(lista_tokens);

  Parser p;

  iniciar_parser(&p, lista_tokens);

  AST *ast = parse_programa(&p);

  imprimir_ast(ast, 0);

  destroiLista(lista_tokens);

  return 0;
}