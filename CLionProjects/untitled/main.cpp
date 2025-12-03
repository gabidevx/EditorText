#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE* fptr;
    FILE* fptrout;
    int numar;

    // Deschidem fisierul text pentru citire
    fptr = fopen("text.txt", "r");
    if (fptr == NULL)
    {
        perror("Eroare la deschiderea text.txt");
        return 1;
    }

    // Deschidem fisierul binar pentru scriere
    fptrout = fopen("fis2.bin", "wb");
    if (fptrout == NULL)
    {
        perror("Eroare la deschiderea fis2.bin");
        fclose(fptr);
        return 1;
    }

    // Citim fiecare numar intreg din text.txt si il scriem binar
    while (fscanf(fptr, "%d", &numar) == 1)
    {
        size_t rezultat = fwrite(&numar, sizeof(int), 1, fptrout);
        if (rezultat != 1)
        {
            perror("Eroare la scriere in fis2.bin");
            fclose(fptr);
            fclose(fptrout);
            return 1;
        }
    }

    printf("Conversie reusita!\n");

    fclose(fptrout);
    fclose(fptr);
    return 0;
}
