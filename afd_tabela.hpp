#ifndef AFD_TABELA_HPP
#define AFD_TABELA_HPP

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>

/* ==================== Tokens da linguagem P ==================== */

/* Obs.: o fim de arquivo se chama END_OF_FILE porque EOF e uma macro do C. */
enum class TokenType {
    FUNCTION, MAIN, LET, INT, FLOAT, CHAR,
    IF, ELSE, WHILE, PRINTLN, RETURN,
    LBRACKET, RBRACKET, LBRACE, RBRACE,
    ARROW, COLON, SEMICOLON, COMMA,
    ASSIGN, EQ, NE, GT, GE, LT, LE,
    PLUS, MINUS, MULT, DIV,
    ID, INT_CONST, FLOAT_CONST, CHAR_LITERAL, FMT_STRING,
    END_OF_FILE,
    NENHUM          /* marca estados nao finais / "sem token" */
};

/* ==================== Classes de caracteres ==================== */

enum class ClasseChar {
    LETRA,            /* a-z A-Z                     */
    DIGITO,           /* 0-9                         */
    UNDERSCORE,       /* _                           */
    PONTO,            /* .                           */
    ESPACO,           /* espaco, \t, \r              */
    NOVA_LINHA,       /* \n                          */
    SIMBOLO,          /* + * / ( ) { } : , ;         */
    MENOS,            /* -                           */
    MAIOR,            /* >                           */
    IGUAL,            /* =                           */
    EXCLAMACAO,       /* !                           */
    MENOR,            /* <                           */
    ASPAS_SIMPLES,    /* '                           */
    ASPAS_DUPLAS,     /* "                           */
    OUTRO,            /* qualquer outro caractere    */
    FIM_ARQUIVO,      /* fim do arquivo              */
    TOTAL             /* quantidade de classes       */
};

/* ==================== Estados do AFD ==================== */

enum class Estado {
    ERRO = -1,        /* transicao inexistente = erro lexico */
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
    TOTAL             /* quantidade de estados */
};

constexpr std::size_t N_ESTADOS = static_cast<std::size_t>(Estado::TOTAL);
constexpr std::size_t N_CLASSES = static_cast<std::size_t>(ClasseChar::TOTAL);

constexpr std::size_t idx(Estado e)     { return static_cast<std::size_t>(e); }
constexpr std::size_t idx(ClasseChar c) { return static_cast<std::size_t>(c); }

/* ==================== O AFD ==================== */

class AFD {
public:
    AFD() { inicializarTabela(); }

    /* Classifica um caractere. Use o int devolvido por get()/peek(), que
       pode ser o valor de fim de arquivo (traits::eof()). */
    static ClasseChar classificar(int c)
    {
        if (c == std::char_traits<char>::eof()) return ClasseChar::FIM_ARQUIVO;
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
            return ClasseChar::LETRA;
        if (c >= '0' && c <= '9') return ClasseChar::DIGITO;

        switch (c) {
            case '_':  return ClasseChar::UNDERSCORE;
            case '.':  return ClasseChar::PONTO;
            case ' ':
            case '\t':
            case '\r': return ClasseChar::ESPACO;
            case '\n': return ClasseChar::NOVA_LINHA;
            case '+': case '*': case '/':
            case '(': case ')': case '{': case '}':
            case ':': case ',': case ';':
                       return ClasseChar::SIMBOLO;
            case '-':  return ClasseChar::MENOS;
            case '>':  return ClasseChar::MAIOR;
            case '=':  return ClasseChar::IGUAL;
            case '!':  return ClasseChar::EXCLAMACAO;
            case '<':  return ClasseChar::MENOR;
            case '\'': return ClasseChar::ASPAS_SIMPLES;
            case '"':  return ClasseChar::ASPAS_DUPLAS;
            default:   return ClasseChar::OUTRO;
        }
    }

    /* Transicao: estado atual + caractere -> proximo estado (ou Estado::ERRO). */
    Estado proximoEstado(Estado e, int c) const
    {
        if (e == Estado::ERRO) return Estado::ERRO;
        return tabela_[idx(e)][idx(classificar(c))];
    }

    /* Token reconhecido por um estado final. TokenType::NENHUM = estado nao
       final. QID e QSIMBOLO tem token provisorio: o tipo real sai de
       tokenPalavraReservada() e tokenSimbolo(). */
    static TokenType tokenDoEstado(Estado e)
    {
        switch (e) {
            case Estado::QID:            return TokenType::ID;
            case Estado::QINT:           return TokenType::INT_CONST;
            case Estado::QFLOAT:         return TokenType::FLOAT_CONST;
            case Estado::QMINUS:         return TokenType::MINUS;
            case Estado::QARROW:         return TokenType::ARROW;
            case Estado::QASSIGN:        return TokenType::ASSIGN;
            case Estado::QEQUALS:        return TokenType::EQ;
            case Estado::QDIFF:          return TokenType::NE;
            case Estado::QLESS:          return TokenType::LT;
            case Estado::QLESSEQUAL:     return TokenType::LE;
            case Estado::QGREATER:       return TokenType::GT;
            case Estado::QGREATEREQUAL:  return TokenType::GE;
            case Estado::QCHAR:          return TokenType::CHAR_LITERAL;
            case Estado::QSTRING:        return TokenType::FMT_STRING;
            default:                     return TokenType::NENHUM;
        }
    }

    /* true se o estado e final (aceita um token). */
    static bool ehFinal(Estado e)
    {
        return e == Estado::QSIMBOLO || tokenDoEstado(e) != TokenType::NENHUM;
    }

private:
    /* tabela_[estado][classe] = proximo estado, ou Estado::ERRO */
    std::array<std::array<Estado, N_CLASSES>, N_ESTADOS> tabela_;

    void set(Estado origem, ClasseChar classe, Estado destino)
    {
        tabela_[idx(origem)][idx(classe)] = destino;
    }

    /* "Qualquer classe, exceto a aspa indicada, \n e fim de arquivo".
       Usado dentro de char literal e de string de formatacao. */
    void permitirQualquerExceto(Estado origem, Estado destino, ClasseChar aspas)
    {
        for (std::size_t c = 0; c < N_CLASSES; c++) {
            ClasseChar classe = static_cast<ClasseChar>(c);
            if (classe == aspas || classe == ClasseChar::NOVA_LINHA ||
                classe == ClasseChar::FIM_ARQUIVO) {
                continue;
            }
            set(origem, classe, destino);
        }
    }

    void inicializarTabela()
    {
        /* Tudo comeca como erro: transicao nao desenhada = erro lexico. */
        for (auto &linha : tabela_) {
            linha.fill(Estado::ERRO);
        }

        /* q0: espacos e quebras de linha nao geram token */
        set(Estado::Q0, ClasseChar::ESPACO,        Estado::Q0);
        set(Estado::Q0, ClasseChar::NOVA_LINHA,    Estado::Q0);

        /* q0: inicio de cada token */
        set(Estado::Q0, ClasseChar::LETRA,         Estado::QID);
        set(Estado::Q0, ClasseChar::DIGITO,        Estado::QINT);
        set(Estado::Q0, ClasseChar::SIMBOLO,       Estado::QSIMBOLO);
        set(Estado::Q0, ClasseChar::MENOS,         Estado::QMINUS);
        set(Estado::Q0, ClasseChar::IGUAL,         Estado::QASSIGN);
        set(Estado::Q0, ClasseChar::EXCLAMACAO,    Estado::QNEGATE);
        set(Estado::Q0, ClasseChar::MENOR,         Estado::QLESS);
        set(Estado::Q0, ClasseChar::MAIOR,         Estado::QGREATER);
        set(Estado::Q0, ClasseChar::ASPAS_SIMPLES, Estado::QCHAR_ABERTO);
        set(Estado::Q0, ClasseChar::ASPAS_DUPLAS,  Estado::QSTR_ABERTA);

        /* identificadores */
        set(Estado::QID, ClasseChar::LETRA,        Estado::QID);
        set(Estado::QID, ClasseChar::DIGITO,       Estado::QID);
        set(Estado::QID, ClasseChar::UNDERSCORE,   Estado::QID);

        /* numeros */
        set(Estado::QINT,   ClasseChar::DIGITO,    Estado::QINT);
        set(Estado::QINT,   ClasseChar::PONTO,     Estado::QDOT);
        set(Estado::QDOT,   ClasseChar::DIGITO,    Estado::QFLOAT);
        set(Estado::QFLOAT, ClasseChar::DIGITO,    Estado::QFLOAT);

        /* operadores de mais de um caractere */
        set(Estado::QMINUS,   ClasseChar::MAIOR,   Estado::QARROW);
        set(Estado::QASSIGN,  ClasseChar::IGUAL,   Estado::QEQUALS);
        set(Estado::QNEGATE,  ClasseChar::IGUAL,   Estado::QDIFF);
        set(Estado::QLESS,    ClasseChar::IGUAL,   Estado::QLESSEQUAL);
        set(Estado::QGREATER, ClasseChar::IGUAL,   Estado::QGREATEREQUAL);

        /* char literal: ' <um char exceto ' e \n> ' */
        permitirQualquerExceto(Estado::QCHAR_ABERTO, Estado::QCHAR_ESCRITO,
                               ClasseChar::ASPAS_SIMPLES);
        set(Estado::QCHAR_ESCRITO, ClasseChar::ASPAS_SIMPLES, Estado::QCHAR);

        /* string de formatacao: " <um ou mais chars exceto " e \n> " */
        permitirQualquerExceto(Estado::QSTR_ABERTA,  Estado::QSTR_ESCRITA,
                               ClasseChar::ASPAS_DUPLAS);
        permitirQualquerExceto(Estado::QSTR_ESCRITA, Estado::QSTR_ESCRITA,
                               ClasseChar::ASPAS_DUPLAS);
        set(Estado::QSTR_ESCRITA, ClasseChar::ASPAS_DUPLAS, Estado::QSTRING);
    }
};

/* ==================== Resolucao por lexema ==================== */

/* Chamar quando o estado final for QID. */
inline TokenType tokenPalavraReservada(const std::string &lexema)
{
    static const std::unordered_map<std::string, TokenType> reservadas = {
        {"fn",      TokenType::FUNCTION},
        {"main",    TokenType::MAIN},
        {"let",     TokenType::LET},
        {"int",     TokenType::INT},
        {"float",   TokenType::FLOAT},
        {"char",    TokenType::CHAR},
        {"if",      TokenType::IF},
        {"else",    TokenType::ELSE},
        {"while",   TokenType::WHILE},
        {"println", TokenType::PRINTLN},
        {"return",  TokenType::RETURN}
    };

    auto it = reservadas.find(lexema);
    return it != reservadas.end() ? it->second : TokenType::ID;
}

/* Chamar quando o estado final for QSIMBOLO (lexema de 1 caractere). */
inline TokenType tokenSimbolo(char c)
{
    switch (c) {
        case '+': return TokenType::PLUS;
        case '*': return TokenType::MULT;
        case '/': return TokenType::DIV;
        case '(': return TokenType::LBRACKET;
        case ')': return TokenType::RBRACKET;
        case '{': return TokenType::LBRACE;
        case '}': return TokenType::RBRACE;
        case ':': return TokenType::COLON;
        case ',': return TokenType::COMMA;
        case ';': return TokenType::SEMICOLON;
        default:  return TokenType::NENHUM;
    }
}

/* Nome do token para imprimir na saida. */
inline std::string_view nomeToken(TokenType t)
{
    switch (t) {
        case TokenType::FUNCTION:     return "FUNCTION";
        case TokenType::MAIN:         return "MAIN";
        case TokenType::LET:          return "LET";
        case TokenType::INT:          return "INT";
        case TokenType::FLOAT:        return "FLOAT";
        case TokenType::CHAR:         return "CHAR";
        case TokenType::IF:           return "IF";
        case TokenType::ELSE:         return "ELSE";
        case TokenType::WHILE:        return "WHILE";
        case TokenType::PRINTLN:      return "PRINTLN";
        case TokenType::RETURN:       return "RETURN";
        case TokenType::LBRACKET:     return "LBRACKET";
        case TokenType::RBRACKET:     return "RBRACKET";
        case TokenType::LBRACE:       return "LBRACE";
        case TokenType::RBRACE:       return "RBRACE";
        case TokenType::ARROW:        return "ARROW";
        case TokenType::COLON:        return "COLON";
        case TokenType::SEMICOLON:    return "SEMICOLON";
        case TokenType::COMMA:        return "COMMA";
        case TokenType::ASSIGN:       return "ASSIGN";
        case TokenType::EQ:           return "EQ";
        case TokenType::NE:           return "NE";
        case TokenType::GT:           return "GT";
        case TokenType::GE:           return "GE";
        case TokenType::LT:           return "LT";
        case TokenType::LE:           return "LE";
        case TokenType::PLUS:         return "PLUS";
        case TokenType::MINUS:        return "MINUS";
        case TokenType::MULT:         return "MULT";
        case TokenType::DIV:          return "DIV";
        case TokenType::ID:           return "ID";
        case TokenType::INT_CONST:    return "INT_CONST";
        case TokenType::FLOAT_CONST:  return "FLOAT_CONST";
        case TokenType::CHAR_LITERAL: return "CHAR_LITERAL";
        case TokenType::FMT_STRING:   return "FMT_STRING";
        case TokenType::END_OF_FILE:  return "EOF";
        default:                      return "NENHUM";
    }
}

#endif /* AFD_TABELA_HPP */