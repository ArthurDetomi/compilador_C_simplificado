// parser.h

#ifndef PARSER_H
#define PARSER_H

#include "../include/AST.h"
#include "../include/LSE.h"
#include "../include/token.h"

typedef struct {
  NO *atual;
} Parser;

// Inicialização
void iniciar_parser(Parser *p, Lista *tokens);

// Programa
AST *parse_programa(Parser *p);

#endif