#ifndef TOKEN_H
#define TOKEN_H

typedef enum {

  // Identificadores e palavras
  IDENT = 0,
  PALAVRA_RESERVADA,

  // Literais
  INT_LITERAL,
  FLOAT_LITERAL,
  CHAR_LITERAL,
  STRING_LITERAL,

  // Operadores
  OP_SOMA, // +
  OP_SUB,  // -
  OP_MULT, // *
  OP_DIV,  // /
  OP_MOD,  // %

  OP_IGUAL,       // ==
  OP_DIF,         // !=
  OP_MENOR,       // <
  OP_MAIOR,       // >
  OP_MENOR_IGUAL, // <=
  OP_MAIOR_IGUAL, // >=
  OP_INCR,        // ++
  OP_DECRE,       // --

  COMENTARIO_BLOCO, /* comment */
  COMENTARIO_LINHA, // comment

  // Atribuição
  OP_ATRIB, // =

  // Operadores lógicos
  OP_AND, // &&
  OP_OR,  // ||
  OP_NOT, // !

  // Delimitadores
  SEP_ABRE_PAR,      // (
  SEP_FECHA_PAR,     // )
  SEP_ABRE_CHAVE,    // {
  SEP_FECHA_CHAVE,   // }
  SEP_PONTO_VIRGULA, // ;
  SEP_VIRGULA,       // ,
  SEP_ABRE_COLCH,    // [
  SEP_FECHA_COLCH,
  EOF_TOKEN
} TokenType;

typedef struct {
  int linha;
  int coluna;
} Posicao;

typedef struct {
  int codigo;
  char *lexema;
  TokenType type;
  Posicao pos;
} Token;

const char *obter_nome_token(TokenType type);

#endif