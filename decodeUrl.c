#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    char s1[9];
    strcpy(s1, "06%3A30");  // String com %3A representando ":" no formato URL
    printf("Before formating: %s\n", s1);

    // Alocando memória para a nova string
    char* s2 = (char*)malloc(strlen(s1) + 1);  // +1 para incluir o caractere nulo '\0'
    if (s2 != NULL)
    {
        int j = 0;  // índice para s2
        for (int i = 0; i < strlen(s1); i++)
        {
            // Substituindo o '%3A' por ':'
            if (i < strlen(s1) - 2 && s1[i] == '%' && s1[i+1] == '3' && s1[i+2] == 'A') {
                s2[j++] = ':';  // Substitui %3A por ':'
                i += 2;  // Avança 2 posições para pular o "%3A"
            } else {
                s2[j++] = s1[i];  // Copia o restante dos caracteres
            }
        }
        s2[j] = '\0';  // Não se esqueça de adicionar o '\0' no final da string

        printf("After formating (new str): %s\n", s2);
        free(s2);  // Libera a memória alocada
    } else {
        printf("Memory allocation failed\n");
    }

    return 0;
}
