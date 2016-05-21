#ifndef FUNCTIONS_H
#define FUNCTIONS_H

/* Fonctions à renommer, voire à réviser */
void afficherArgs(char**);
void showArgs(char**,int);
int estRepertoire(char*);
void chercherChemin(char*,Minishell*);
void popChemin(char*);
char dernier(char*);
char premier(char*);
char* setdernier(char*);
char* setpremier(char*);

void adapt(char*);
int contains(char,char*,int);
int containsOneOf(char*,int,char*,int);
int fileExists(const char*);
int isAPID(char*);

#endif
