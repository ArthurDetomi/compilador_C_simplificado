//====================================================
// parser.c
//====================================================

#include "../include/parser.h"
#include "../include/AST.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//====================================================
// CONTADOR DE ERROS
//====================================================

int erros_sintaticos = 0;

//====================================================
// UTILITÁRIOS
//====================================================

Token *token_atual(Parser *p) {

  if (p->atual == NULL)
    return NULL;

  return &p->atual->info;
}

void avancar(Parser *p) {

  if (p->atual != NULL)
    p->atual = p->atual->prox;
}

//====================================================
// ERRO
//====================================================

void erro_sintatico(const char *msg, Token *tk) {

  erros_sintaticos++;

  if (tk == NULL) {

    printf("ERRO SINTATICO: %s\n", msg);

    return;
  }

  printf("ERRO SINTATICO: %s na linha %d coluna %d token '%s'\n", msg,
         tk->pos.linha, tk->pos.coluna, tk->lexema);
}

//====================================================
// PANIC MODE
//====================================================

void sincronizar(Parser *p) {

  Token *tk = token_atual(p);

  while (tk != NULL && tk->type != EOF_TOKEN) {

    if (tk->type == SEP_PONTO_VIRGULA || tk->type == SEP_FECHA_CHAVE) {

      avancar(p);
      return;
    }

    avancar(p);

    tk = token_atual(p);
  }
}

//====================================================
// MATCH
//====================================================

int match(Parser *p, TokenType type) {

  Token *tk = token_atual(p);

  if (tk != NULL && tk->type == type) {

    avancar(p);

    return 1;
  }

  return 0;
}

//====================================================
// CONSUMIR
//====================================================

int consumir(Parser *p, TokenType type, const char *erro) {

  if (match(p, type))
    return 1;

  erro_sintatico(erro, token_atual(p));

  sincronizar(p);

  return 0;
}

//====================================================
// PALAVRAS RESERVADAS
//====================================================

int eh_tipo(Token *tk) {

  if (tk == NULL)
    return 0;

  if (tk->type != PALAVRA_RESERVADA)
    return 0;

  return (strcmp(tk->lexema, "int") == 0 || strcmp(tk->lexema, "float") == 0 ||
          strcmp(tk->lexema, "char") == 0 || strcmp(tk->lexema, "void") == 0);
}

int eh_if(Token *tk) {

  if (tk == NULL)
    return 0;

  return (tk->type == PALAVRA_RESERVADA && strcmp(tk->lexema, "if") == 0);
}

int eh_else(Token *tk) {

  if (tk == NULL)
    return 0;

  return (tk->type == PALAVRA_RESERVADA && strcmp(tk->lexema, "else") == 0);
}

int eh_while(Token *tk) {

  if (tk == NULL)
    return 0;

  return (tk->type == PALAVRA_RESERVADA && strcmp(tk->lexema, "while") == 0);
}

int eh_for(Token *tk) {

  if (tk == NULL)
    return 0;

  return (tk->type == PALAVRA_RESERVADA && strcmp(tk->lexema, "for") == 0);
}

int eh_return(Token *tk) {

  if (tk == NULL)
    return 0;

  return (tk->type == PALAVRA_RESERVADA && strcmp(tk->lexema, "return") == 0);
}

int eh_operador_relacional(Token *tk) {

  if (tk == NULL)
    return 0;

  return (tk->type == OP_IGUAL || tk->type == OP_DIF || tk->type == OP_MAIOR ||
          tk->type == OP_MENOR || tk->type == OP_MAIOR_IGUAL ||
          tk->type == OP_MENOR_IGUAL);
}

int eh_incr_decr(Token *tk) {

  if (tk == NULL)
    return 0;

  return (tk->type == OP_INCR || tk->type == OP_DECRE);
}

//====================================================
// LOOKAHEAD AUXILIAR
// Verifica se o token seguinte ao atual é do tipo dado
// sem avançar o parser.
//====================================================

int proximo_eh(Parser *p, TokenType type) {

  if (p->atual == NULL || p->atual->prox == NULL)
    return 0;

  return p->atual->prox->info.type == type;
}

//====================================================
// PROTÓTIPOS
//====================================================

AST *parse_programa(Parser *p);
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

//====================================================
// INICIALIZAÇÃO
//====================================================

void iniciar_parser(Parser *p, Lista *tokens) { p->atual = *tokens; }

//====================================================
// PROGRAMA
//====================================================

AST *parse_programa(Parser *p) {

  Token token_prog;
  token_prog.lexema = "PROGRAMA";
  token_prog.type = EOF_TOKEN;

  AST *programa = criar_no_ast(AST_PROGRAMA, token_prog);

  while (1) {

    Token *tk = token_atual(p);

    if (tk == NULL || tk->type == EOF_TOKEN)
      break;

    if (!eh_tipo(tk)) {
      erro_sintatico("Esperado declaracao ou definicao de funcao", tk);
      sincronizar(p);
      continue;
    }

    AST *item = parse_declaracao_ou_funcao(p);

    if (item != NULL)
      adicionar_filho(programa, item);
  }

  return programa;
}

//====================================================
// DECLARAÇÃO OU FUNÇÃO (escopo global)
// tipo IDENT ( ...           → função
// tipo IDENT [= expr] { , }  → declaração global
//====================================================

AST *parse_declaracao_ou_funcao(Parser *p) {

  Token tipo = *token_atual(p);
  avancar(p);

  Token *tk_nome = token_atual(p);
  if (tk_nome == NULL || tk_nome->type != IDENT) {
    erro_sintatico("Esperado identificador apos tipo", tk_nome);
    sincronizar(p);
    return NULL;
  }

  Token nome = *tk_nome;
  avancar(p);

  // Lookahead: '(' → definição de função
  Token *tk = token_atual(p);
  if (tk != NULL && tk->type == SEP_ABRE_PAR)
    return parse_funcao(p, tipo, nome);

  // Declaração global com lista de declaradores
  AST *dec = criar_no_ast(AST_DECLARACAO, tipo);
  adicionar_filho(dec, criar_no_ast(AST_TIPO, tipo));

  // Primeiro declarador (nome já consumido)
  AST *declarador = criar_no_ast(AST_DECLARADOR, nome);
  adicionar_filho(declarador, criar_no_ast(AST_IDENTIFICADOR, nome));

  if (match(p, OP_ATRIB)) {
    AST *expr = parse_expressao(p);
    adicionar_filho(declarador, expr);
  }

  adicionar_filho(dec, declarador);

  // Declaradores adicionais separados por ','
  while (match(p, SEP_VIRGULA)) {

    Token *tk_extra = token_atual(p);
    if (tk_extra == NULL || tk_extra->type != IDENT) {
      erro_sintatico("Esperado identificador", tk_extra);
      sincronizar(p);
      return dec;
    }

    Token extra = *tk_extra;
    avancar(p);

    AST *dec_extra = criar_no_ast(AST_DECLARADOR, extra);
    adicionar_filho(dec_extra, criar_no_ast(AST_IDENTIFICADOR, extra));

    if (match(p, OP_ATRIB)) {
      AST *expr = parse_expressao(p);
      adicionar_filho(dec_extra, expr);
    }

    adicionar_filho(dec, dec_extra);
  }

  consumir(p, SEP_PONTO_VIRGULA, "Esperado ';'");
  return dec;
}

//====================================================
// FUNÇÃO   tipo nome ( params ) bloco
//====================================================

AST *parse_funcao(Parser *p, Token tipo, Token nome) {

  AST *funcao = criar_no_ast(AST_FUNCAO, nome);

  adicionar_filho(funcao, criar_no_ast(AST_TIPO, tipo));

  consumir(p, SEP_ABRE_PAR, "Esperado '(' na definicao de funcao");
  AST *params = parse_lista_params(p);
  adicionar_filho(funcao, params);
  consumir(p, SEP_FECHA_PAR, "Esperado ')' apos parametros");

  AST *corpo = parse_bloco(p);
  adicionar_filho(funcao, corpo);

  return funcao;
}

//====================================================
// LISTA DE PARÂMETROS
//====================================================

AST *parse_lista_params(Parser *p) {

  Token tk_fake;
  tk_fake.lexema = "PARAMS";
  tk_fake.type = EOF_TOKEN;

  AST *lista = criar_no_ast(AST_PARAMS, tk_fake);

  Token *tk = token_atual(p);

  if (tk == NULL || tk->type == SEP_FECHA_PAR)
    return lista;

  // void sozinho = sem parâmetros
  if (tk->type == PALAVRA_RESERVADA && strcmp(tk->lexema, "void") == 0) {
    avancar(p);
    return lista;
  }

  AST *param = parse_param(p);
  if (param)
    adicionar_filho(lista, param);

  while (match(p, SEP_VIRGULA)) {
    param = parse_param(p);
    if (param)
      adicionar_filho(lista, param);
  }

  return lista;
}

//====================================================
// PARÂMETRO INDIVIDUAL   tipo identificador
//====================================================

AST *parse_param(Parser *p) {

  Token *tk = token_atual(p);

  if (!eh_tipo(tk)) {
    erro_sintatico("Esperado tipo no parametro", tk);
    sincronizar(p);
    return NULL;
  }

  Token tipo = *tk;
  avancar(p);

  Token *tk_nome = token_atual(p);
  if (tk_nome == NULL || tk_nome->type != IDENT) {
    erro_sintatico("Esperado identificador no parametro", tk_nome);
    sincronizar(p);
    return NULL;
  }

  Token nome = *tk_nome;
  avancar(p);

  AST *param = criar_no_ast(AST_PARAM, nome);
  adicionar_filho(param, criar_no_ast(AST_TIPO, tipo));
  adicionar_filho(param, criar_no_ast(AST_IDENTIFICADOR, nome));

  return param;
}

//====================================================
// COMANDO
//====================================================

AST *parse_comando(Parser *p) {

  Token *tk = token_atual(p);

  if (tk == NULL)
    return NULL;

  if (eh_tipo(tk))
    return parse_dec(p);

  // IDENT pode ser: atribuição (x = ...) ou incremento (x++ / x--)
  if (tk->type == IDENT)
    return parse_cmd_ident(p);

  if (eh_if(tk))
    return parse_cond(p);

  if (eh_while(tk))
    return parse_rep(p);

  if (eh_for(tk))
    return parse_for(p);

  if (eh_return(tk))
    return parse_return(p);

  if (tk->type == SEP_ABRE_CHAVE)
    return parse_bloco(p);

  // Comando vazio
  if (tk->type == SEP_PONTO_VIRGULA) {
    avancar(p);
    return NULL;
  }

  erro_sintatico("Comando invalido", tk);
  sincronizar(p);
  return NULL;
}

//====================================================
// COMANDO INICIADO POR IDENT
// Decide entre:  x = expr;   ou   x++;   ou   x--;
//====================================================

AST *parse_cmd_ident(Parser *p) {

  Token ident = *token_atual(p);
  avancar(p); // consome IDENT

  Token *tk = token_atual(p);

  // x++ ou x--  (pós-incremento/decremento como statement)
  if (tk != NULL && eh_incr_decr(tk))
    return parse_incr_decr(p, ident);

  // x = expr;
  return parse_att(p, ident);
}

//====================================================
// ATRIBUIÇÃO   (ident já consumido)   = expr ;
//====================================================

AST *parse_att(Parser *p, Token ident) {

  Token *tk_op = token_atual(p);
  if (tk_op == NULL || tk_op->type != OP_ATRIB) {
    erro_sintatico("Esperado '=' ou '++'/'--)' apos identificador", tk_op);
    sincronizar(p);
    return NULL;
  }

  Token op_token = *tk_op;
  avancar(p); // consome '='

  AST *expr = parse_expressao(p);

  consumir(p, SEP_PONTO_VIRGULA, "Esperado ';'");

  AST *att = criar_no_ast(AST_ATRIBUICAO, op_token);
  att->esquerda = criar_no_ast(AST_IDENTIFICADOR, ident);
  att->direita = expr;

  return att;
}

//====================================================
// PÓS-INCREMENTO / PÓS-DECREMENTO   ident++ ;  ident-- ;
// (ident já consumido)
//====================================================

AST *parse_incr_decr(Parser *p, Token ident) {

  Token op = *token_atual(p); // ++ ou --
  avancar(p);                 // consome ++ / --

  consumir(p, SEP_PONTO_VIRGULA, "Esperado ';'");

  // Representa como nó unário: filho esquerdo = operando
  AST *no = criar_no_ast(AST_UNOP, op);
  no->esquerda = criar_no_ast(AST_IDENTIFICADOR, ident);

  return no;
}

//====================================================
// BLOCO  { lista_cmds }
//====================================================

AST *parse_bloco(Parser *p) {

  Token tk_fake;
  tk_fake.lexema = "BLOCO";
  tk_fake.type = EOF_TOKEN;

  consumir(p, SEP_ABRE_CHAVE, "Esperado '{'");

  AST *bloco = criar_no_ast(AST_BLOCO, tk_fake);

  while (1) {

    Token *tk = token_atual(p);

    if (tk == NULL || tk->type == SEP_FECHA_CHAVE || tk->type == EOF_TOKEN)
      break;

    AST *cmd = parse_comando(p);

    if (cmd != NULL)
      adicionar_filho(bloco, cmd);
  }

  consumir(p, SEP_FECHA_CHAVE, "Esperado '}'");

  return bloco;
}

//====================================================
// DECLARAÇÃO LOCAL
// tipo nome [= expr] { , nome [= expr] } ;
// Ex: int a, b = 2, c;
//====================================================

AST *parse_dec(Parser *p) {

  Token tipo = *token_atual(p);
  avancar(p);

  AST *dec = criar_no_ast(AST_DECLARACAO, tipo);
  adicionar_filho(dec, criar_no_ast(AST_TIPO, tipo));

  do {

    Token *tk_ident = token_atual(p);
    if (tk_ident == NULL || tk_ident->type != IDENT) {
      erro_sintatico("Esperado identificador", tk_ident);
      sincronizar(p);
      return dec;
    }

    Token ident = *tk_ident;
    avancar(p);

    AST *declarador = criar_no_ast(AST_DECLARADOR, ident);
    adicionar_filho(declarador, criar_no_ast(AST_IDENTIFICADOR, ident));

    if (match(p, OP_ATRIB)) {
      AST *expr = parse_expressao(p);
      adicionar_filho(declarador, expr);
    }

    adicionar_filho(dec, declarador);

  } while (match(p, SEP_VIRGULA));

  consumir(p, SEP_PONTO_VIRGULA, "Esperado ';'");

  return dec;
}

//====================================================
// FOR   for ( init ; cond ; update ) cmd
//
// init   → declaração (sem ';' extra) ou atribuição
// cond   → expr_rel  (pode ser vazia)
// update → ident++ / ident-- / ident = expr  (sem ';')
//====================================================

AST *parse_for(Parser *p) {

  Token tk_for = *token_atual(p);
  avancar(p); // consome 'for'

  consumir(p, SEP_ABRE_PAR, "Esperado '(' apos 'for'");

  AST *for_node = criar_no_ast(AST_FOR, tk_for);

  // ── inicialização ────────────────────────────────
  // Pode ser declaração (int i = 0;) ou atribuição (i = 0;) ou vazia (;)
  Token *tk = token_atual(p);
  AST *init = NULL;

  if (tk != NULL && tk->type == SEP_PONTO_VIRGULA) {
    // init vazia
    avancar(p);

  } else if (tk != NULL && eh_tipo(tk)) {
    // declaração: int i = 0;   — parse_dec já consome o ';'
    init = parse_dec(p);

  } else if (tk != NULL && tk->type == IDENT) {
    // atribuição: i = 0;   — consome IDENT e delega
    Token ident = *tk;
    avancar(p);
    init = parse_att(p, ident); // parse_att consome o ';'

  } else {
    erro_sintatico("Esperado inicializacao no 'for'", tk);
    sincronizar(p);
  }

  adicionar_filho(for_node, init); // filho 0 — pode ser NULL

  // ── condição ─────────────────────────────────────
  tk = token_atual(p);
  AST *cond = NULL;

  if (tk != NULL && tk->type != SEP_PONTO_VIRGULA)
    cond = parse_expr_logica(p);

  consumir(p, SEP_PONTO_VIRGULA, "Esperado ';' apos condicao do 'for'");
  adicionar_filho(for_node, cond); // filho 1 — pode ser NULL

  // ── atualização ──────────────────────────────────
  // Não termina com ';', termina com ')'
  tk = token_atual(p);
  AST *update = NULL;

  if (tk != NULL && tk->type != SEP_FECHA_PAR) {

    if (tk->type == IDENT) {

      Token ident = *tk;
      avancar(p); // consome IDENT

      Token *tk2 = token_atual(p);

      if (tk2 != NULL && eh_incr_decr(tk2)) {
        // i++ ou i--  (sem ';' aqui pois termina em ')')
        Token op = *tk2;
        avancar(p);
        update = criar_no_ast(AST_UNOP, op);
        update->esquerda = criar_no_ast(AST_IDENTIFICADOR, ident);

      } else if (tk2 != NULL && tk2->type == OP_ATRIB) {
        // i = expr  (sem ';')
        Token op_token = *tk2;
        avancar(p); // consome '='
        AST *expr = parse_expressao(p);
        update = criar_no_ast(AST_ATRIBUICAO, op_token);
        update->esquerda = criar_no_ast(AST_IDENTIFICADOR, ident);
        update->direita = expr;

      } else {
        erro_sintatico("Esperado '++', '--' ou '=' na atualizacao do 'for'",
                       tk2);
        sincronizar(p);
      }

    } else {
      erro_sintatico("Esperado identificador na atualizacao do 'for'", tk);
      sincronizar(p);
    }
  }

  consumir(p, SEP_FECHA_PAR, "Esperado ')' apos atualizacao do 'for'");
  adicionar_filho(for_node, update); // filho 2 — pode ser NULL

  // ── corpo ─────────────────────────────────────────
  AST *corpo = parse_comando(p);
  adicionar_filho(for_node, corpo); // filho 3

  return for_node;
}

//====================================================
// RETURN   return [expr] ;
//====================================================

AST *parse_return(Parser *p) {

  Token tk_ret = *token_atual(p);
  avancar(p);

  AST *ret = criar_no_ast(AST_RETURN, tk_ret);

  Token *tk = token_atual(p);
  if (tk != NULL && tk->type != SEP_PONTO_VIRGULA) {
    AST *expr = parse_expressao(p);
    adicionar_filho(ret, expr);
  }

  consumir(p, SEP_PONTO_VIRGULA, "Esperado ';'");

  return ret;
}

//====================================================
// IF   if ( expr_rel ) cmd [else cmd]
//====================================================

AST *parse_cond(Parser *p) {

  Token tk_if = *token_atual(p);
  avancar(p);

  consumir(p, SEP_ABRE_PAR, "Esperado '('");
  AST *condicao = parse_expr_logica(p);
  consumir(p, SEP_FECHA_PAR, "Esperado ')'");

  AST *cmd_if = parse_comando(p);

  AST *if_node = criar_no_ast(AST_IF, tk_if);
  if_node->esquerda = condicao;
  if_node->direita = cmd_if;

  Token *tk = token_atual(p);
  if (eh_else(tk)) {
    avancar(p);
    AST *else_cmd = parse_comando(p);
    adicionar_filho(if_node, else_cmd);
  }

  return if_node;
}

//====================================================
// WHILE   while ( expr_rel ) cmd
//====================================================

AST *parse_rep(Parser *p) {

  Token tk_while = *token_atual(p);
  avancar(p);

  consumir(p, SEP_ABRE_PAR, "Esperado '('");
  AST *condicao = parse_expr_logica(p);
  consumir(p, SEP_FECHA_PAR, "Esperado ')'");

  AST *corpo = parse_comando(p);

  AST *while_node = criar_no_ast(AST_WHILE, tk_while);
  while_node->esquerda = condicao;
  while_node->direita = corpo;

  return while_node;
}

//====================================================
// EXPRESSÃO RELACIONAL   expressao [op_rel expressao]
// Ex: a + b > c * 2
//====================================================

AST *parse_expr_rel(Parser *p) {

  AST *esquerda = parse_expressao(p);

  Token *tk = token_atual(p);

  if (!eh_operador_relacional(tk))
    return esquerda;

  Token op = *tk;
  avancar(p);

  AST *direita = parse_expressao(p);

  AST *binop = criar_no_ast(AST_BINOP, op);
  binop->esquerda = esquerda;
  binop->direita = direita;

  return binop;
}

//====================================================
// EXPRESSÃO LÓGICA   expr_rel { (&&  ||) expr_rel }
// Ex: a > b && b > a && c < a || c < d
// Precedência: && tem maior prioridade que ||
// Para simplificar tratamos ambos no mesmo nível (left-assoc).
//====================================================

AST *parse_expr_logica(Parser *p) {

  AST *esquerda = parse_expr_rel(p);

  Token *tk = token_atual(p);

  while (tk != NULL && (tk->type == OP_AND || tk->type == OP_OR)) {

    Token op = *tk;
    avancar(p);

    AST *direita = parse_expr_rel(p);

    AST *binop = criar_no_ast(AST_BINOP, op);
    binop->esquerda = esquerda;
    binop->direita = direita;
    esquerda = binop;

    tk = token_atual(p);
  }

  return esquerda;
}

//====================================================
// EXPRESSÃO   termo { (+|-) termo }
//====================================================

AST *parse_expressao(Parser *p) {

  AST *esquerda = parse_termo(p);

  Token *tk = token_atual(p);

  while (tk != NULL && (tk->type == OP_SOMA || tk->type == OP_SUB)) {

    Token op = *tk;
    avancar(p);

    AST *direita = parse_termo(p);

    AST *binop = criar_no_ast(AST_BINOP, op);
    binop->esquerda = esquerda;
    binop->direita = direita;
    esquerda = binop;

    tk = token_atual(p);
  }

  return esquerda;
}

//====================================================
// TERMO   fator { (*|/) fator }
//====================================================

AST *parse_termo(Parser *p) {

  AST *esquerda = parse_fator(p);

  Token *tk = token_atual(p);

  while (tk != NULL && (tk->type == OP_MULT || tk->type == OP_DIV)) {

    Token op = *tk;
    avancar(p);

    AST *direita = parse_fator(p);

    AST *binop = criar_no_ast(AST_BINOP, op);
    binop->esquerda = esquerda;
    binop->direita = direita;
    esquerda = binop;

    tk = token_atual(p);
  }

  return esquerda;
}

//====================================================
// FATOR   IDENT | LITERAL | ( expr )
//====================================================

AST *parse_fator(Parser *p) {

  Token *tk = token_atual(p);

  if (tk == NULL)
    return NULL;

  if (tk->type == IDENT) {
    AST *no = criar_no_ast(AST_IDENTIFICADOR, *tk);
    avancar(p);
    return no;
  }

  if (tk->type == INT_LITERAL || tk->type == FLOAT_LITERAL ||
      tk->type == CHAR_LITERAL || tk->type == STRING_LITERAL) {
    AST *no = criar_no_ast(AST_LITERAL, *tk);
    avancar(p);
    return no;
  }

  if (tk->type == SEP_ABRE_PAR) {
    avancar(p);
    AST *expr = parse_expressao(p);
    consumir(p, SEP_FECHA_PAR, "Esperado ')'");
    return expr;
  }

  erro_sintatico("Fator invalido", tk);
  sincronizar(p);
  return NULL;
}