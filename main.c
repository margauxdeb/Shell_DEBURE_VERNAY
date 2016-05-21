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


void ExecuterCommande(Minishell* monShell, int argc, char** tabMots);

int detect(char* str, char** tabMots, int argc) {
    for (int i=0;i<argc;i++) {
        if (!strcmp(str,tabMots[i]))
            return i;
    }
    return -1;
}

void InterpreterCommande(Minishell* monShell, unsigned int argc, char** tabMots, int nbpipes, int* myPipe) {
    if (!argc) return;
    char cstmlog[40];
    sprintf(cstmlog,"%s%d","./logs/log",nbpipes);
	FILE* log = fopen(cstmlog,"a");
	fprintf(log,"#init:%d\n",nbpipes);
    fprintf(log,"%d:%d:%d:interpreting %s\n",nbpipes,nbpipes,myPipe != NULL,tabMots[0]);
    //showArgs(tabMots,argc);
    fprintf(log,"Handling (%d) : ",argc);
    for (int i=0;i<argc;i++) {
        fprintf(log,"%s ",tabMots[i]);
    }
    fprintf(log,"\n");
	int i;
	// pipes ou redirections

    int stdout_sub = dup(1); // on sauvegarde une copie de stdout pour le remettre en place en suite

    if (nbpipes && myPipe != NULL) {
        char** temp = calloc(argc,sizeof(char*));
        int tmpfile = 0;
        int indice = 0;
        if (indice = detect("|",tabMots,argc)) {
            int newargc = getSousTableau(tabMots,temp,argc,0,indice);
            fprintf(log,"new sub table of %d : (afore)\n",newargc);
             for (int i=0;i<newargc;i++) {
                fprintf(log,"%s ",temp[i]);
            }
            fprintf(log,"\n");
            dup2(myPipe[PIPE_WRITE],1);
           /* fclose(log);
            log = NULL;*/
            InterpreterCommande(monShell,newargc,temp, nbpipes, NULL);

            //if (!(nbpipes-1))
                dup2(stdout_sub,1);
            dup2(myPipe[PIPE_READ],0);
            newargc = getSousTableau(tabMots,temp,argc,indice+1,argc);
            fprintf(log,"new sub table of %d : (after)\n",newargc);
            for (int i=0;i<newargc;i++) {
                fprintf(log,"%s ",temp[i]);
            }
            fprintf(log,"\n");

            /*fclose(log);
            log = NULL;*/
            InterpreterCommande(monShell,newargc,temp, nbpipes-1,myPipe);
        }
        /*else if ((indice = detect(">",tabMots,argc)) > 0) {
            if (indice < argc-1) {
                tmpfile = open(tabMots[indice+1],"a");
                if (tmpfile != NULL) {
                    dup2(tmpfile,1);
                    // EXEC
                    close(tmpfile);
                }
            }
        }*/

        else {
            fprintf(log,"do we even make it here ?\n");
            ExecuterCommande(monShell,argc,tabMots);
        }
        free(temp);
    }
    else {
        if (!nbpipes) {
            dup2(stdout_sub,1);
            if (log != NULL)
                fprintf(log,"We should now be on stdout because executing %s\n",tabMots[0]);
            else
                fprintf(log,"We should be on stdout\n");
        }
        ExecuterCommande(monShell,argc,tabMots);
	}
    if (log != NULL) {
        fprintf(log,"%s ended\n",tabMots[0]);
        fclose(log);
    }
}

void ExecuterCommande(Minishell* monShell, int argc, char** tabMots) {

	pid_t pid;
	int known = true;

    if (!strcmp(tabMots[0], "cd"))
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
	else
	{
        if (!contains('/',tabMots[0],strlen(tabMots[0])) && !contains('.',tabMots[0],strlen(tabMots[0]))) {
            /*afficherArgs(tabMots);
            chercherCommande(tabMots[0]);
            afficherArgs(tabMots);
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
                if (known)
                    execv(tabMots[0], tabMots);
                else {
                    char temp[LONGUEUR];
                    strcpy(temp,tabMots[0]);
                    chercherCommande(temp);
                    execv(temp,tabMots);
                }
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
	}
}

void InterpreterLigne(Minishell* monShell, unsigned int argc, char ** tabMots) {
    int nbpipes = compterPipes(tabMots,argc);
    int sub_stdin = dup(0);
    int sub_stdout = dup(1);
    int sub_stderr = dup(2);
    if (nbpipes) {
        printf("Found some pipes !(%d)\n",nbpipes);
        int thisPipe[2];
        pipe(thisPipe);
        InterpreterCommande(monShell,argc,tabMots,nbpipes,thisPipe);
        dup2(sub_stdout,1);
        dup2(sub_stdin,0);
        dup2(sub_stderr,2);
        close(thisPipe[1]);
        char buffer[60];
        printf("Residu du pipe : ");
        //while (!feof(thisPipe[0])) {
            /*read(thisPipe[0],buffer,60*sizeof(char));
            printf("%s",buffer);*/
        //}*/
        printf("\n");
        close(thisPipe[0]);
    }
    else if (isSpecial(tabMots,argc)) {
        InterpreterCommande(monShell,argc,tabMots,0,NULL);
        dup2(sub_stdout,1);
        dup2(sub_stdin,0);
        dup2(sub_stderr,2);
    }
    else {
        ExecuterCommande(monShell,argc,tabMots);
    }
}


/* TODO:
    il = interpreter_ligne
    ic = interpreter_commande
    ec = executer_commande

    dans le main :  on sépare les commandes avec ';' on appelle il sur chaque element
    dans il :       on sépare les commandes avec '|' on appelle ic sur chaque element + gestion des pipes
    dans ic :       gestion des redirections '>','<' on appelle ec
*/

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
	monShell.historique = (char**)malloc(TAILLE_HISTORIQUE*sizeof(char*));

    getcwd(monShell.repertoire,sizeof(monShell.repertoire));
    strcpy(monShell.historyPath, monShell.repertoire);
    strcat(monShell.historyPath, "/minishell.history");
    gethostname(nomHote,sizeof(nomHote));
    getlogin_r(nomUtilisateur,sizeof(nomUtilisateur));

	while (!feof(stdin)&&(!boolExit))
	{
		printf("%s@%s:%s> ", nomUtilisateur, nomHote, monShell.repertoire);
		//printf("> minishell > ");
		if (SaisirChaine(chaine, LONGUEUR) && (!feof(stdin)))
		{
            InsererHistorique(chaine, &monShell);
            int argc = DecouperChaine(chaine, tabMots, delimiteurs);
			if (argc)
			{
                arguments = calloc(argc,sizeof(char*));
                for (int i=0;i<argc;i++) {
                    arguments[i] = calloc(LONGUEUR,sizeof(char));
                    strcpy(arguments[i],tabMots[i]);
                }
                if (!strcmp(tabMots[0], "exit"))
                    boolExit = 1;
                else {
                    InterpreterLigne(&monShell, argc, arguments);
                    dup2(stdout,1);
                    dup2(stdin,0);
                }
                for (int i=0; i<argc;i++)
                    free(arguments[i]);
                free(arguments);
            }
		}
	}

    free(chaine);
    free(tabMots);
    for (int i=0;i<monShell.compteurHistorique;i++)
        free(monShell.historique[i]);
    free(monShell.historique);

	return (0);
}

