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

#include "commands.h"
#include "functions.h"

void commandeCD(char* path, Minishell* monShell) {

    if (path == NULL) {
        strcpy(monShell->repertoire,"/");
        chdir(monShell->repertoire);
    }
    else {
        char temp[LONGUEUR];
        strcpy(temp,path);
        chercherChemin(temp,monShell);
        if (estRepertoire(temp)) {
            strcpy(monShell->repertoire,temp);
            chdir(monShell->repertoire);
        }
        else {

        }
    }
}

void commandeTouch(int argc, char** args, char* repertoire) {
    FILE* monFichier;
    for (int i=0;i<argc;i++) {
        if ( args[i][0] != '-' && (monFichier = fopen(args[i],"a")) != NULL) {
            fclose(monFichier);
        }
    }
}

void commandeCat(int argc, char** args) {
	char caractere;
    char tampon1, tampon2;
	FILE* monFichier;
	int count = 0;
	if (argc) {
        for (int i=0;i<argc;i++) {
            if (!strcmp(args[i],"-n")) {
                count = 1;
                break;
            }
        }
        for (int i=0;i<argc;i++) {
            if ( args[i][0] != '-' && (monFichier = fopen(args[i],"r")) != NULL)
            {
                if (count)
                {
                    printf("%d ", count);
                    count++;
                }

                tampon1 = fgetc(monFichier);
                tampon2 = fgetc(monFichier);

                while (tampon1 != EOF)
                {
                    caractere = fgetc(monFichier);

                    printf("%c",tampon1);

                    if (tampon1 == '\n' && count && (tampon2 != EOF))
                    {
                        printf("%d ", count);
                        count++;
                    }

                    tampon1 = tampon2;
                    tampon2 = caractere;
                }
                printf("\n");
                fclose(monFichier);
            }
        }
	}
	else { // on lit l'entrée standard
        while (caractere != EOF) {
            printf("%c",caractere);
            caractere = fgetc(stdin);
        }
        printf("\n");
	}
}

void commandeWait(char* arg) {
    pid_t pid = atoi(arg);
    waitpid(pid,0,0);
}

void commandePS(Minishell* monShell) {
    printf("WIP\n");
    int i = 0;
    DIR* proc = opendir("/proc");
    struct dirent* content = readdir(proc);
    char cmdline[LONGUEUR];
    printf("PID\tCMD\n");
    while (content != NULL && i < 5) {
        if (isAPID(content->d_name) && strcmp(content->d_name,".") && strcmp(content->d_name,"..")) {
            char path[LONGUEUR];
            strcpy(path,"/proc/");
            strcat(path,content->d_name);
            strcat(path,"/cmdline");
           // printf("path:%s\n",path);
            FILE* myf = fopen(path,"r");
            fgets(cmdline,LONGUEUR,myf);
            fclose(myf);
            if (getppidlive(content->d_name) == monShell->pid || getpid() == monShell->pid)
                printf("%s\t%s\n",content->d_name,cmdline);
           // i++;
        }
        content = readdir(proc);
    }
    printf("\n");
    closedir(proc);
}

void commandeHistory(Minishell* monShell, int argc, char** args) {
    char buffer[LONGUEUR];
    int ind = 0;
    int execute = false;
        for (int j=0;j<argc;j++) {
            if (args[j][0] == '!') {
                ind = atoi(&args[j][1]);
                execute = true;
                break;
            }
        }
        if (!execute && argc > 1)
            ind = atoi(args[1]);
	FILE* monFichier;
	int i = 0;
	int all;
	if ((monFichier = fopen(monShell->historyPath,"r")) != NULL)
	{
        if (!execute) {
            if (ind) {
                while ((fgets(buffer,LONGUEUR,monFichier)) != NULL) i++;
                all = i;
                fseek(monFichier,0,SEEK_SET);
                i = 0;
            }
            while ((fgets(buffer,LONGUEUR,monFichier)) != NULL)
            {
                if (i >= all-ind) {
                    printf("%d\t%s",i,buffer);
                }
                i++;
            }
        }
        else {
            while ((fgets(buffer,LONGUEUR,monFichier)) != NULL && i <= ind)
            {
                if (i == ind) {
                    *setdernier(buffer) = 0;
                    interpreterLigne(buffer,monShell);
                    break;
                }
                i++;
            }
        }
		fclose(monFichier);
	}
    //printf("ind=%d\n",ind);
}

int commandeCP(const char* srcPath, const char* destPath) {

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
                        commandeCP(tmps,tmpd);
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

int identifySignal(char* str) {
    int i = atoi(str);
    if (i > 0)
        return i;
    else if (!strcmp("SIGKILL",str)) {
        return SIGKILL;
    }
    else if (!strcmp("SIGCONT",str)) {
        return SIGCONT;
    }
    else if (!strcmp("SIGSTOP",str)) {
        return SIGSTOP;
    }
    else {
        return -1;
    }
}

void commandeKill(char* str_sigh, char* str_pid) {
    if (str_pid == NULL) {
        pid_t pid =atoi(str_sigh);
        kill(pid,SIGTERM);
    }
    else {
        for (int i=0;i<strlen(str_sigh)-1;i++) {
            str_sigh[i] = str_sigh[i+1];
        }
        str_sigh[strlen(str_sigh)-1] = '\0';
        pid_t pid =atoi(str_pid);
        int sigh;
        if ((sigh = identifySignal(str_sigh)) > 0) {
            printf("SIG%d to PID%lu\n",sigh,pid);
            kill(pid,sigh);
        }
        else {
            printf("Couldn't tell what %s is!\n",str_sigh);
        }
    }
}

void commandeFG(Minishell* monShell, char* str) {
    printf("WIP\n");
    int id = atoi(str);
    Job* job = getJob(monShell,id);
    if (job != NULL) {
        kill(job->pid,SIGCONT);
    }
}

void commandeBG(Minishell* monShell, char* str) {
    printf("WIP\n");
    int id = atoi(str);
    Job* job = getJob(monShell,id);
    if (job != NULL) {
        kill(job->pid,SIGSTOP);
    }
}

char* getStatus(int status) {
    switch (status) {
    case 1:
        return "Running";
        break;
    case 2:
        return "Stopped";
        break;
    default:
        return "Unknown";
        break;
    }
}

void commandeJobs(Minishell* monShell) {
    printf("WIP\n");
    int i=0;
     for (int j=0;i<monShell->nbjobs && j<64;j++) {
        if (monShell->jobs[j].status) {
            printf("[%d]\t%lu\t%s\t%s\n",monShell->jobs[j].id,monShell->jobs[j].pid,getStatus(monShell->jobs[j].status),monShell->jobs[j].cmdline);
            i++;
        }
    }
}
