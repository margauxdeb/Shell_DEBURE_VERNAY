#ifndef FUNCTIONS_H
#define FUNCTIONS_H

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

pid_t getppidlive(char*);
int detect(char* str, char** tabMots, int argc);

void chercherCommande(char* cmd);
void removeElement(int ind, char** tab, int argc);
int removeRedirections(char** args, int argc);

#endif
