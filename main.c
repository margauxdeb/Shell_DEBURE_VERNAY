//
//  main.c
//  MiniShell
//
//  Created by Margaux Debure on 30/03/2016.
//  Copyright © 2016 Margaux Debure. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <wait.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "consts.h"
#include "commands.h"
#include "functions.h"

int cpt = 0;

/* On stockera dans cette structure toutes les données relatives au Shell, pour les passer facilement
aux diverses fonctions */



/* La fonction create_process duplique le processus appelant et retourne
 le PID du processus fils ainsi créé */
pid_t CreerProcessus() {
    /* On crée une nouvelle valeur de type pid_t */
    pid_t pid;

    /* On fork() tant que l'erreur est EAGAIN */
    do
    {
        pid = fork();
    }
    while ((pid == -1) && (errno == EAGAIN));

    /* On retourne le PID du processus ainsi créé */
    return pid;
}

void AjouterJob(Minishell* monShell) {
    //TODO
    monShell->nbjobs++;
}

Job* GetAFuckingJob(Minishell* monShell, int id) {
    int i=0;
    for (int j=0;i<monShell->nbjobs && j<64;j++) {
        if (monShell->jobs[j].status && monShell->jobs[j].id == id) {
            return &monShell->jobs[j];
        }
        else {
            i++;
        }
    }
    return NULL;
}

void SupprimerJob(Minishell* monShell) {
    if (monShell->nbjobs) {
        //TODO
        //TODO

        //TODO TODO TODO, ...
        monShell->nbjobs--;
        if (!monShell->nbjobs) {
            monShell->jobCounter = 0;
        }
    }
}

void InsererHistorique(char* chaine, Minishell* monShell) {
    if (chaine != NULL && strlen(chaine)) {
        FILE* history = fopen(monShell->historyPath,"a");
        if (history != NULL) {
            fprintf(history,"%s\n",chaine);
            fclose(history);
        }
    }
}

int SaisirChaine(char *chaine, int longueur) {
    int verif = 0;
    char *positionEntree = NULL;

    if (fgets(chaine, longueur, stdin) != NULL)
    {
        verif = 1;
        positionEntree = strchr(chaine, '\n');
        if (positionEntree != NULL)
            *positionEntree = '\0';
    }
    return (verif);

}

int DecouperChaine(char* chaine, char** tabMots, char* delimiteurs) {
	char *token;
	int i=0;

	//Découper la chaîne selon les espaces
	token = strtok (chaine,delimiteurs);

	while (token != NULL)
	{
		tabMots[i]= token;
		token = strtok (NULL, delimiteurs);
		i++;
	}

	return i;
}

int getSousTableau(char** tab, char** result, int argc, int a, int b) {
    if (a >= 0 && a < b && b <= argc) {
        for (int i=a;i<b;i++) {
            result[i-a] = tab[i];
        }
        return b-a;
    }
    else {
        return 0;
    }
}

void chercherCommande(char* cmd) {
    char temp[LONGUEUR] = "/usr/bin/";
    strcat(temp,cmd);
    strcpy(cmd,temp);
}

int compterPipes(char** tabMots, int argc) {
    int special = 0;
    for (int i=0;i<argc;i++)
	if (!strcmp("|",tabMots[i])){
        special++;
	}
	return special;
}

int isSpecial(char** tabMots, int argc) {
	char delem[] = "<>|";
    for (int i=0;i<argc;i++)
	if (containsOneOf(delem,3,tabMots[i],strlen(tabMots[i]))) {
        return true;
	}
	return false;
}


void ExecuterCommandeDansFils(Minishell* monShell, char* cmdline, int in, int out) {
    if (in >= 0 || out >= 0) {
        printf("attempting:\n");
        if (in < 0) printf("in\tstdin\n"); else printf("in\t%d\n",in);
        if (out < 0) printf("out\tstdout\n"); else printf("out\t%d\n",out);
    }
    char** tabMots = malloc(64*sizeof(char*));
    int argc = DecouperChaine(cmdline,tabMots," ");

    // DETECT REDIRECTIONS

    int rin = detect("<",tabMots,argc);
    int rout= detect(">",tabMots,argc);
    int inFile = -1;
    int outFile = -1;

    if (rin > 0) {
        inFile = open(tabMots[rin+1],O_RDONLY);
        if (inFile >= 0) {
            printf("Should be reading from %s(%d)\n",tabMots[rin+1]);
            if (in >= 0) close(in);
            in = inFile;
        }
    }
    if (rout > 0) {
        outFile = open(tabMots[rout+1],O_WRONLY | O_CREAT, 0666);
        if (outFile >= 0) {
            printf("Should be writing in %s\n(%d)",tabMots[rout+1],outFile);
            if (out >= 0) close(out);
            out = outFile;
        }
    }
	pid_t pid;
	int known = true;
    if (!contains('/',tabMots[0],strlen(tabMots[0])) && !contains('.',tabMots[0],strlen(tabMots[0]))) {
        /*affichertabMots(tabMots);
        chercherCommande(tabMots[0]);
        affichertabMots(tabMots);
    */
        known = false;
    }
    //chercherChemin(tabMots[1],monShell);

    int status;
    pid = CreerProcessus();
    switch (pid)
    {
        //Si on a une erreur irrémédiable (ENOMEM dans notre cas)
        case -1:
            perror("fork");
        break;
        //Si on est dans le fils
        case 0:
            // pipe ?
            // redirection ?
            if (in >= 0)
                dup2(in,0);
            if (out >= 0)
                dup2(out,1);
            ExecuterCommande(monShell,argc,tabMots);
            if (in >= 0) close(in);
            if (out >=0) close(out);
            // printf("Never executed!\n");
            exit(0);
        break;
        //  Si on est dans le père
        default:
            if (!strcmp(tabMots[argc-1],"&"))
            {

            }
            else
                waitpid(pid, &status, 0);
            //printf("Son ended with status %d\n",status);
        break;
    }

    if (in >= 0)
        close(in);
    if (out >= 0)
        close(out);
}


int detect(char* str, char** tabMots, int argc) {
    for (int i=0;i<argc;i++) {
        if (!strcmp(str,tabMots[i]))
            return i;
    }
    return -1;
}

void InterpreterCommande(Minishell* monShell,char* cmdline, int nbpipes, int* myPipe) {
    printf("ic:%s\n",cmdline);
    char** args = calloc(64,sizeof(char*));
    int rin =  contains('<',cmdline,strlen(cmdline));
    int rout = contains('>',cmdline,strlen(cmdline));
    int argc = DecouperChaine(cmdline,args," ");
    rin = detect("<",args,argc);
    rout= detect(">",args,argc);
    FILE* inFile = NULL;
    FILE* outFile = NULL;
    int stdin_sub = -1;
    int stdout_sub = -1;
    if (rin > 0) {
        printf("(%d)redirection < ",rin);
        if ((inFile = fopen(args[rin+1],"r+")) != NULL) {
            printf("%s",args[rin+1]);
           // stdin_sub = dup(0);
            dup2(inFile,0);
        }
        printf("\n");
    }
    if (rout > 0) {
        printf("redirection > ");
        if ((outFile = fopen(args[rout+1],"r+")) != NULL) {
            printf("%s",args[rout+1]);
           // stdout_sub = dup(1);
            dup2(outFile,1);
        }
        printf("\n");
    }
    ExecuterCommande(monShell,argc,args);
    if (rin > 0) {
        if (inFile != NULL)
            fclose(inFile);
        //if (stdin_sub >= 0)
        //dup2(stdin,0);
    }
    if (rout > 0) {
        if (outFile != NULL)
            fclose(outFile);
        //if (stdout_sub >= 0)
        //dup2(stdout,1);
    }
    free(args);
}

void ExecuterCommande(Minishell* monShell, int argc, char** tabMots) {

	int known = true;
    if (!contains('/',tabMots[0],strlen(tabMots[0])) && !contains('.',tabMots[0],strlen(tabMots[0]))) {
        /*afficherArgs(tabMots);
        chercherCommande(tabMots[0]);
        afficherArgs(tabMots);
    */
        known = false;
    }

    if (!strcmp(tabMots[0], "exit")) {
        exit(0);
    }
    else if (!strcmp(tabMots[0], "cd"))
    {
        CommandeCD(tabMots[1], monShell);
    }
    else if (!strcmp(tabMots[0], "history"))
    {
        CommandeHistory(monShell);
    }
    else if (!strcmp(tabMots[0], "cat") && argc > 1)
    {
        char temp[LONGUEUR];
        strcpy(temp,tabMots[1]);
        chercherChemin(temp,monShell);
        CommandeCat(temp);
    }
    else if (!strcmp(tabMots[0], "touch")) {
        CommandeTouch(tabMots,monShell->repertoire);
    }
    else if (!strcmp(tabMots[0], "cp")) {
        if (argc > 2)
            CommandeCP(tabMots[1],tabMots[2]);
    }
    else if (!strcmp(tabMots[0], "ps")) {
        CommandePS();
    }
    else if (!strcmp(tabMots[0], "wait")) {
        CommandeWait(tabMots[1]);
    }
    else if (!strcmp(tabMots[0],"kill")) {
        if (argc > 1)
            CommandeKill(tabMots[1],tabMots[2]);
    }
    else if (!strcmp(tabMots[0],"jobs")) {
        CommandeJobs(monShell);
    }
    else if (!strcmp(tabMots[0],"fg")) {
        if (argc > 1)
            CommandeFG(monShell, tabMots[1]);
    }
    else if (!strcmp(tabMots[0],"bg")) {
        if (argc > 1)
            CommandeBG(monShell, tabMots[1]);
    }
    else if (!strcmp(tabMots[0],"pwd")) {
        printf("%s\n",monShell->repertoire);
    }
    else if (!strcmp(tabMots[0],"show") && argc > 1) {
        char temp[LONGUEUR];
        strcpy(temp,tabMots[1]);
        chercherChemin(temp,monShell);
        printf("%s\n",temp);
    }
    else if (known)
        execv(tabMots[0], tabMots);
    else {
        char temp[LONGUEUR];
        strcpy(temp,tabMots[0]);
        chercherCommande(temp);
        execv(temp,tabMots);
        printf("%s was not executed!\n",tabMots[0]);
    }
}

void InterpreterLigne(char* cmdline, Minishell* monShell) {
    char delim[] = "|";
    int havepipe = false;
    if (containsOneOf(delim,1,cmdline,strlen(cmdline))) {
        havepipe = true;
    }
    if (havepipe) {
        char** cmds = malloc(20*sizeof(char*));
        int nbpipes = contains('|',cmdline,strlen(cmdline));
        int idpipe = 0;
        int cmdc = DecouperChaine(cmdline,cmds,delim);
        if (cmdc <= nbpipes) {
            nbpipes = cmdc - 1;
        }
        int pipes[nbpipes][2];
        while (idpipe < nbpipes) {
            pipe(pipes[idpipe]); // on ouvre le pipe actuel
            printf("newpipe : r=%d\tw=%d\n",pipes[idpipe][0],pipes[idpipe][1]);
            if (!idpipe) {
                ExecuterCommandeDansFils(monShell,cmds[idpipe],-1,pipes[idpipe][PIPE_WRITE]);
                close(pipes[idpipe][PIPE_WRITE]);
            }
            else
                ExecuterCommandeDansFils(monShell,cmds[idpipe],pipes[idpipe-1][PIPE_READ],pipes[idpipe][PIPE_WRITE]);
            idpipe++;
        }
        if (idpipe < cmdc) {
                ExecuterCommandeDansFils(monShell,cmds[idpipe],pipes[idpipe-1][PIPE_READ],-1);
                close(pipes[idpipe-1][PIPE_READ]);
        }

        /*for (int i=0;i<nbpipes;i++) {
            for (int j=0;j<2;j++)
                if (pipes[i][j] >= 0) {
                    printf("%d;",pipes[i][j]);
                    close(pipes[i][j]);
                }
        }
        printf("\n");*/

        free(cmds);
    }
    else {
        ExecuterCommandeDansFils(monShell,cmdline,-1,-1);
    }
}

int main(void)
{

    /*FILE* log = fopen("./log","w");
    fclose(log);*/
	char *chaine;
	char nomUtilisateur [LONGUEUR];
	char nomHote        [LONGUEUR];
	int nombreMots = 20;
	char ** tabMots;
	char ** arguments;
	int boolExit = 0;

	char delimiteurs[] = " ,:;";

    Minishell monShell = {};

    chaine              = (char*) calloc(LONGUEUR,sizeof(char));
	tabMots             = (char**)malloc(nombreMots*sizeof(char*));

    getcwd(monShell.repertoire,sizeof(monShell.repertoire));
    strcpy(monShell.historyPath, monShell.repertoire);
    strcat(monShell.historyPath, "/minishell.history");
    gethostname(nomHote,sizeof(nomHote));
    getlogin_r(nomUtilisateur,sizeof(nomUtilisateur));

	while (!feof(stdin))
	{
		printf("%s@%s:%s> ", nomUtilisateur, nomHote, monShell.repertoire);
		//printf("> minishell > ");
		if (SaisirChaine(chaine, LONGUEUR) && (!feof(stdin)))
		{
            InsererHistorique(chaine, &monShell);
            InterpreterLigne( chaine, &monShell);
		}
	}

    free(chaine);
    free(tabMots);

	return (0);
}

