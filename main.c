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

void tryAndFindOut() {
    char* path = getenv("PATH");
    printf("PATH:%s\n",path);
}

pid_t getppidlive(char* pid) {
    char path[LONGUEUR];
    sprintf(path,"/proc/%s/stat",pid);
    FILE* there = fopen(path,"r");
    if (there != NULL) {
        fgets(path,1024,there);
        char** tabs = calloc(64,sizeof(char*));
        DecouperChaine(path,tabs," ");
        pid_t ppid = atoi(tabs[3]);
        free(tabs);
        fclose(there);
        return ppid;
    }
    return -1;
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
void chercherCommande(char* cmd) {
    char* path = getenv("PATH");
    char** paths = calloc(32, sizeof(char*));
    int nbpaths = DecouperChaine(path,paths,":");
    char temp[LONGUEUR];
    for (int i=0;i<nbpaths;i++) {
        strcpy(temp,paths[i]);
        strcat(temp,"/");
        strcat(temp,cmd);
        if (fileExists(temp) || estRepertoire(temp)) {
            strcpy(cmd,temp);
            return;
        }
    }
}

void removeElement(int ind, char** tab, int argc) {
    if (argc - ind > 0) {
        for (int i=ind;i<argc-1;i++) {
            strcpy(tab[i],
            tab[i+1]);
        }
        tab[argc-1] = NULL;
    }
}

int removeRedirections(char** args, int argc) {
    printf("before:\n");
    afficherArgs(args);

    for (int i=0;i<argc;i++) {
        if (!strcmp(args[i],">") || !strcmp(args[i],"<")) {
            removeElement(i,args,argc);
            argc--;
            if (i < argc) {
                removeElement(i,args,argc);
                argc--;
                i--;
            }
            i--;
        }
    }

    printf("after:\n");
    afficherArgs(args);
    return argc;
}

void ExecuterCommande(Minishell* monShell, char* cmdline, int in, int out) {
    char** tabMots = malloc(64*sizeof(char*));
    int argc = DecouperChaine(cmdline,tabMots," ");
    if (!strcmp(tabMots[0], "exit")) {
        exit(0);
    }
    else if (!strcmp(tabMots[0], "cd"))
    {
        CommandeCD(tabMots[1], monShell);
    }
    else if (!strcmp(tabMots[0], "wait")) {
        CommandeWait(tabMots[1]);
    }
    else {
        if (in >= 0 || out >= 0) {
            printf("attempting:\n");
            if (in < 0) printf("in\tstdin\n"); else printf("in\t%d\n",in);
            if (out < 0) printf("out\tstdout\n"); else printf("out\t%d\n",out);
        }

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
        if (in >= 0 || out >= 0) argc = removeRedirections(tabMots,argc);
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
                ExecuterCommandeDansFils(monShell,argc,tabMots);
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
}


int detect(char* str, char** tabMots, int argc) {
    for (int i=0;i<argc;i++) {
        if (!strcmp(str,tabMots[i]))
            return i;
    }
    return -1;
}

void ExecuterCommandeDansFils(Minishell* monShell, int argc, char** tabMots) {

	int known = true;
    if (!contains('/',tabMots[0],strlen(tabMots[0])) && !contains('.',tabMots[0],strlen(tabMots[0]))) {
        /*afficherArgs(tabMots);
        chercherCommande(tabMots[0]);
        afficherArgs(tabMots);
    */
        known = false;
    }

    if (!strcmp(tabMots[0], "history") || tabMots[0][0] == '!')
    {
        CommandeHistory(monShell,argc,tabMots);
    }
    else if (!strcmp(tabMots[0], "cat"))
    {
        CommandeCat(argc,tabMots);
    }
    else if (!strcmp(tabMots[0], "touch")) {
        CommandeTouch(tabMots,monShell->repertoire);
    }
    else if (!strcmp(tabMots[0], "cp")) {
        if (argc > 2)
            CommandeCP(tabMots[1],tabMots[2]);
    }
    else if (!strcmp(tabMots[0], "ps")) {
        CommandePS(monShell);
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
    else if (!strcmp(tabMots[0],"try") && argc > 1) {
        getppidlive(tabMots[1]);
    }
    else if (known)
        execv(tabMots[0], tabMots);
    else {
        char temp[LONGUEUR];
        strcpy(temp,tabMots[0]);
        chercherCommande(temp);
        //printf("RESULT:%s\n",temp);
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
                ExecuterCommande(monShell,cmds[idpipe],-1,pipes[idpipe][PIPE_WRITE]);
                close(pipes[idpipe][PIPE_WRITE]);
            }
            else
                ExecuterCommande(monShell,cmds[idpipe],pipes[idpipe-1][PIPE_READ],pipes[idpipe][PIPE_WRITE]);
            idpipe++;
        }
        if (idpipe < cmdc) {
                ExecuterCommande(monShell,cmds[idpipe],pipes[idpipe-1][PIPE_READ],-1);
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
        ExecuterCommande(monShell,cmdline,-1,-1);
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
	monShell.pid = getpid();
    printf("PID:%lu\n",monShell.pid);
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

