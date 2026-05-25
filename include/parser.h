#ifndef PARSER_H
#define PARSER_H

#include "../include/AST.h"
#include "../include/LSE.h"
#include "../include/token.h"

typedef struct {
  NO *atual;
} Parser;

void iniciar_parser(Parser *p, Lista *tokens);

AST *executar_analise_sintatica(Parser *p, Lista *tokens);

// HIDE
AST *parse_declaracao_ou_funcao(Parser *p);
AST *parse_funcao(Parser *p, Token tipo, Token nome);
AST *parse_lista_params(Parser *p);
AST *parse_param(Parser *p);
AST *parse_comando(Parser *p);
AST *parse_bloco(Parser *p);
AST *parse_dec(Parser *p);
AST *parse_cmd_ident(Parser *p);
AST *parse_att(Parser *p, Token ident);
AST *parse_incr_decr(Parser *p, Token ident);
AST *parse_cond(Parser *p);
AST *parse_rep(Parser *p);
AST *parse_for(Parser *p);
AST *parse_cmd_for(Parser *p);
AST *parse_return(Parser *p);
AST *parse_expr_logica(Parser *p);
AST *parse_expr_rel(Parser *p);
AST *parse_expressao(Parser *p);
AST *parse_termo(Parser *p);
AST *parse_fator(Parser *p);

#endif