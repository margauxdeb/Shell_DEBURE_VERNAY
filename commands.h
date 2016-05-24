#ifndef COMMANDS_H
#define COMMANDS_H

#include "consts.h"

void CommandeCD(char* args, Minishell* monShell);
void CommandeCDx(char**,Minishell*);
void CommandeTouch(char** args, char* repertoire);
void CommandeCat(int argc, char** args);
void CommandeWait(char* arg);
void CommandePS(Minishell*);
void CommandeKill(char*,char*);
void CommandeFG(Minishell*,char*);
void CommandeBG(Minishell*,char*);
void CommandeJobs(Minishell*);
void CommandeHistory(Minishell* monShell,int, char**);
int CommandeCP(const char* srcPath, const char* destPath);

#endif // COMMANDS_H
