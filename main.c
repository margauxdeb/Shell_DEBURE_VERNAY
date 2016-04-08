//
//  main.c
//  MiniShell
//
//  Created by Margaux Debure on 30/03/2016.
//  Copyright Â© 2016 Margaux Debure. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <wait.h>

#define LONGUEUR 2048
#define TAILLE_HISTORIQUE 100

/* On stockera dans cette structure toutes les donnÃ©es relatives au Shell, pour les passer facilement
aux diverses fonctions */
typedef struct minishell {
    char**  historique;
    char    repertoire[LONGUEUR];
    int     compteurHistorique;
} Minishell;

void showArgs(char**);
/* La fonction create_process duplique le processus appelant et retourne
 le PID du processus fils ainsi crÃ©Ã© */
pid_t CreerProcessus()
{
    /* On crÃ©e une nouvelle valeur de type pid_t */
    pid_t pid;

    /* On fork() tant que l'erreur est EAGAIN */
    do
    {
        pid = fork();
    }
    while ((pid == -1) && (errno == EAGAIN));

    /* On retourne le PID du processus ainsi crÃ©Ã© */
    return pid;
}

void InsererHistorique(char *chaine, Minishell* monShell)
{

    if (monShell->compteurHistorique < TAILLE_HISTORIQUE) {
        monShell->historique[monShell->compteurHistorique]=(char*)malloc(LONGUEUR*sizeof(char));
        strcpy(monShell->historique[monShell->compteurHistorique++], chaine);
    }

    else {
        char* temp = calloc(LONGUEUR,sizeof(char));
        for (int i=0;i<TAILLE_HISTORIQUE-1;i++) {
            strcpy(temp,monShell->historique[i]);
            strcpy(monShell->historique[i],monShell->historique[i+1]);
            strcpy(monShell->historique[i+1],temp);
        }
        strcpy(monShell->historique[monShell->compteurHistorique-1], chaine);
        free(temp);
    }
}

void CommandeHistory(char **historique, int compteurHistorique)
{
    int i;

    for (i=0; i<compteurHistorique; i++)
	printf("\t%d : %s \n", i+1, historique[i]);
}

int SaisirChaine(char *chaine, int longueur)
{
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

void CommandeCat(char* chaine)
{
	char caractere;
	FILE* monFichier;
	if ((monFichier = fopen(chaine,"r")) != NULL)
	{
		while ((caractere = fgetc(monFichier)) != EOF)
		{
			printf("%c",caractere);
		}
		printf("\n");
		fclose(monFichier);
	}
}

int DecouperChaine(char* chaine, char** tabMots)
{
	char *token;
	int i=0;

	//DÃ©couper la chaÃ®ne selon les espaces
	token = strtok (chaine," ");

	while (token != NULL)
	{
		tabMots[i]= token;
		token = strtok (NULL, " ,");
		i++;
	}

	return i;
}

void CommandeCD(char **tabMots, char *repertoire)
{
	DIR* dt;

	if(tabMots[1] == NULL)
	{
		strcpy(repertoire, "/");
	}
	else if(!strcmp(tabMots[1], "../") || !strcmp(tabMots[1], ".."))
	{
		if(strcmp(repertoire, "/"))
		{
			if(repertoire[strlen(repertoire)-1] == '/')
				repertoire[strlen(repertoire)-1] = 0;
			while(repertoire[strlen(repertoire)-1] != '/')
				repertoire[strlen(repertoire)-1] = 0;
		}
	}
	else if(!strcmp(tabMots[1], ".") || !strcmp(tabMots[1], "./"))
	{}
	else if(tabMots[1][0] != '/')
	{
		if((repertoire[strlen(repertoire)-1] == '/') && ((dt = opendir(tabMots[1])) != NULL))
		{
			strcat(repertoire, tabMots[1]);
			closedir(dt);
		}
	}
	else
	{
		if ((dt = opendir(tabMots[1])) != NULL)
		{
			strcat(repertoire, "/");
			strcat(repertoire, tabMots[1]);
			closedir(dt);
		}

		if(repertoire[strlen(repertoire)-1] != '/')
			strcat(repertoire, "/");
		else
		{
			if((dt = opendir(tabMots[1])) != NULL)
			{
				closedir(dt);
				strcpy(repertoire, tabMots[1]);
				if(repertoire[strlen(repertoire)-1] != '/')
					strcat(repertoire, "/");
			}
			else
				printf("%s: not a directory\n", tabMots[1]);
		}
	}
}

void InterpreterCommande(Minishell* monShell,int argc, char** tabMots) {

	pid_t pid;

    printf("argc=%d\n",argc);
    char** args = calloc(argc,sizeof(char*));

    for (int i=0;i<argc;i++) {
        args[i] = tabMots[i];
    }

    if (!strcmp(args[0], "cd"))
    {
        CommandeCD(args, monShell->repertoire);
    }
    else if (!strcmp(args[0], "history"))
    {
        CommandeHistory(monShell->historique, monShell->compteurHistorique);
    }
	else if (!strcmp(args[0], "cat"))
	{
		CommandeCat(args[1]);
	}
	else
	{
        printf("Unknow command ; trying to execv it ...\n");
        int status;
		pid = CreerProcessus();
		switch (pid)
		{
			//Si on a une erreur irrÃ©mÃ©diable (ENOMEM dans notre cas)
			case -1:
				perror("fork");
				return EXIT_FAILURE;
			break;
			//Si on est dans le fils
			case 0:
				if (execv(args[0], args) < 0)
				printf("Attained the unattainable ! \n");
				exit(0);
			break;
			//  Si on est dans le pÃ¨re
			default:
				waitpid(pid, &status, 0);
				printf("Son ended with status %d\n",status);
			break;
		}
	}
}


int main(void)
{
	char *chaine;
	int nombreMots = 20;
	char ** tabMots;
	int boolExit = 0;

    Minishell monShell = {.compteurHistorique = 0};

    chaine      = (char*) calloc(LONGUEUR,sizeof(char));
	tabMots     = (char**)malloc(nombreMots*sizeof(char*));
	monShell.historique = (char**)malloc(TAILLE_HISTORIQUE*sizeof(char*));

	// Cette fonction fonctionne bien avec une chaine dÃ©clarÃ©e en statique, mais pas avec un pointeur (comme fait prÃ©cÃ©demment)
    getcwd(monShell.repertoire,sizeof(monShell.repertoire));

	while (!feof(stdin)&&(!boolExit))
	{
		printf("> [%s]", monShell.repertoire);
		if (SaisirChaine(chaine, LONGUEUR) && (!feof(stdin)))
		{
			int argc = DecouperChaine(chaine, tabMots);

			if (!strcmp(tabMots[0], "exit"))
				boolExit = 1;
			else {
                InterpreterCommande(&monShell, argc, tabMots);
                InsererHistorique(chaine, &monShell);
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



void showArgs(char** tabMots) {
        int i = 0;
        printf("Your arguments :\n");
        while (tabMots[i] != NULL) {
            printf("\n\t%d : %s(%d)",i,tabMots[i],strlen(tabMots[i]));
            i++;
        }
        printf("\n");
}
