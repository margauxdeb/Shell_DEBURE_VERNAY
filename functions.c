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
    for (int i=0;i<len;i++) {
        if (tab[i] == c)
            return true;
    }
    return false;
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
        printf("Your arguments :\n");
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
    printf("Your arguments : \n");
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

