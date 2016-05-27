# Rapport de projet Minishell
## Sujet du projet

Description du sujet : qu’est ce qu’un shell en quelque lignes, ce que l’on est censé reproduire.
> “*Le rapport quant à lui devra être soumis à vos profs de TD, TP respectifs au plus grand tard le vendredi 27 mai à 18 h. Pour rappel, c’est un rapport technique c’est à dire un rapport qui explique l’analyse, les choix en terme d’algorithmes et structures de données que vous avez effectués pour résoudre le problème. Il s’agira également de décrire explicitement l’architecture, les différentes fonctions et procédures du shell de telle sorte que votre petit frère au lycée puisse comprendre votre code sans difficulté ;). Pour les questions que vous n’avez pas pu faire, l’analyse de la solution et l’approche que vous auriez envisagées pourront être incluses dans le rapport. Il faut également noter que la note du rapport dépendra en grande partie de la clarté et de la précision de la rédaction.  Pour finir, le mail d’envoi du rapport devra contenir l’URL de votre repo github public.
Bon courage et bon weekend.*”

## Fonctionnement de base

La gestion de la saisie, les découpages successifs en nommant les fonctions correspondantes, ...

Pour manipuler facilement les données du Shell, nous avons pris le parti d'utiliser la structure suivante :

	typedef struct minishell {
	    char   historyPath[LONGUEUR];
	    char    repertoire[LONGUEUR];
	    Job     jobs[64];
	    int     jobCounter;
	    int     nbjobs;
	    pid_t pid;
	} Minishell;

* `historyPath` contient simplement le chemin du fichier où l'on stocke l'historique du shell ; il s'agit du chemin absolu, car on ne veut évidemment pas créer un nouveau fichier partout où l'on se déplace via la commande `cd`.
* `repertoire` contient le nom du répertoire de travail courant ; on s'en sert pour l'affichage classique du shell, ainsi que pour calculer les chemins
* `jobs` contient la liste des jobs en cours
* `jobCounter` numérote les identifiants de jobs
* `nbjobs` est le nombre de jobs en cours, taille de `jobs`
* `pid` est le PID du shell ; on s'en sert dans la commande `ps` pour déterminer quels processus ont le shell pour parent (PPID).

## Fonctionnalités

La gestion des commandes par rapport au shell (internes -> dans le programme père, internes & externes -> dans un programme fils, …)
### Gestion des commandes internes
* cd
* ...

### Gestion des commandes externes


## I/O (E/S) management

### Pipes

La fonction `interpreterLigne(char* cmdline, Minishell* monShell)` prend la chaîne saisie par l'utilisateur et regarde si elle contient le caractère `|` ; si tel est le cas, alors on utilise la fonction `decouperChaine(char* str, char** strs, char* delim)` pour récupérer les différentes commandes entre chaque pipe. On fait en sorte que le nombre de pipes soit correct (s'il est supérieur ou égal au nombre de commandes, alors on le réduit au nombre de commandes - 1, soit le cas attendu), puis on instancie successivement les pipes nécessaires à l'exécution de la ligne de commande, à mesure que l'on exécute chaque commande l'une après l'autre.

Pour chaque commande, on  appelle la fonction `executerCommande(Minishell*, char cmdline, int in, int out)` en fournissant la commande à exécuter, et les changement d'entrée et sortie standard dans la table des descripteurs de fichier. Cette fonction crée un processus fils, et y utilise la fonction `dup2(int,int)` pour remplacer `stdin` et `stdout` par les descripteurs passés en paramètre. La fonction `executerCommandeDansFils(Minishell*, int argc, char* args)` est appelée dans le processus fils ; s'y trouve certaines commandes internes au Shell et l'utilisation de `execv()`.

e.g. on veut exécuter la commande `ls | grep .c` :

`ls` a pour retour :

	main.c	functions.c functions.h makefile README.md	bin	src	logs

On remarque qu'il y a un pipe, à exécuter, donc on crée un nouveau pipe avec `pipe(monPipe)` et on exécute `ls` avec `in=stdin, out=monPipe[write_end]` ; une fois terminée, la fonction va fermer `monPipe[write_end]` ;

On crée un nouveau processus fils et on y exécute la commande `grep .c` avec `in=monPipe[read_end], out=stdout` ; le pipe contient alors le retour de la commande `ls`, ce qui nous donne :

	ls | grep .c :
		main.c
		functions.c
		functions.h
		src

On termine l'exécution par la fermeture de `monPipe[read_end]`. En toute logique, les deux extrémités du pipe sont maintenant fermées.

Il est à noter que les opérations de lecture (dans un pipe en l'occurence) sont **bloquantes**.

Egalement, on ne peut pas se permettre d'écrire une seule fois et de lire une seule fois dans un pipe ; un pipe ne peut contenir qu'une quantité limitée de données. C'est pour cette raison, qu'on ne peut pas se contenter d'exécuter les commandes les unes après les autres au sein du même processus. On crée donc un processus fils par commande à exécuter ; chaque processus terminera son opération quand il le jugera ; un processus devant écrire dans un pipe s'arrêtera une fois qu'il aura fini d'écrire, et un processus lisant dans un pipe se terminera après avoir lu jusqu'à ce que le pipe soit vide, et qu'il ne soit plus possible d'y écrire (i.e `close(write_end)` car on aura fermé l'entrée en écriture du pipe. 

### Redirections

Une fois les commandes déterminées après séparation par les pipes par la fonction `interpreterLigne()`, la fonction `executerCommande()` sépare les termes de la commande par le caractère  ` ` (le caractère espace, en termes de mots donc) (ce n'est guère optimal, car cela omet les formations du type ` echo "hello there!").

On ne traite ici que les cas `<` et `>`. Si l'on trouve l'un de ces deux symboles parmi les mots de la ligne de commande, on lit ce qui les suit :

e.g. `grep ^error: < log > logerr`

On trouve le symbole `<` à l'indice 3 ; donc se trouve à l'indice 4 (`log`) la destination de la redirection de l'entrée standard. On tente d'ouvrir le fichier correspondant (si on a quelque chose comme `grep ^error: < > logerr`, `<` sera simplement ignoré), et si on y parvient, on ferme l'entrée standard passée en paramètre (on se trouve dans la fonction `executerCommande(char*,Minishell*, int in, int out)`) et on la remplace par le descripteur du fichier que l'on vient d'ouvrir.

On effectue la même opération pour le symbole `>`.

Une fois la tâche effectué, on ferme les entrée et sortie de remplacement, qu'il s'agissent de fichiers ou de pipes. 

Tout ceci s'effectue avant de créer un processus fils.

## Historique

À chaque nouvelle ligne non vide entrée par l'utilisateur, on stocke celle-ci dans l'historique. 
L'historique est enregistré dans un fichier (minishell.history), on ajoute donc la nouvelle ligne à la fin du fichier.

La commande `history` permet d'afficher le contenu de l'historique du shell. Elle lit le fichier ligne par ligne en numérotant chaque nouvelle ligne.

La commande `history n` retourne les n dernières lignes de ce fichier.

La commande `!n` ou `history !n` exécute la commande se trouvant à la n-ième ligne de l'historique, à l'instar du ligne de commande de l'utilisateur.

e.g. l'historique contient :

	0 pwd
	1 cd Documents
	2 ls
	3 rm -rf *.c
	4 cat log | grep ^error: | wc -l
	5 echo $?

* `historique 2` nous donne :

		4 cat log | grep ^error: | wc -l
		5 echo $?


* `historique !2` ou `!2` exécute la commande `ls`.

## Jobs

Cette partie n'a pas été réalisé dans sa totalité ; on a fait le choix d'utiliser, à l'instar d'un objet :

	typedef struct job {
	    char* cmdline;
	    int status;
	    pid_t pid;
	    int id;
	} Job;