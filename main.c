//
//  main.c
//  MiniShell
//

// Functions are explained in associated header files.

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

/* s'execute au sein d'un processus issu d'un fork */
void executerCommandeDansFils(Minishell* monShell, int argc, char** tabMots) {

	int known = (contains('/',tabMots[0],strlen(tabMots[0])));

    if (!strcmp(tabMots[0], "history") || tabMots[0][0] == '!')
    {
        commandeHistory(monShell,argc,tabMots);
    }
    else if (!strcmp(tabMots[0], "cat"))
    {
        commandeCat(argc,tabMots);
    }
    else if (!strcmp(tabMots[0], "touch")) {
        commandeTouch(argc, tabMots,monShell->repertoire);
    }
    else if (!strcmp(tabMots[0], "cp")) {
        if (argc > 2)
            commandeCP(tabMots[1],tabMots[2]);
    }
    else if (!strcmp(tabMots[0], "ps")) {
        commandePS(monShell);
    }
    else if (!strcmp(tabMots[0],"kill")) {
        if (argc > 1)
            commandeKill(tabMots[1],tabMots[2]);
    }
    else if (!strcmp(tabMots[0],"jobs")) {
        commandeJobs(monShell);
    }
    else if (!strcmp(tabMots[0],"fg")) {
        if (argc > 1)
            commandeFG(monShell, tabMots[1]);
    }
    else if (!strcmp(tabMots[0],"bg")) {
        if (argc > 1)
            commandeBG(monShell, tabMots[1]);
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
        afficherArgs(tabMots);
        execv(temp,tabMots);
        //printf("%s was not executed!\n",temp);
    }
}

/* cree le processus fils, et redefinit stdin et stdout pour celui-ci */
void executerCommande(Minishell* monShell, char* cmdline, int in, int out) {
    char** tabMots = malloc(64*sizeof(char*));
    int argc = decouperChaine(cmdline,tabMots," ");
    int shouldiwait = strcmp(tabMots[argc-1],"&");
    if (!shouldiwait) {
        removeElement(argc-1,tabMots,argc);
        argc--;
    }
    if (!strcmp(tabMots[0], "exit")) {
        exit(0);
    }
    else if (!strcmp(tabMots[0], "cd"))
    {
        commandeCD(tabMots[1], monShell);
    }
    else if (!strcmp(tabMots[0], "wait")) {
        commandeWait(tabMots[1]);
    }
    else {
        /*if (in >= 0 || out >= 0) {
            printf("attempting:\n");
            if (in < 0) printf("in\tstdin\n"); else printf("in\t%d\n",in);
            if (out < 0) printf("out\tstdout\n"); else printf("out\t%d\n",out);
        }*/

        // DETECT REDIRECTIONS

        int rin = detect("<",tabMots,argc);
        int rout= detect(">",tabMots,argc);
        int inFile = -1;
        int outFile = -1;

        if (rin > 0) {
            inFile = open(tabMots[rin+1],O_RDONLY);
            if (inFile >= 0) {
                printf("Should be reading from %s(%d)\n",tabMots[rin+1],inFile);
                if (in >= 0) close(in);
                in = inFile;
            }
        }
        if (rout > 0) {
            outFile = open(tabMots[rout+1],O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (outFile >= 0) {
                printf("Should be writing in %s(%d)\n",tabMots[rout+1],outFile);
                if (out >= 0) close(out);
                out = outFile;
            }
        }
        if (in >= 0 || out >= 0) argc = removeRedirections(tabMots,argc);
        int status;
        pid_t pid = creerProcessus();
        switch (pid)
        {
            case -1:
                perror("fork");
            break;
            case 0:
                if (in >= 0)
                    dup2(in,0);
                if (out >= 0)
                    dup2(out,1);
                executerCommandeDansFils(monShell,argc,tabMots);
                if (in >= 0) close(in);
                if (out >=0) close(out);
                exit(0);
            break;
            default:
                if (shouldiwait)
                    waitpid(pid, &status, 0);
            break;
        }

        if (in >= 0)
            close(in);
        if (out >= 0)
            close(out);
    }
}

/* lit la ligne de commande et determine l'usage de pipes */
void interpreterLigne(char* cmdline, Minishell* monShell) {
    char delim[] = "|";
    int havepipe = false;
    if (containsOneOf(delim,1,cmdline,strlen(cmdline))) {
        havepipe = true;
    }
    if (havepipe) {
        char** cmds = malloc(20*sizeof(char*));
        int nbpipes = contains('|',cmdline,strlen(cmdline));
        int idpipe = 0;
        int cmdc = decouperChaine(cmdline,cmds,delim);
        if (cmdc <= nbpipes) {
            nbpipes = cmdc - 1;
        }
        int pipes[nbpipes][2];
        while (idpipe < nbpipes) {
            pipe(pipes[idpipe]); // on ouvre le pipe actuel
            //printf("newpipe : r=%d\tw=%d\n",pipes[idpipe][0],pipes[idpipe][1]);
            if (!idpipe) {
                executerCommande(monShell,cmds[idpipe],-1,pipes[idpipe][PIPE_WRITE]);
                close(pipes[idpipe][PIPE_WRITE]);
            }
            else
                executerCommande(monShell,cmds[idpipe],pipes[idpipe-1][PIPE_READ],pipes[idpipe][PIPE_WRITE]);
            idpipe++;
        }
        if (idpipe < cmdc) {
                executerCommande(monShell,cmds[idpipe],pipes[idpipe-1][PIPE_READ],-1);
                close(pipes[idpipe-1][PIPE_READ]);
        }

        free(cmds);
    }
    else {
        executerCommande(monShell,cmdline,-1,-1);
    }
}
int main(void)
{
	char *chaine;
	char nomUtilisateur [LONGUEUR];
	char nomHote        [LONGUEUR];
	int nombreMots = 20;
	char ** tabMots;
	int boolExit = 0;

	char delimiteurs[] = " ,:;";

    Minishell monShell = {};

    chaine              = (char*) calloc(LONGUEUR,sizeof(char));
	tabMots             = (char**)malloc(nombreMots*sizeof(char*));
	monShell.pid = getpid();
    getcwd(monShell.repertoire,sizeof(monShell.repertoire));
    strcpy(monShell.historyPath, monShell.repertoire);
    strcat(monShell.historyPath, "/minishell.history");
    gethostname(nomHote,sizeof(nomHote));
    getlogin_r(nomUtilisateur,sizeof(nomUtilisateur));

	while (!feof(stdin))
	{
        char there[LONGUEUR];
        strcpy(there,monShell.repertoire);
        int dirc = decouperChaine(there,tabMots,"/");
        if (dirc) strcpy(there,tabMots[dirc-1]);
		printf("%s@%s:%s> ", nomUtilisateur, nomHote, there);
		if (saisirChaine(chaine, LONGUEUR) && (!feof(stdin)))
		{
            insererHistorique(chaine, &monShell);
            interpreterLigne( chaine, &monShell);
		}
	}

    free(chaine);
    free(tabMots);

	return (0);
}

