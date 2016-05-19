#ifndef COMMANDS_H
#define COMMANDS_H

#include "consts.h"

void CommandeCD(char** args, char* repertoire);
void CommandeTouch(char** args, char* repertoire);
void CommandeCat(char* chaine);
void CommandeWait(char* arg);
void CommandePS();
void CommandeKill(char*,char*);
void CommandeFG(Minishell*,char*);
void CommandeBG(Minishell*,char*);
void CommandeJobs(Minishell*);
void CommandeHistory(Minishell* monShell);
int CommandeCP(const char* srcPath, const char* destPath);

#endif // COMMANDS_H
