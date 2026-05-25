#include "../include/token.h"

const char *obter_nome_token(TokenType type) {
  switch (type) {
  case IDENT:
    return "IDENT";
  case PALAVRA_RESERVADA:
    return "PALAVRA_RESERVADA";
  case INT_LITERAL:
    return "INT_LITERAL";
  case FLOAT_LITERAL:
    return "FLOAT_LITERAL";
  case STRING_LITERAL:
    return "STRING_LITERAL";
  case CHAR_LITERAL:
    return "CHAR_LITERAL";

  // Operadores
  case OP_SOMA:
    return "OP_SOMA";
  case OP_INCR:
    return "OP_INCR";
  case OP_SUB:
    return "OP_SUB";
  case OP_DECRE:
    return "OP_DECRE";
  case OP_MULT:
    return "OP_MULT";
  case OP_DIV:
    return "OP_DIV";
  case OP_ATRIB:
    return "OP_ATRIB";
  case OP_IGUAL:
    return "OP_IGUAL";
  case OP_NOT:
    return "OP_NOT";
  case OP_DIF:
    return "OP_DIF";
  case OP_MAIOR:
    return "OP_MAIOR";
  case OP_MAIOR_IGUAL:
    return "OP_MAIOR_IGUAL";
  case OP_MENOR:
    return "OP_MENOR";
  case OP_MENOR_IGUAL:
    return "OP_MENOR_IGUAL";

  // Separadores
  case SEP_VIRGULA:
    return "SEP_VIRGULA";
  case SEP_PONTO_VIRGULA:
    return "SEP_PONTO_VIRGULA";
  case SEP_ABRE_PAR:
    return "SEP_ABRE_PAR";
  case SEP_FECHA_PAR:
    return "SEP_FECHA_PAR";
  case SEP_ABRE_CHAVE:
    return "SEP_ABRE_CHAVE";
  case SEP_FECHA_CHAVE:
    return "SEP_FECHA_CHAVE";
  case SEP_ABRE_COLCH:
    return "SEP_ABRE_COLCH";
  case SEP_FECHA_COLCH:
    return "SEP_FECHA_COLCH";
  case EOF_TOKEN:
    return "EOF_TOKEN";

  default:
    return "DESCONHECIDO";
  }
}