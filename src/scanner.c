#include "../include/scanner.h"

#include "../include/LSE.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void disparar_erro(const char *mensagem, int linha, int coluna, char *lexema) {
  printf("ERRO LEXICO: %s na linha %d, coluna %d, lexema_erro = %s\n", mensagem,
         linha, coluna, lexema);
}

int is_palavra_reservada(char *lexema) {
  static char palavras_reservadas[][50] = {"int",    "float", "char",    "void",

                                           "if",     "else",  "while",   "for",

                                           "return", "break", "continue"};

  const int N = sizeof(palavras_reservadas) / sizeof(palavras_reservadas[0]);

  for (int i = 0; i < N; i++) {
    if (strcmp(palavras_reservadas[i], lexema) == 0) {
      return 1;
    }
  }
  return 0;
}

Token criar_token(char *lexema, TokenType type, Posicao p) {
  Token token;

  token.lexema = (char *)malloc(strlen(lexema) + 1);
  token.pos = p;
  token.type = type;

  strcpy(token.lexema, lexema);

  return token;
}

void coletar_ident_keyword(FILE *arq, char c_inicial, Lista *lista_tokens,
                           Posicao *p) {
  int capacidade = 16;
  int index = 0;

  char *lexema = (char *)malloc(capacidade * sizeof(char));
  if (!lexema) {
    perror("Erro de alocação de memória para o lexema");
    exit(1);
  }

  char c = c_inicial;

  while (isalnum(c) || c == '_') {
    if (index >= capacidade - 1) {
      capacidade *= 2;
      char *temp = (char *)realloc(lexema, capacidade * sizeof(char));
      if (!temp) {
        perror("Erro de realocação de memória para o número");
        free(lexema);
        exit(1);
      }
      lexema = temp;
    }

    lexema[index++] = c;
    c = fgetc(arq);
  }

  lexema[index] = '\0';

  if (c != EOF) {
    ungetc(c, arq);
  }

  TokenType type = is_palavra_reservada(lexema) ? PALAVRA_RESERVADA : IDENT;

  Token token = criar_token(lexema, type, *p);

  const int COL = p->coluna;
  p->coluna = COL + index;

  insereFim(lista_tokens, token);

  free(lexema);
}

void coletar_numero(FILE *arq, char c_inicial, Lista *lista_tokens,
                    Posicao *p) {
  int capacidade = 16;
  int index = 0;

  char *lexema = (char *)malloc(capacidade * sizeof(char));
  if (!lexema) {
    perror("Erro de alocação de memória para o número");
    exit(1);
  }

  char c = c_inicial;

  while (isdigit(c)) {
    if (index >= capacidade - 1) {
      capacidade *= 2;
      char *temp = (char *)realloc(lexema, capacidade * sizeof(char));
      if (!temp) {
        perror("Erro de realocação de memória para o número");
        free(lexema);
        exit(1);
      }
      lexema = temp;
    }
    lexema[index++] = c;
    c = fgetc(arq);
  }

  int is_inteiro = 1;

  if (c == '.') {
    is_inteiro = 0;

    if (index >= capacidade - 1) {
      capacidade *= 2;
      char *temp = (char *)realloc(lexema, capacidade * sizeof(char));
      if (!temp) {
        perror("Erro de realocação");
        free(lexema);
        exit(1);
      }
      lexema = temp;
    }
    lexema[index++] = c;

    c = fgetc(arq);

    if (!isdigit(c)) {
      is_inteiro = 1;
      disparar_erro("Numero de ponto flutuante mal formatado", p->linha,
                    p->coluna + index, lexema);
    }

    while (isdigit(c)) {
      if (index >= capacidade - 1) {
        capacidade *= 2;
        char *temp = (char *)realloc(lexema, capacidade * sizeof(char));
        if (!temp) {
          perror("Erro de realocação");
          free(lexema);
          exit(1);
        }
        lexema = temp;
      }
      lexema[index++] = c;
      c = fgetc(arq);
    }
  }

  lexema[index] = '\0';

  if (c != EOF) {
    ungetc(c, arq);
  }

  TokenType type = is_inteiro ? INT_LITERAL : FLOAT_LITERAL;

  Token token = criar_token(lexema, type, *p);

  const int COL = p->coluna;
  p->coluna = COL + index;

  insereFim(lista_tokens, token);

  free(lexema);
}

void coletar_string(FILE *arq, char c_inicial, Lista *lista_tokens,
                    Posicao *p) {
  int capacidade = 16;
  int index = 0;

  char *lexema = (char *)malloc(capacidade * sizeof(char));
  if (!lexema) {
    perror("Erro de alocação de memória para a string");
    exit(1);
  }

  lexema[index++] = c_inicial;

  char c = fgetc(arq);

  while (c != '"' && c != EOF) {
    if (index >= capacidade - 2) {
      capacidade *= 2;
      char *temp = (char *)realloc(lexema, capacidade * sizeof(char));
      if (!temp) {
        perror("Erro de realocação");
        free(lexema);
        exit(1);
      }
      lexema = temp;
    }

    lexema[index++] = c;
    c = fgetc(arq);
  }

  if (c == EOF) {
    disparar_erro("String nao fechada antes do fim do arquivo", p->linha,
                  p->coluna, lexema);
  } else {
    lexema[index++] = c;
  }

  lexema[index] = '\0';

  Token token = criar_token(lexema, STRING_LITERAL, *p);

  const int COL = p->coluna;
  p->coluna = COL + index;

  insereFim(lista_tokens, token);

  free(lexema);
}

void coletar_char(FILE *arq, char c_inicial, Lista *lista_tokens, Posicao *p) {
  int capacidade = 5;
  int index = 0;

  char *lexema = (char *)malloc(capacidade * sizeof(char));
  if (!lexema) {
    perror("Erro de alocação");
    exit(1);
  }

  lexema[index++] = c_inicial;

  char c = fgetc(arq);

  if (c != EOF) {

    lexema[index++] = c;

    if (c == '\\') {
      c = fgetc(arq);
      if (c != EOF) {
        lexema[index++] = c;
      }
    }

    c = fgetc(arq);
    if (c == '\'') {
      lexema[index++] = c;
    } else {
      lexema[index] = '\0';

      disparar_erro("Caractere mal formatado ou nao fechado", p->linha,
                    p->coluna, lexema);

      if (c != EOF) {
        ungetc(c, arq);
      }

      free(lexema);
      return;
    }
  }

  lexema[index] = '\0';

  Token token = criar_token(lexema, CHAR_LITERAL, *p);

  const int COL = p->coluna;
  p->coluna = COL + index;

  insereFim(lista_tokens, token);

  free(lexema);
}

void ignorar_comentario_linha(FILE *arq, Posicao *p) {
  int c;

  while ((c = fgetc(arq)) != '\n' && c != EOF) {
    p->coluna++;
  }

  if (c == '\n') {
    ungetc(c, arq);
  }
}

void ignorar_comentario_bloco(FILE *arq, Posicao *p) {
  int c;
  int estado = 0;

  while ((c = fgetc(arq)) != EOF) {
    if (c == '\n') {
      p->linha++;
      p->coluna = 0;
    } else {
      p->coluna++;
    }

    if (estado == 0 && c == '*') {
      estado = 1;
    } else if (estado == 1 && c == '/') {
      break;
    } else if (estado == 1 && c != '*') {
      estado = 0;
    }
  }

  if (c == EOF) {
    disparar_erro("Comentario de bloco '/*' nao foi fechado", p->linha,
                  p->coluna, "");
  }
}

/*
  Coleta operadores:
  +  ++  -  --  *  /  %
  =  ==  !  !=  <  <=  >  >=
  &&  ||
*/
void coletar_operadores(FILE *arq, char c_inicial, Lista *lista_tokens,
                        Posicao *p) {
  int capacidade = 3;
  int index = 0;

  char *lexema = (char *)malloc(capacidade * sizeof(char));
  if (!lexema) {
    perror("Erro de alocação de memória");
    exit(1);
  }

  lexema[index++] = c_inicial;
  TokenType type;

  int next_char = fgetc(arq);

  switch (c_inicial) {

  case '+':
    if (next_char == '+') {
      type = OP_INCR;
      lexema[index++] = (char)next_char;
    } else {
      type = OP_SOMA;
      ungetc(next_char, arq);
    }
    break;

  case '-':
    if (next_char == '-') {
      type = OP_DECRE;
      lexema[index++] = (char)next_char;
    } else {
      type = OP_SUB;
      ungetc(next_char, arq);
    }
    break;

  case '*':
    type = OP_MULT;
    ungetc(next_char, arq);
    break;

  case '%':
    type = OP_MOD;
    ungetc(next_char, arq);
    break;

  case '=':
    if (next_char == '=') {
      type = OP_IGUAL;
      lexema[index++] = (char)next_char;
    } else {
      type = OP_ATRIB;
      ungetc(next_char, arq);
    }
    break;

  case '!':
    if (next_char == '=') {
      type = OP_DIF;
      lexema[index++] = (char)next_char;
    } else {
      type = OP_NOT;
      ungetc(next_char, arq);
    }
    break;

  case '<':
    if (next_char == '=') {
      type = OP_MENOR_IGUAL;
      lexema[index++] = (char)next_char;
    } else {
      type = OP_MENOR;
      ungetc(next_char, arq);
    }
    break;

  case '>':
    if (next_char == '=') {
      type = OP_MAIOR_IGUAL;
      lexema[index++] = (char)next_char;
    } else {
      type = OP_MAIOR;
      ungetc(next_char, arq);
    }
    break;

  // && — se vier só '&' é erro léxico
  case '&':
    if (next_char == '&') {
      type = OP_AND;
      lexema[index++] = (char)next_char;
    } else {
      ungetc(next_char, arq);
      lexema[index] = '\0';
      disparar_erro("Operador '&' invalido, use '&&'", p->linha, p->coluna,
                    lexema);
      free(lexema);
      return;
    }
    break;

  // || — se vier só '|' é erro léxico
  case '|':
    if (next_char == '|') {
      type = OP_OR;
      lexema[index++] = (char)next_char;
    } else {
      ungetc(next_char, arq);
      lexema[index] = '\0';
      disparar_erro("Operador '|' invalido, use '||'", p->linha, p->coluna,
                    lexema);
      free(lexema);
      return;
    }
    break;

  default:
    // Caractere não reconhecido como operador
    ungetc(next_char, arq);
    lexema[index] = '\0';
    disparar_erro("Operador invalido", p->linha, p->coluna, lexema);
    free(lexema);
    return;
  }

  lexema[index] = '\0';

  Token token = criar_token(lexema, type, *p);

  const int COL = p->coluna;
  p->coluna = COL + index;

  insereFim(lista_tokens, token);
  free(lexema);
}

void coletar_separadores(char c_inicial, Lista *lista_tokens, Posicao *p) {
  char lexema[2];
  lexema[0] = c_inicial;
  lexema[1] = '\0';

  TokenType type;

  switch (c_inicial) {
  case ',':
    type = SEP_VIRGULA;
    break;
  case ';':
    type = SEP_PONTO_VIRGULA;
    break;
  case '(':
    type = SEP_ABRE_PAR;
    break;
  case ')':
    type = SEP_FECHA_PAR;
    break;
  case '{':
    type = SEP_ABRE_CHAVE;
    break;
  case '}':
    type = SEP_FECHA_CHAVE;
    break;
  case '[':
    type = SEP_ABRE_COLCH;
    break;
  case ']':
    type = SEP_FECHA_COLCH;
    break;
  default:
    disparar_erro("Separador invalido", p->linha, p->coluna, lexema);
    p->coluna++;
    return;
  }

  Token token = criar_token(lexema, type, *p);

  p->coluna++;

  insereFim(lista_tokens, token);
}

Lista *executar_analise_lexica(char *nome_arq_codigo_fonte) {
  FILE *arq = fopen(nome_arq_codigo_fonte, "r");

  if (!arq) {
    perror("Falha ao abrir arquivo");
    exit(1);
  }

  Lista *lista_tokens = criaLista();
  int c;

  Posicao p = {1, 1};

  while ((c = fgetc(arq)) != EOF) {

    if (isspace(c)) {
      if (c == '\n') {
        p.linha++;
        p.coluna = 1;
      } else {
        p.coluna++;
      }
      continue;
    }

    if (isalpha(c) || c == '_') {
      coletar_ident_keyword(arq, (char)c, lista_tokens, &p);

    } else if (isdigit(c)) {
      coletar_numero(arq, (char)c, lista_tokens, &p);

    } else if (c == '"') {
      coletar_string(arq, (char)c, lista_tokens, &p);

    } else if (c == '\'') {
      coletar_char(arq, (char)c, lista_tokens, &p);

    } else if (c == '/') {
      int prox = fgetc(arq);

      if (prox == '/') {
        p.coluna += 2;
        ignorar_comentario_linha(arq, &p);
      } else if (prox == '*') {
        p.coluna += 2;
        ignorar_comentario_bloco(arq, &p);
      } else {
        if (prox != EOF)
          ungetc(prox, arq);
        coletar_operadores(arq, '/', lista_tokens, &p);
      }

    } else if (strchr("+-*%=!<>&|", c)) {
      // Inclui agora: & | %
      coletar_operadores(arq, (char)c, lista_tokens, &p);

    } else if (strchr(",;(){}[]", c)) {
      coletar_separadores((char)c, lista_tokens, &p);

    } else {
      char lexema[2];
      lexema[0] = (char)c;
      lexema[1] = '\0';
      disparar_erro("Caractere invalido", p.linha, p.coluna, lexema);
      p.coluna++;
    }
  }

  // Insere token de fim de arquivo
  Posicao pos_eof = p;
  Token eof_token = criar_token("EOF", EOF_TOKEN, pos_eof);
  insereFim(lista_tokens, eof_token);

  fclose(arq);
  return lista_tokens;
}