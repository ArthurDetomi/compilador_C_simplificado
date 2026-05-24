//====================================================
// ast.c
//====================================================

#include "../include/AST.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//====================================================
// CRIAR NÓ
//====================================================

AST *criar_no_ast(ASTTipo tipo, Token token) {

  AST *no = (AST *)malloc(sizeof(AST));

  if (!no) {
    perror("Erro malloc AST");
    exit(1);
  }

  no->tipo = tipo;

  no->token = token;

  no->esquerda = NULL;
  no->direita = NULL;

  no->filhos = NULL;
  no->num_filhos = 0;

  return no;
}

//====================================================
// ADICIONAR FILHO
//====================================================

void adicionar_filho(AST *pai, AST *filho) {

  if (!pai || !filho)
    return;

  pai->num_filhos++;

  pai->filhos = realloc(pai->filhos, sizeof(AST *) * pai->num_filhos);

  pai->filhos[pai->num_filhos - 1] = filho;
}

//====================================================
// NOME DO TIPO
//====================================================

const char *nome_ast(ASTTipo tipo) {

  switch (tipo) {

  case AST_PROGRAMA:
    return "PROGRAMA";

  case AST_BLOCO:
    return "BLOCO";

  case AST_DECLARACAO:
    return "DECLARACAO";

  case AST_ATRIBUICAO:
    return "ATRIBUICAO";

  case AST_IF:
    return "IF";

  case AST_WHILE:
    return "WHILE";

  case AST_BINOP:
    return "BINOP";

  case AST_LITERAL:
    return "LITERAL";

  case AST_IDENTIFICADOR:
    return "IDENTIFICADOR";

  case AST_TIPO:
    return "TIPO";
  }

  return "DESCONHECIDO";
}

//====================================================
// IMPRIMIR AST
//====================================================

void imprimir_ast(AST *raiz, int nivel) {

  if (!raiz)
    return;

  for (int i = 0; i < nivel; i++) {
    printf("  ");
  }

  printf("%s", nome_ast(raiz->tipo));

  if (raiz->token.lexema != NULL) {

    printf(" (%s)", raiz->token.lexema);
  }

  printf("\n");

  imprimir_ast(raiz->esquerda, nivel + 1);

  imprimir_ast(raiz->direita, nivel + 1);

  for (int i = 0; i < raiz->num_filhos; i++) {

    imprimir_ast(raiz->filhos[i], nivel + 1);
  }
}

//====================================================
// LIBERAR AST
//====================================================

void liberar_ast(AST *raiz) {

  if (!raiz)
    return;

  liberar_ast(raiz->esquerda);

  liberar_ast(raiz->direita);

  for (int i = 0; i < raiz->num_filhos; i++) {

    liberar_ast(raiz->filhos[i]);
  }

  free(raiz->filhos);

  free(raiz);
}