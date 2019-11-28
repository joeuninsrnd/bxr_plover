#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	char *hex;
	char *braille;
} Braille;

char remplacerbraille (Braille carac[], int tailleCarac, FILE *out, char *find_text, long lSize)
{
	int i = 0, now_replacing = 0,j,converDansBalise,k;
	while (lSize != i)
	{
		for (i = 0; find_text[i] != '\0';) 
		{
			converDansBalise=1;
			if (strncmp(&find_text[i], "|(", 2) == 0)
			{
				now_replacing = 1;
				i += 2;
				fprintf(out, "|(");
				continue;
			}
			if (strncmp(&find_text[i], "|)", 2) == 0)
			{
				now_replacing = 0;
				i += 2;
				fprintf(out, "|)");
				continue;
			}
			if (now_replacing == 1)
			{
				for (j=0;j<tailleCarac;j++)
				{
					if (strncmp(&find_text[i], carac[j].hex, strlen(carac[j].hex)) == 0)
					{
						for (k=0;k<strlen(carac[j].braille);k++)
						{
							fputc(carac[j].braille[k],out);
						}
						i+= strlen(carac[j].hex);
						converDansBalise=0;
						break;
					}
				}
				if (converDansBalise == 1){ fputc(find_text[i], out); i++;}
			}
			else
			{
				fputc(find_text[i], out);
				i++;
			}
		}
	}
	memset (find_text, 0, lSize);
	fread (find_text, 1, lSize, out);

	return *find_text;
}

char func_pdf2txt(char *find_text, char *filepath, long lSize)
{
	char nomFichier[50],nouveauFichier[50],commande[60]="pdftotext -enc UTF-8 ";
    Braille carac[] = {
		{"\xe2\x88\x9e","¤c"}, // ∞
            {"\x3d","\""}, //=
            {"\xe2\x88\xaa","¸!"}, // ∪
            {"\x2b","!"}, //+
            {"\xc3\x97","*"}, //×
            {"\x3e","@"}, // >
            {"\x3c","2"}, // <
            {"\x5d","à"}, // [
            {"\x5b","ù"}, // ]
            {"\xe2\x8b\x82","¤!"}, //⋂
            {"\xe2\x89\x85","null"}, //≅
            {"\x3d\x3e","¤2"}, // <=
            {"\xe2\x88\x9a","@"},// √
            {"\xe2\x88\xab","ç"}, // ∫
            {"\xe2\x88\xa7","é"}, // ∧
            {"\xe2\x88\xa8","¸"}, // ∨
            {"\xe2\x88\x8b","¤\""}, // ∋
            {"\xe2\x88\x85","¤0"}, // ∅
            {"\xe2\x88\xa5","¸8"}, // ∥
    };
	int tailleCarac=19;
    FILE *out;

	strcat(commande, filepath);
	system(commande);

	strcpy(nouveauFichier,nomFichier);
    out = fopen(nouveauFichier, "w");
	if (out==NULL) return 0;
	
	if (remplacerbraille (carac, tailleCarac, out, find_text, lSize)==-1)
	fclose(out);

	return *find_text;
}
