#ifndef AFD_TABELA_H
#define AFD_TABELA_H

#include <stdio.h>
#include <string.h>

/* ==================== Tokens da linguagem P ==================== */

typedef enum {
    TOK_FUNCTION, TOK_MAIN, TOK_LET, TOK_INT, TOK_FLOAT, TOK_CHAR,
    TOK_IF, TOK_ELSE, TOK_WHILE, TOK_PRINTLN, TOK_RETURN,
    TOK_LBRACKET, TOK_RBRACKET, TOK_LBRACE, TOK_RBRACE,
    TOK_ARROW, TOK_COLON, TOK_SEMICOLON, TOK_COMMA,
    TOK_ASSIGN, TOK_EQ, TOK_NE, TOK_GT, TOK_GE, TOK_LT, TOK_LE,
    TOK_PLUS, TOK_MINUS, TOK_MULT, TOK_DIV,
    TOK_ID, TOK_INT_CONST, TOK_FLOAT_CONST, TOK_CHAR_LITERAL, TOK_FMT_STRING,
    TOK_EOF,
    TOK_NENHUM      /* marca estados nao finais / "sem token" */
} TokenType;

/* ==================== Classes de caracteres ==================== */

typedef enum {
    C_LETRA,            /* a-z A-Z                     */
    C_DIGITO,           /* 0-9                         */
    C_UNDERSCORE,       /* _                           */
    C_PONTO,            /* .                           */
    C_ESPACO,           /* espaco, \t, \r              */
    C_NOVA_LINHA,       /* \n                          */
    C_SIMBOLO,          /* + * / ( ) { } : , ;         */
    C_MENOS,            /* -                           */
    C_MAIOR,            /* >                           */
    C_IGUAL,            /* =                           */
    C_EXCLAMACAO,       /* !                           */
    C_MENOR,            /* <                           */
    C_ASPAS_SIMPLES,    /* '                           */
    C_ASPAS_DUPLAS,     /* "                           */
    C_OUTRO,            /* qualquer outro caractere    */
    C_EOF,              /* fim do arquivo              */
    NUM_CLASSES
} ClasseChar;

/* ==================== Estados do AFD ==================== */

typedef enum {
    Q0,
    QID,
    QINT, QDOT, QFLOAT,
    QSIMBOLO,
    QMINUS, QARROW,
    QASSIGN, QEQUALS,
    QNEGATE, QDIFF,
    QLESS, QLESSEQUAL,
    QGREATER, QGREATEREQUAL,
    QCHAR_ABERTO, QCHAR_ESCRITO, QCHAR,
    QSTR_ABERTA, QSTR_ESCRITA, QSTRING,
    NUM_ESTADOS
} Estado;

#define ESTADO_ERRO (-1)

/* ==================== Classificacao de caracteres ==================== */

static int classificar(int c)
{
    if (c == EOF) return C_EOF;
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) return C_LETRA;
    if (c >= '0' && c <= '9') return C_DIGITO;

    switch (c) {
        case '_':return C_UNDERSCORE;
        case '.':return C_PONTO;
        case ' ':
        case '\t':
        case '\r': return C_ESPACO;
        case '\n': return C_NOVA_LINHA;
        case '+': case '*': case '/':
        case '(': case ')': case '{': case '}':
        case ':': case ',': case ';':
                   return C_SIMBOLO;
        case '-':  return C_MENOS;
        case '>':  return C_MAIOR;
        case '=':  return C_IGUAL;
        case '!':  return C_EXCLAMACAO;
        case '<':  return C_MENOR;
        case '\'': return C_ASPAS_SIMPLES;
        case '"':  return C_ASPAS_DUPLAS;
        default:   return C_OUTRO;
    }
}

/* ==================== Tabela de transicao ==================== */

/* tabela[estado][classe] = proximo estado, ou ESTADO_ERRO */
static int tabela[NUM_ESTADOS][NUM_CLASSES];

/* Preenche as transicoes de "qualquer classe, exceto aspas, \n e EOF".
   Usado dentro de char literal e de string de formatacao. */
static void permitirQualquerExceto(int origem, int destino, int aspas)
{
    for (int c = 0; c < NUM_CLASSES; c++) {
        if (c == aspas || c == C_NOVA_LINHA || c == C_EOF) {
            continue;
        }
        tabela[origem][c] = destino;
    }
}

/* Chamar UMA vez antes de comecar a analise lexica. */
static void inicializarTabela(void)
{
    /* Tudo comeca como erro: transicao nao desenhada = erro lexico. */
    for (int e = 0; e < NUM_ESTADOS; e++) {
        for (int c = 0; c < NUM_CLASSES; c++) {
            tabela[e][c] = ESTADO_ERRO;
        }
    }

    /* q0: espacos e quebras de linha nao geram token */
    tabela[Q0][C_ESPACO]        = Q0;
    tabela[Q0][C_NOVA_LINHA]    = Q0;

    /* q0: inicio de cada token */
    tabela[Q0][C_LETRA]         = QID;
    tabela[Q0][C_DIGITO]        = QINT;
    tabela[Q0][C_SIMBOLO]       = QSIMBOLO;
    tabela[Q0][C_MENOS]         = QMINUS;
    tabela[Q0][C_IGUAL]         = QASSIGN;
    tabela[Q0][C_EXCLAMACAO]    = QNEGATE;
    tabela[Q0][C_MENOR]         = QLESS;
    tabela[Q0][C_MAIOR]         = QGREATER;
    tabela[Q0][C_ASPAS_SIMPLES] = QCHAR_ABERTO;
    tabela[Q0][C_ASPAS_DUPLAS]  = QSTR_ABERTA;

    /* identificadores */
    tabela[QID][C_LETRA]        = QID;
    tabela[QID][C_DIGITO]       = QID;
    tabela[QID][C_UNDERSCORE]   = QID;

    /* numeros */
    tabela[QINT][C_DIGITO]      = QINT;
    tabela[QINT][C_PONTO]       = QDOT;
    tabela[QDOT][C_DIGITO]      = QFLOAT;
    tabela[QFLOAT][C_DIGITO]    = QFLOAT;

    /* operadores de mais de um caractere */
    tabela[QMINUS][C_MAIOR]     = QARROW;
    tabela[QASSIGN][C_IGUAL]    = QEQUALS;
    tabela[QNEGATE][C_IGUAL]    = QDIFF;
    tabela[QLESS][C_IGUAL]      = QLESSEQUAL;
    tabela[QGREATER][C_IGUAL]   = QGREATEREQUAL;

    /* char literal: ' <um char exceto ' e \n> ' */
    permitirQualquerExceto(QCHAR_ABERTO, QCHAR_ESCRITO, C_ASPAS_SIMPLES);
    tabela[QCHAR_ESCRITO][C_ASPAS_SIMPLES] = QCHAR;

    /* string de formatacao: " <um ou mais chars exceto " e \n> " */
    permitirQualquerExceto(QSTR_ABERTA,  QSTR_ESCRITA, C_ASPAS_DUPLAS);
    permitirQualquerExceto(QSTR_ESCRITA, QSTR_ESCRITA, C_ASPAS_DUPLAS);
    tabela[QSTR_ESCRITA][C_ASPAS_DUPLAS] = QSTRING;
}

/* ==================== Estados finais e seus tokens ==================== */

/* TOK_NENHUM = estado NAO final.
   QID e QSIMBOLO tem um token provisorio: o tipo real sai de
   tokenPalavraReservada() e tokenSimbolo(). */
static const TokenType tokenDoEstado[NUM_ESTADOS] = {
    [Q0]            = TOK_NENHUM,
    [QID]           = TOK_ID,
    [QINT]          = TOK_INT_CONST,
    [QDOT]          = TOK_NENHUM,
    [QFLOAT]        = TOK_FLOAT_CONST,
    [QSIMBOLO]      = TOK_NENHUM,      /* resolvido por tokenSimbolo() */
    [QMINUS]        = TOK_MINUS,
    [QARROW]        = TOK_ARROW,
    [QASSIGN]       = TOK_ASSIGN,
    [QEQUALS]       = TOK_EQ,
    [QNEGATE]       = TOK_NENHUM,
    [QDIFF]         = TOK_NE,
    [QLESS]         = TOK_LT,
    [QLESSEQUAL]    = TOK_LE,
    [QGREATER]      = TOK_GT,
    [QGREATEREQUAL] = TOK_GE,
    [QCHAR_ABERTO]  = TOK_NENHUM,
    [QCHAR_ESCRITO] = TOK_NENHUM,
    [QCHAR]         = TOK_CHAR_LITERAL,
    [QSTR_ABERTA]   = TOK_NENHUM,
    [QSTR_ESCRITA]  = TOK_NENHUM,
    [QSTRING]       = TOK_FMT_STRING
};

/* 1 se o estado e final (aceita um token). */
static int ehFinal(int estado)
{
    if (estado == QSIMBOLO) return 1;
    return estado >= 0 && estado < NUM_ESTADOS &&
           tokenDoEstado[estado] != TOK_NENHUM;
}

/* ==================== Resolucao por lexema ==================== */

/* Chamar quando o estado final for QID. */
static TokenType tokenPalavraReservada(const char *lexema)
{
    if (strcmp(lexema, "fn") == 0)      return TOK_FUNCTION;
    if (strcmp(lexema, "main") == 0)    return TOK_MAIN;
    if (strcmp(lexema, "let") == 0)     return TOK_LET;
    if (strcmp(lexema, "int") == 0)     return TOK_INT;
    if (strcmp(lexema, "float") == 0)   return TOK_FLOAT;
    if (strcmp(lexema, "char") == 0)    return TOK_CHAR;
    if (strcmp(lexema, "if") == 0)      return TOK_IF;
    if (strcmp(lexema, "else") == 0)    return TOK_ELSE;
    if (strcmp(lexema, "while") == 0)   return TOK_WHILE;
    if (strcmp(lexema, "println") == 0) return TOK_PRINTLN;
    if (strcmp(lexema, "return") == 0)  return TOK_RETURN;
    return TOK_ID;
}

/* Chamar quando o estado final for QSIMBOLO (lexema de 1 caractere). */
static TokenType tokenSimbolo(char c)
{
    switch (c) {
        case '+': return TOK_PLUS;
        case '*': return TOK_MULT;
        case '/': return TOK_DIV;
        case '(': return TOK_LBRACKET;
        case ')': return TOK_RBRACKET;
        case '{': return TOK_LBRACE;
        case '}': return TOK_RBRACE;
        case ':': return TOK_COLON;
        case ',': return TOK_COMMA;
        case ';': return TOK_SEMICOLON;
        default:  return TOK_NENHUM;
    }
}

/* Nome do token para imprimir na saida. */
static const char *nomeToken(TokenType t)
{
    switch (t) {
        case TOK_FUNCTION:     return "FUNCTION";
        case TOK_MAIN:         return "MAIN";
        case TOK_LET:          return "LET";
        case TOK_INT:          return "INT";
        case TOK_FLOAT:        return "FLOAT";
        case TOK_CHAR:         return "CHAR";
        case TOK_IF:           return "IF";
        case TOK_ELSE:         return "ELSE";
        case TOK_WHILE:        return "WHILE";
        case TOK_PRINTLN:      return "PRINTLN";
        case TOK_RETURN:       return "RETURN";
        case TOK_LBRACKET:     return "LBRACKET";
        case TOK_RBRACKET:     return "RBRACKET";
        case TOK_LBRACE:       return "LBRACE";
        case TOK_RBRACE:       return "RBRACE";
        case TOK_ARROW:        return "ARROW";
        case TOK_COLON:        return "COLON";
        case TOK_SEMICOLON:    return "SEMICOLON";
        case TOK_COMMA:        return "COMMA";
        case TOK_ASSIGN:       return "ASSIGN";
        case TOK_EQ:           return "EQ";
        case TOK_NE:           return "NE";
        case TOK_GT:           return "GT";
        case TOK_GE:           return "GE";
        case TOK_LT:           return "LT";
        case TOK_LE:           return "LE";
        case TOK_PLUS:         return "PLUS";
        case TOK_MINUS:        return "MINUS";
        case TOK_MULT:         return "MULT";
        case TOK_DIV:          return "DIV";
        case TOK_ID:           return "ID";
        case TOK_INT_CONST:    return "INT_CONST";
        case TOK_FLOAT_CONST:  return "FLOAT_CONST";
        case TOK_CHAR_LITERAL: return "CHAR_LITERAL";
        case TOK_FMT_STRING:   return "FMT_STRING";
        case TOK_EOF:          return "EOF";
        default:               return "NENHUM";
    }
}

#endif /* AFD_TABELA_H */