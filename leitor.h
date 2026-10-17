#ifndef LEITOR_H
#define LEITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    FILE *arquivo;
    int linha;
} CodigoFonte;

int abrirCodigoFonte(CodigoFonte *codigo, const char *nomeArquivo);

void fecharCodigoFonte(CodigoFonte *codigo);

int lerCaractere(CodigoFonte *codigo);

void devolverCaractere(CodigoFonte *codigo, int caractere);

int olharCaractere(CodigoFonte *codigo);

#endif