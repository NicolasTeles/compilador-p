#include "leitor.h"


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