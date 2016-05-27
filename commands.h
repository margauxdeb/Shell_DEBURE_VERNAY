#ifndef COMMANDS_H
#define COMMANDS_H

#include "consts.h"

/* effectue le decoupage de chaine, utilise chdir pour modifier le repertoire de travail */
void commandeCD(char* args, Minishell* monShell);

void commandeTouch(int argc, char** args, char* repertoire);

void commandeCat(int argc, char** args);

void commandeWait(char* arg);

/*
liste le contenu du repertoire /proc
*/
void commandePS(Minishell*);

/* interprete l'argument ("est-ce un nombre ou qqch comme -SIGKILL ?") et
envoie le signal correspondant */
void commandeKill(char*,char*);

/* WIP */
void commandeFG(Minishell*,char*);

/* WIP */
void commandeBG(Minishell*,char*);

/* (WIP)
liste les jobs du minishell
*/
void commandeJobs(Minishell*);

/* gere :
    history     affiche l'historique
    history n   affiche les n dernieres lignes
    history !n  execute la n-ieme ligne
    !n          execute la n-ieme ligne
*/
void commandeHistory(Minishell* monShell,int, char**);

int commandeCP(const char* srcPath, const char* destPath);

#endif // COMMANDS_H
