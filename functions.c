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
#include "functions.h"


/* Ajoute un '/' à la fin de la chaine */
void adapt(char* s) {
    if (s[strlen(s)-1] != '/') {
        printf("%s",s);
        strcat(s,"/");
        printf(" changed to %s\n",s);
    }
}

int contains(char c, char* tab, int len) {
    int nb = 0;
    for (int i=0;i<len;i++) {
        nb += (tab[i] == c);
    }
    return nb;
}

int containsOneOf(char* c, int nb, char* tab, int len) {
    for (int i=0;i<nb;i++) {
        if (contains(c[i],tab,len))
            return true;
    }
    return false;
}

int fileExists(const char* path) {
    FILE* f = fopen(path,"r");
    if (f != NULL) {
        fclose(f);
        return true;
    }
    else {
        return false;
    }
}

int isAPID(char* str) {
    for (int i=0;i<strlen(str);i++) {
        if (str[i] < 48 || str[i] > 57)
            return false;
    }
    return true;
}

void afficherArgs(char** tabMots) {
        int i = 0;
        printf("Your arguments :");
        if (tabMots == NULL) {
            printf("it's void !\n");
        }
        else
        while (tabMots[i] != NULL) {
            printf("\n\t%d : %s(%d)",i,tabMots[i],strlen(tabMots[i]));
            i++;
        }
        printf("\n");
}

void showArgs(char** tab, int len) {
    printf("Your arguments : ");
    for (int i=0;i<len;i++) {
        printf("\n\t%d : %s",i,tab[i]);
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


void chercherChemin(char* thispath, Minishell* monShell) {
    if (!strcmp(thispath,"/"))
        return;

    char result[LONGUEUR];
    char path[LONGUEUR];
    strcpy(path,thispath);
    int beginslash = thispath[0] == '/';
    char** elems = malloc(64*sizeof(char*));
    //for (int i=0;i<64;i++)
     //   elems[i] = malloc(LONGUEUR*sizeof(char));
    char delem[] = "/";
    int nb = decouperChaine(path,elems,delem);
    //showArgs(elems,nb);
    int prevnb = 0;
    for (int i=0;i<nb;i++) {
        prevnb += !strcmp(elems[i],"..");
    }
    if (!strcmp(path,".")) {
        strcpy(result,monShell->repertoire);
    }
    else if (!strcmp(path,"..")) {
        char temp[LONGUEUR];
        strcpy(temp,monShell->repertoire);
        while (prevnb--)
            popChemin(temp);
        strcpy(result,temp);
    }
    else if (beginslash) {
        return;
    }
    else {
        char temp[LONGUEUR];
        strcpy(temp,monShell->repertoire);
        if (strcmp(temp,"/"))
            strcat(temp,"/");
        strcat(temp,thispath);
        if (fileExists(temp) || estRepertoire(temp)) {
            strcpy(thispath,temp);
        }
        return;
    }
    for (int i=0;i<nb;i++) {
         if (elems[i][0] != '.') {
            strcat(result,"/");
            strcat(result,elems[i]);
        }
    }

    //for (int i=0;i<64;i++)
    //    free(elems[i]);
    free(elems);
    strcpy(thispath,result);
    //printf("absolute_path=%s\n", path);
}



pid_t getppidlive(char* pid) {
    char path[LONGUEUR];
    sprintf(path,"/proc/%s/stat",pid);
    FILE* there = fopen(path,"r");
    if (there != NULL) {
        fgets(path,1024,there);
        char** tabs = calloc(64,sizeof(char*));
        decouperChaine(path,tabs," ");
        pid_t ppid = atoi(tabs[3]);
        free(tabs);
        fclose(there);
        return ppid;
    }
    return -1;
}

int detect(char* str, char** tabMots, int argc) {
    for (int i=0;i<argc;i++) {
        if (!strcmp(str,tabMots[i]))
            return i;
    }
    return -1;
}



void chercherCommande(char* cmd) {
    char* path = getenv("PATH");
    char** paths = calloc(32, sizeof(char*));
    int nbpaths = decouperChaine(path,paths,":");
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

    return argc;
}




/* La fonction create_process duplique le processus appelant et retourne
 le PID du processus fils ainsi créé */
pid_t creerProcessus() {
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

void ajouterJob(Minishell* monShell) {
    //TODO
    monShell->nbjobs++;
}

Job* getJob(Minishell* monShell, int id) {
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

void supprimerJob(Minishell* monShell) {
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

void insererHistorique(char* chaine, Minishell* monShell) {
    if (chaine != NULL && strlen(chaine)) {
        FILE* history = fopen(monShell->historyPath,"a");
        if (history != NULL) {
            fprintf(history,"%s\n",chaine);
            fclose(history);
        }
    }
}

int saisirChaine(char *chaine, int longueur) {
    int verif = 0;
    char *positionEntree = NULL;

    if (fgets(chaine, longueur, stdin) != NULL)
    {
        verif = 1;
        positionEntree = strchr(chaine, '\n');
        if (positionEntree != NULL)
            *positionEntree = '\0';
    }
    return (verif) && strcmp(chaine,"");

}

int decouperChaine(char* chaine, char** tabMots, char* delimiteurs) {
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

