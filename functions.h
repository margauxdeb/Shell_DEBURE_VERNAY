#ifndef FUNCTIONS_H
#define FUNCTIONS_H

void afficherArgs(char**);
void showArgs(char**,int);
int estRepertoire(char*);
/*cherche le chemin absolu à partir d'un chemin relatif
*/
void chercherChemin(char*,Minishell*);

/* Un chemin est comme une pile ; on ne peut enlever que l'element du dessus
Ici on enlève un element jusqu'au '/' precedent
*/
void popChemin(char*);

/* Fonctions de manipulation de chaine */
char dernier(char*);
char premier(char*);
char* setdernier(char*);
char* setpremier(char*);

void adapt(char*);
int contains(char,char*,int);
int containsOneOf(char*,int,char*,int);
int fileExists(const char*);
int isAPID(char*);

/* Regarde dans /proc/X/stat pour tenter de retrouver le PID (4e champ) */
pid_t getppidlive(char*);

/* cherche la chaine 'str' dans 'tabMots' et renvoie l'indice de la premiere occurence
*/
int detect(char* str, char** tabMots, int argc);

/* cherche l'emplacement d'une commande en regardant dans PATH */
void chercherCommande(char* cmd);

void removeElement(int ind, char** tab, int argc);

/* nettoie le tableau des elements redirectionnels (<,> + leurs arguments) */
int removeRedirections(char** args, int argc);




/* duplique le processus appelant et retourne
 le PID du processus fils ainsi créé */
pid_t creerProcessus();

void ajouterJob(Minishell* monShell);

Job* getJob(Minishell* monShell, int id);

void supprimerJob(Minishell* monShell);

/* insere 'chaine' dans l'historique */
void insererHistorique(char* chaine, Minishell* monShell);

/* effectue la saisie d'une ligne */
int saisirChaine(char *chaine, int longueur);

/* fait appel a strtok() pour effectuer un split de 'chaine' */
int decouperChaine(char* chaine, char** tabMots, char* delimiteurs);

#endif
