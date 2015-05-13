#ifndef STDFUNC_H
#define STDFUNC_H

#include <stdio.h>

// ======================================
// stdfunc.h : STANDARD UTILITY FUNCTIONS
// ======================================

double      STD_Pow10(int p);
char      *STD_StrTrim(char *str);
int         STD_Sscanf(char *line,char *formatString,...);
long        STD_FileLength(FILE *fp);

char       *STD_Strupr(char *n);
char       *STD_Strlwr(char *n);

int         STD_IsDir(char *filename);

#endif
