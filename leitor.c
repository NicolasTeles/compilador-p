#include "leitor.h"
#include "lexer.h"

int abrirCodigoFonte(CodigoFonte *codigo, const char *nomeArquivo)
{
    codigo->arquivo = fopen(nomeArquivo, "r");
    codigo->linha = 1;

    if (codigo->arquivo == NULL) {
        fprintf(stderr, "Nao foi possivel abrir o arquivo: %s\n", nomeArquivo);
        return 0;
    }

    return 1;
}

void fecharCodigoFonte(CodigoFonte *codigo)
{
   if (codigo->arquivo != NULL) {
        fclose(codigo->arquivo);
        codigo->arquivo = NULL;
    }
}

int lerCaractere(CodigoFonte *codigo)
{
    int caractere = fgetc(codigo->arquivo);

    if (caractere == '\n') {
        codigo->linha++;
    }

    return caractere;
}

void devolverCaractere(CodigoFonte *codigo, int caractere)
{
    if (caractere == EOF) {
        return;
    }

    ungetc(caractere, codigo->arquivo);

    if (caractere == '\n') {
        codigo->linha--;
    }
}

int olharCaractere(CodigoFonte *codigo)
{
    int caractere = lerCaractere(codigo);
    devolverCaractere(codigo, caractere);
    return caractere;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo-fonte>\n", argv[0]);
        return EXIT_FAILURE;
    }

    CodigoFonte codigo = {0};

    if (!abrirCodigoFonte(&codigo, argv[1])) {
        fprintf(stderr, "Erro ao abrir o arquivo: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    VetorTokens tokens = {0};
    VetorErrosLexicos erros = {0};

    if (!analisarLexico(&codigo, &tokens, &erros)) {
        fprintf(stderr, "Erro ao executar a análise léxica.\n");

        fecharCodigoFonte(&codigo);
        liberarTokens(&tokens);
        liberarErrosLexicos(&erros);

        return EXIT_FAILURE;
    }

    fecharCodigoFonte(&codigo);

    printf("=== TOKENS ===\n");

    for (size_t i = 0; i < tokens.tamanho; i++) {
        printf(
            "Linha %-4d Tipo %-15s Lexema: %s\n",
            tokens.dados[i].linha,
            nomeToken(tokens.dados[i].tipo),
            tokens.dados[i].lexema
        );
    }

    printf("\n=== ERROS LEXICOS ===\n");

    if (erros.tamanho == 0) {
        printf("Nenhum erro lexico encontrado.\n");
    } else {
        for (size_t i = 0; i < erros.tamanho; i++) {
            printf(
                "Linha %d: %s (lexema: %s)\n",
                erros.dados[i].linha,
                erros.dados[i].mensagem,
                erros.dados[i].lexema
            );
        }
    }

    /* Guardar antes de liberar o vetor de erros. */
    int houveErros = erros.tamanho > 0;

    liberarTokens(&tokens);
    liberarErrosLexicos(&erros);

    return houveErros ? EXIT_FAILURE : EXIT_SUCCESS;
}