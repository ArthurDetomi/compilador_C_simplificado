#ifndef AST_H
#define AST_H

#include "token.h"

typedef enum {
  AST_PROGRAMA,
  AST_BLOCO,
  AST_DECLARACAO,
  AST_ATRIBUICAO,
  AST_IF,
  AST_WHILE,
  AST_FOR,
  AST_BINOP,
  AST_UNOP,
  AST_LITERAL,
  AST_IDENTIFICADOR,
  AST_TIPO,
  AST_FUNCAO,
  AST_PARAMS,
  AST_DECLARADOR,
  AST_PARAM,
  AST_RETURN,
} ASTTipo;

typedef struct AST {
  ASTTipo tipo;
  Token token;
  struct AST *esquerda;
  struct AST *direita;
  struct AST **filhos;
  int num_filhos;
} AST;

AST *criar_no_ast(ASTTipo tipo, Token token);

void adicionar_filho(AST *pai, AST *filho);

void imprimir_ast(AST *raiz, int nivel);

void liberar_ast(AST *raiz);

#endif