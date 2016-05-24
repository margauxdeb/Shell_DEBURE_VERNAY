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
            printf("\n\t%d : %s",i,tabMots[i]);
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
    int nb = DecouperChaine(path,elems,delem);
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

