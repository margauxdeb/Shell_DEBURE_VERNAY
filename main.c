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

#define LONGUEUR 2048

/* On stockera dans cette structure toutes les données relatives au Shell, pour les passer facilement
aux diverses fonctions */
typedef struct minishell {
    char**  historique;
    char    repertoire[LONGUEUR];
    int     compteurHistorique;
} Minishell;


/* La fonction create_process duplique le processus appelant et retourne
 le PID du processus fils ainsi créé */
pid_t CreerProcessus()
{
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

void InsererHistorique(char *chaine, Minishell* monShell)
{
    monShell->historique[monShell->compteurHistorique]=(char*)malloc(LONGUEUR*sizeof(char));
    strcpy(monShell->historique[monShell->compteurHistorique++], chaine);
}

void CommandeHistory(char **historique, int compteurHistorique)
{
    int i;

    for (i=0; i<compteurHistorique; i++)
	printf("%d : %s \n", i+1, historique[i]);
}

int SaisirChaine(char *chaine, int longueur)
{
    int verif=0;
    char *positionEntree = NULL;

    if (fgets(chaine, longueur, stdin) != NULL)
    {
	verif=1;
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

void DecouperChaine(char* chaine, char** tabMots)
{
	char *token;
	int i=0;

	//Découper la chaîne selon les espaces
	token = strtok (chaine," ");

	while (token != NULL)
	{
		tabMots[i]= token;
		token = strtok (NULL, " ,");
		i++;
	}
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

void InterpreterCommande(Minishell* monShell,char** tabMots) {

	pid_t pid;

    if (!strcmp(tabMots[0], "cd"))
    {
        CommandeCD(tabMots, monShell->repertoire);
    }
    else if (!strcmp(tabMots[0], "history"))
    {
        CommandeHistory(monShell->historique, monShell->compteurHistorique);
    }
	else if (!strcmp(tabMots[0], "cat"))
	{
		CommandeCat(tabMots[1]);
	}
	else
	{
		pid = CreerProcessus();
		switch (pid)
		{
			//Si on a une erreur irrémédiable (ENOMEM dans notre cas)
			case -1:
				perror("fork");
				return EXIT_FAILURE;
			break;
			//Si on est dans le fils
			case 0:
				execv(tabMots[0], tabMots);
				exit(0);
			break;
			//  Si on est dans le père
			default:
				waitpid(-1, 0, 0);
			break;
		}
	}
}


int main(void)
{
	char *chaine;
	//char repertoire[LONGUEUR];
	int nombreMots = 20;
	char ** tabMots;
	//char ** historique;
	//int compteurHistorique=0;
	int boolExit=0;

    Minishell monShell = {.compteurHistorique = 0};

    chaine      = (char*) calloc(LONGUEUR,sizeof(char));
	tabMots     = (char**)malloc(nombreMots*sizeof(char*));
	//repertoire  = (char*) malloc(LONGUEUR*sizeof(char));
	monShell.historique  = (char**)malloc(20*sizeof(char*));

	//strcpy(repertoire, "/");

	// Cette fonction fonctionne bien avec une chaine déclarée en statique, mais pas avec un pointeur (comme fait précédemment)
    getcwd(monShell.repertoire,sizeof(monShell.repertoire));

	while (!feof(stdin)&&(!boolExit))
	{
		printf("> [%s]", monShell.repertoire);
		if (SaisirChaine(chaine, LONGUEUR) && (!feof(stdin)))
		{
			InsererHistorique(chaine, &monShell);
			//monShell.compteurHistorique++;

			DecouperChaine(chaine, tabMots);

			if (!strcmp(tabMots[0], "exit"))
				boolExit = 1;
			else {
                InterpreterCommande(&monShell,tabMots);
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
