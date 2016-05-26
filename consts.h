#ifndef CONSTS_H
#define CONSTS_H

#define true    1
#define false   0

#define PIPE_READ   0
#define PIPE_WRITE  1

#define LONGUEUR 2048
#define TAILLE_HISTORIQUE 100

typedef struct job {
    char* cmdline;
    int status;
    pid_t pid;
    int id;
} Job;

typedef struct minishell {
    char   historyPath[LONGUEUR];
    char    repertoire[LONGUEUR];
    Job     jobs[64];
    int     jobCounter;
    int     nbjobs;
    pid_t pid;
} Minishell;

#endif
