
#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>
#include "leitor.h"
#include "afd_tabela.h"

typedef struct token {
    char *lexema;
    TokenType tipo;
    int linha;
} Token;

typedef struct erroLexico {
    char *lexema;
    char *mensagem;
    int linha;
} ErroLexico;

typedef struct vetorTokens {
    Token *dados;
    size_t tamanho;
    size_t capacidade;
} VetorTokens;

typedef struct vetorErrosLexicos {
    ErroLexico *dados;
    size_t tamanho;
    size_t capacidade;
} VetorErrosLexicos;

/*
 * Analisa o arquivo e preenche os vetores de tokens e erros.
 * Retorna 1 se a analise foi executada, ou 0 se houve
 * falha de memoria.
 *
 * Os vetores devem ser inicializados com {0}.
 */
int analisarLexico(
    CodigoFonte *codigo,
    VetorTokens *tokens,
    VetorErrosLexicos *erros
);

void liberarTokens(VetorTokens *tokens);
void liberarErrosLexicos(VetorErrosLexicos *erros);

#endif
