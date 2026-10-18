/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

char* copiaString(const char* texto){
    int tamanho = strlen(texto) + 1;
    char* copia = malloc(tamanho);

    if(copia != NULL){
        strcpy(copia, texto);
    }

    return copia;
}

int reservarTokens(VetorTokens* tokens){
    if(tokens->tamanho < tokens->capacidade){
        return 1;
    }

    int novaCapacidade = 
        tokens->capacidade == 0 ? 16 : tokens->capacidade * 2;

    Token* novo = realloc(tokens->dados, novaCapacidade * sizeof(Token));

    if (novo == NULL) {
        return 0;
    }

    tokens->dados = novo;
    tokens->capacidade = novaCapacidade;

    return 1;
}

int reservarErrosLexicos(VetorErrosLexicos* erros){
    if (erros->tamanho < erros->capacidade) {
        return 1;
    }

    size_t novaCapacidade =
        erros->capacidade == 0 ? 8 : erros->capacidade * 2;

    ErroLexico *novo = realloc(
        erros->dados,
        novaCapacidade * sizeof(ErroLexico)
    );

    if (novo == NULL) {
        return 0;
    }

    erros->dados = novo;
    erros->capacidade = novaCapacidade;

    return 1;
}

int adicionarToken(VetorTokens* tokens, char* lexema, TokenType tipo, int linha)
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

/* ==================== Gerenciamento de memoria ==================== */

static char *copiarString(const char *texto)
{
    size_t tamanho = strlen(texto) + 1;
    char *copia = malloc(tamanho);

    if (copia != NULL) {
        strcpy(copia, texto);
    }

    return copia;
}

static int acrescentarCaractere(
    char **lexema,
    size_t *tamanho,
    size_t *capacidade,
    int caractere
)
{
    if (*tamanho + 1 >= *capacidade) {
        size_t novaCapacidade =
            (*capacidade == 0) ? 16 : *capacidade * 2;

        char *novo = realloc(*lexema, novaCapacidade);

        if (novo == NULL) {
            return 0;
        }

        *lexema = novo;
        *capacidade = novaCapacidade;
    }

    (*lexema)[(*tamanho)++] = (char)caractere;
    (*lexema)[*tamanho] = '\0';

    return 1;
}

static void limparLexema(char *lexema, size_t *tamanho)
{
    *tamanho = 0;
    lexema[0] = '\0';
}

/* ==================== Vetor de tokens ==================== */

static int adicionarToken(
    VetorTokens *tokens,
    const char *lexema,
    TokenType tipo,
    int linha
)
{
    if (tokens->tamanho == tokens->capacidade) {
        size_t novaCapacidade =
            (tokens->capacidade == 0) ? 16 : tokens->capacidade * 2;

        Token *novo = realloc(
            tokens->dados,
            novaCapacidade * sizeof(Token)
        );

        if (novo == NULL) {
            return 0;
        }

        tokens->dados = novo;
        tokens->capacidade = novaCapacidade;
    }

    char *copia = copiarString(lexema);

    if (copia == NULL) {
        return 0;
    }

    Token *token = &tokens->dados[tokens->tamanho];

    token->lexema = copia;
    token->tipo = tipo;
    token->linha = linha;

    tokens->tamanho++;

    return 1;
}

/* ==================== Vetor de erros ==================== */

static int adicionarErro(
    VetorErrosLexicos *erros,
    const char *lexema,
    const char *mensagem,
    int linha
)
{
    if (erros->tamanho == erros->capacidade) {
        size_t novaCapacidade =
            (erros->capacidade == 0) ? 8 : erros->capacidade * 2;

        ErroLexico *novo = realloc(
            erros->dados,
            novaCapacidade * sizeof(ErroLexico)
        );

        if (novo == NULL) {
            return 0;
        }

        erros->dados = novo;
        erros->capacidade = novaCapacidade;
    }

    char *lexemaCopia = copiarString(lexema);
    char *mensagemCopia = copiarString(mensagem);

    if (lexemaCopia == NULL || mensagemCopia == NULL) {
        free(lexemaCopia);
        free(mensagemCopia);
        return 0;
    }

    ErroLexico *erro = &erros->dados[erros->tamanho];

    erro->lexema = lexemaCopia;
    erro->mensagem = mensagemCopia;
    erro->linha = linha;

    erros->tamanho++;

    return 1;
}

/* ==================== Resolucao do token ==================== */

/*
 * O AFD reconhece a estrutura do lexema.
 * Estas funcoes determinam o tipo definitivo.
 */
static TokenType resolverToken(int estado, const char *lexema)
{
    if (estado == QID) {
        return tokenPalavraReservada(lexema);
    }

    if (estado == QSIMBOLO) {
        return tokenSimbolo(lexema[0]);
    }

    return tokenDoEstado[estado];
}

/* ==================== Recuperacao de erros ==================== */

static int ehDelimitador(int c)
{
    return c == EOF ||
           c == ' ' || c == '\t' || c == '\r' ||
           c == '\n' || c == '\f' || c == '\v' ||
           c == ';' || c == ',' ||
           c == '(' || c == ')' ||
           c == '{' || c == '}' ||
           c == '+' || c == '-' ||
           c == '*' || c == '/' ||
           c == '=' || c == '!' ||
           c == '<' || c == '>' ||
           c == '\'' || c == '"';
}

/*
 * Para literais delimitados por aspas, tenta consumir ate
 * a aspa de fechamento ou ate o fim da linha.
 * Para os demais lexemas incompletos, avanca ate um delimitador.
 */
static int recuperarLexema(
    CodigoFonte *codigo,
    int estado,
    char **lexema,
    size_t *tamanho,
    size_t *capacidade
)
{
    int caractere = olharCaractere(codigo);

    int literalChar =
        estado == QCHAR_ABERTO || estado == QCHAR_ESCRITO;

    int literalString =
        estado == QSTR_ABERTA || estado == QSTR_ESCRITA;

    if (literalChar || literalString) {
        int aspa = literalChar ? '\'' : '"';

        while (caractere != EOF &&
               caractere != '\n' &&
               caractere != aspa) {
            if (!acrescentarCaractere(
                    lexema, tamanho, capacidade,
                    lerCaractere(codigo))) {
                return 0;
            }

            caractere = olharCaractere(codigo);
        }

        if (caractere == aspa) {
            if (!acrescentarCaractere(
                    lexema, tamanho, capacidade,
                    lerCaractere(codigo))) {
                return 0;
            }
        }

        return 1;
    }

    while (caractere != EOF && !ehDelimitador(caractere)) {
        if (!acrescentarCaractere(
                lexema, tamanho, capacidade,
                lerCaractere(codigo))) {
            return 0;
        }

        caractere = olharCaractere(codigo);
    }

    return 1;
}

/* ==================== Analisador lexico ==================== */

int analisarLexico(
    CodigoFonte *codigo,
    VetorTokens *tokens,
    VetorErrosLexicos *erros
)
{
    if (codigo == NULL || codigo->arquivo == NULL ||
        tokens == NULL || erros == NULL) {
        return 0;
    }

    /*
     * Contrato: tokens e erros devem ser inicializados com {0}
     * antes desta chamada.
     */
    tokens->dados = NULL;
    tokens->tamanho = 0;
    tokens->capacidade = 0;

    erros->dados = NULL;
    erros->tamanho = 0;
    erros->capacidade = 0;

    inicializarTabela();

    char *lexema = NULL;
    size_t tamanho = 0;
    size_t capacidade = 0;

    int estado = Q0;
    int linhaInicial = codigo->linha;

    for (;;) {
        int caractere = olharCaractere(codigo);
        ClasseChar classe = (ClasseChar)classificar(caractere);

        /* Procurando o inicio de um token. */
        if (estado == Q0) {
            if (caractere == EOF) {
                if (!adicionarToken(tokens, "", TOK_EOF, codigo->linha)) {
                    goto erroMemoria;
                }

                break;
            }

            int proximo = tabela[Q0][classe];

            /* Espacos e quebras de linha nao geram tokens. */
            if (proximo == Q0) {
                lerCaractere(codigo);
                continue;
            }

            /* Caractere sem transicao inicial valida. */
            if (proximo == ESTADO_ERRO) {
                linhaInicial = codigo->linha;

                if (!acrescentarCaractere(
                        &lexema, &tamanho, &capacidade,
                        lerCaractere(codigo))) {
                    goto erroMemoria;
                }

                if (!adicionarErro(
                        erros, lexema,
                        "Caractere invalido", linhaInicial)) {
                    goto erroMemoria;
                }

                limparLexema(lexema, &tamanho);
                continue;
            }

            /* Inicio de um token reconhecido pela tabela. */
            linhaInicial = codigo->linha;

            if (!acrescentarCaractere(
                    &lexema, &tamanho, &capacidade,
                    lerCaractere(codigo))) {
                goto erroMemoria;
            }

            estado = proximo;
            continue;
        }

        /*
         * Ja existe um lexema em construcao.
         * A tabela decide se o caractere pode ser consumido.
         */
        int proximo = tabela[estado][classe];

        if (proximo != ESTADO_ERRO) {
            if (!acrescentarCaractere(
                    &lexema, &tamanho, &capacidade,
                    lerCaractere(codigo))) {
                goto erroMemoria;
            }

            estado = proximo;
            continue;
        }

        /*
         * A transicao falhou.
         * Se o estado atual for final, emite o token sem
         * consumir o caractere consultado.
         */
        if (ehFinal(estado)) {
            TokenType tipo = resolverToken(estado, lexema);

            if (tipo == TOK_NENHUM) {
                if (!adicionarErro(
                        erros, lexema,
                        "Token nao reconhecido", linhaInicial)) {
                    goto erroMemoria;
                }
            } else {
                if (!adicionarToken(
                        tokens, lexema, tipo, linhaInicial)) {
                    goto erroMemoria;
                }
            }

            limparLexema(lexema, &tamanho);
            estado = Q0;
            continue;
        }

        /*
         * Estado nao final: o lexema esta incompleto.
         * Recupera a entrada e registra o erro.
         */
        if (!recuperarLexema(
                codigo, estado,
                &lexema, &tamanho, &capacidade)) {
            goto erroMemoria;
        }

        if (!adicionarErro(
                erros, lexema,
                "Lexema incompleto ou invalido", linhaInicial)) {
            goto erroMemoria;
        }

        limparLexema(lexema, &tamanho);
        estado = Q0;
    }

    free(lexema);
    return 1;

erroMemoria:
    free(lexema);
    return 0;
}

/* ==================== Liberacao de memoria ==================== */

void liberarTokens(VetorTokens *tokens)
{
    if (tokens == NULL) {
        return;
    }

    for (size_t i = 0; i < tokens->tamanho; i++) {
        free(tokens->dados[i].lexema);
    }

    free(tokens->dados);

    tokens->dados = NULL;
    tokens->tamanho = 0;
    tokens->capacidade = 0;
}

void liberarErrosLexicos(VetorErrosLexicos *erros)
{
    if (erros == NULL) {
        return;
    }

    for (size_t i = 0; i < erros->tamanho; i++) {
        free(erros->dados[i].lexema);
        free(erros->dados[i].mensagem);
    }

    free(erros->dados);

    erros->dados = NULL;
    erros->tamanho = 0;
    erros->capacidade = 0;
}
