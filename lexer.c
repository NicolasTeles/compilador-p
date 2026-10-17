#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

char* copiaString(const char* texto){
    int tamanho = strlen(texto) + 1;
    char* copia = malloc(tamanho);

    if(copia != NULL){
        strcpy(copia, texto);
    }
}