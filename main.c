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
#define true 1
#define false 0

#define LONGUEUR 2048
#define TAILLE_HISTORIQUE 100

/* On stockera dans cette structure toutes les données relatives au Shell, pour les passer facilement
aux diverses fonctions */
typedef struct minishell {
    char**  historique;
    char    repertoire[LONGUEUR];
    int     compteurHistorique;
} Minishell;


/* Fonctions à renommer, voire à réviser */
void afficherArgs(char**);
int estRepertoire(char*);
void popChemin(char*);
char dernier(char*);
char premier(char*);
char* setdernier(char*);
char* setpremier(char*);

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
    if (monShell->compteurHistorique < TAILLE_HISTORIQUE) {
        monShell->historique[monShell->compteurHistorique] = (char*) malloc (LONGUEUR * sizeof(char));
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

/* Ajoute un '/' à la fin de la chaine */
void adapt(char* s) {
    if (s[strlen(s)-1] != '/') {
        printf("%s",s);
        strcat(s,"/");
        printf(" changed to %s\n",s);
    }
}

int CommandeCP(const char* srcPath, const char* destPath) {

    char src_path[200];
    char dest_path[200];

    memcpy(src_path,srcPath,strlen(srcPath)+1);
    memcpy(dest_path,destPath,strlen(destPath)+1);

    void* buffer = calloc(50,sizeof(void));
    int src = open(src_path,O_RDONLY);
    int result = 0;

    if (src != -1)  {

        struct stat buff;
        fstat(src,&buff);
        int stm = buff.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

        if (S_ISDIR(buff.st_mode)) {
            adapt(dest_path);
            adapt(src_path);
            DIR* dsrc = opendir(src_path);
            if (dsrc < 0) {
                perror("opendir src ");
            }

            if (mkdir(dest_path,stm) < 0) {
                fprintf(stderr,"on %s; ",dest_path);
                perror("dest mkdir ");
                result = -3;
            }
            else {
                struct dirent* content = readdir(dsrc);
                int i = 0;
                while (content != NULL) {
                    if (i++ > 1) {

                        char tmpd[200];
                        strcpy(tmpd,dest_path);
                        char tmps[200];
                        strcpy(tmps,src_path);

                        strcat(tmps,content->d_name);
                        strcat(tmpd,content->d_name);

                        DIR* adir = opendir(tmps);
                        if (adir != NULL) {
                            strcat(tmps,"/");
                            strcat(tmpd,"/");
                        }
                        closedir(adir);
                        // RECURSIVE CALL
                        CommandeCP(tmps,tmpd);
                    }
                        content = readdir(dsrc);
                }
            }
            close(src);
            closedir(dsrc);
        }
        else {

            int dest = open(dest_path,O_WRONLY | O_CREAT);
            int nbbytes;
            while ((nbbytes = read(src,buffer,50)) != 0) {
                if (nbbytes < 0) {
                    if (!(errno == EINTR || errno == EAGAIN)) {
                        perror("read src ");
                        result = -5;
                        break;
                    }
                }
                else if (write(dest,buffer,50) < 0) {
                   if (!(errno == EINTR || errno == EAGAIN)) {
                        perror("write dest ");
                        result = -4;
                        break;
                    }
                }
            }

            fchmod(dest,stm);
            close(dest);
        }
    }
    else {
       result = -2;
       fprintf(stderr,"%s : ",src_path);
       perror("");
    }

    free(buffer);
    close(src);

    return result;
}

void CommandeHistory(Minishell* monShell)
{
    int i;
    for (i=0; i<monShell->compteurHistorique; i++)
        printf("\t%d : %s \n", i+1, monShell->historique[i]);
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

	//Découper la chaîne selon les espaces
	token = strtok (chaine," ");

	while (token != NULL)
	{
		tabMots[i]= token;
		token = strtok (NULL, " ,");
		i++;
	}

	return i;
}

void CommandeCD(char** args, char* repertoire) {

    if (args[1] == NULL) {
        strcpy(repertoire,"/");
    }
    else {
        if (strlen(args[1]) > 1 && dernier(args[1])== '/') {
            *setdernier(args[1]) = 1;
        }
        if (strlen(args[1]) > 1 && premier(args[1]) == '/') {
            *setpremier(args[1]++) = 1;
        }

        if (!strcmp(args[1],"/")) {
            strcpy(repertoire,"/");
        }
        else if (!strcmp(args[1],".")){

        }
        else if (!strcmp(args[1],"..")) {
            popChemin(repertoire);
        }
        else  {
            char* temp = calloc(LONGUEUR,sizeof(char));
            strcpy(temp,repertoire);
            if ( !(!strcmp(repertoire,"/") || dernier(repertoire) == '/') ) {
                strcat(temp,"/");
            }
            strcat(temp,args[1]);

            if (estRepertoire(temp)) {
                strcpy(repertoire,temp);
            }
            else {
                printf("minishell : No such file or directory.\n");
            }

            free(temp);
        }
    }
}

void CommandeTouch(char** args, char* repertoire) {
    FILE* monFichier;
    if ((monFichier = fopen(args[1],"a")) != NULL) {
        fclose(monFichier);
    }
}

void InterpreterCommande(Minishell* monShell, int argc, char** tabMots) {

	pid_t pid;



    if (!strcmp(tabMots[0], "cd"))
    {
        CommandeCD(tabMots, monShell->repertoire);
    }
    else if (!strcmp(tabMots[0], "history"))
    {
        CommandeHistory(monShell);
    }
	else if (!strcmp(tabMots[0], "cat"))
	{
		CommandeCat(tabMots[1]);
	}
	else if (!strcmp(tabMots[0], "touch")) {
        CommandeTouch(tabMots,monShell->repertoire);
	}
	else if (!strcmp(tabMots[0], "cp")) {
        if (argc > 2)
            CommandeCP(tabMots[1],tabMots[2]);
	}
	else
	{
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
                execv(tabMots[0], tabMots);
               // printf("Never executed!\n");
				exit(0);
			break;
			//  Si on est dans le père
			default:
				waitpid(pid, &status, 0);
				//printf("Son ended with status %d\n",status);
			break;
		}
	}
}


int main(void)
{
	char *chaine;
	char nomUtilisateur [LONGUEUR];
	char nomHote        [LONGUEUR];
	int nombreMots = 20;
	char ** tabMots;
	char ** arguments;
	int boolExit = 0;

    Minishell monShell = {.compteurHistorique = 0};

    chaine              = (char*) calloc(LONGUEUR,sizeof(char));
	tabMots             = (char**)malloc(nombreMots*sizeof(char*));
	monShell.historique = (char**)malloc(TAILLE_HISTORIQUE*sizeof(char*));

    getcwd(monShell.repertoire,sizeof(monShell.repertoire));
    gethostname(nomHote,sizeof(nomHote));
    getlogin_r(nomUtilisateur,sizeof(nomUtilisateur));

	while (!feof(stdin)&&(!boolExit))
	{
		printf("> [%s@%s:%s]", nomUtilisateur, nomHote, monShell.repertoire);
		if (SaisirChaine(chaine, LONGUEUR) && (!feof(stdin)))
		{
            InsererHistorique(chaine, &monShell);
            int argc = DecouperChaine(chaine, tabMots);
			if (argc)
			{
                arguments = calloc(argc,sizeof(char*));
                for (int i=0;i<argc;i++) {
                    arguments[i] = tabMots[i];
                }
                if (!strcmp(tabMots[0], "exit"))
                    boolExit = 1;
                else {
                    InterpreterCommande(&monShell, argc, arguments);
                }
                free(arguments);
            }
		}
	}

    free(chaine);
    free(tabMots);
    for (int
    i=0;i<monShell.compteurHistorique;i++)
        free(monShell.historique[i]);
    free(monShell.historique);

	return (0);
}



void afficherArgs(char** tabMots) {
        int i = 0;
        printf("Your arguments :\n");
        while (tabMots[i] != NULL) {
            printf("\n\t%d : %s(%lu)",i,tabMots[i],strlen(tabMots[i]));
            i++;
        }
        printf("\n");
}

int estRepertoire(char* path) {
    DIR* dir;
    if ((dir = opendir(path)) != NULL) {
        closedir(dir);
        return true;
    }
    return false;
}

char dernier(char* str) {
    return str[strlen(str)-1];
}

char* setdernier(char* str) {
    return &str[strlen(str)-1];
}

char premier(char* str) {
    return str[0];
}

char* setpremier(char* str) {
    return &str[0];
}

void popChemin(char* path) {
    if (strcmp(path,"/")) {
        char c = dernier(path);
        while (strlen(path) && strcmp(path,"/") && c != '/') {
            c = dernier(path);
            *setdernier(path) = 0;
        }
    }
}
