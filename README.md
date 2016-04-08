# Minishell
Margaux Debure, Jocelyn Vernay

## Etape 1 : Préliminaires

Le shell sera une simple boucle d'interaction qui affiche une invite de commande de la forme `user@machine:repertoire/courant>`, 
lit une entrée clavier et l'exécute. Le shell quitte quand l'utilisateur tape `ctrl+D` à la place d'une commande.

### Exercice 1

On suppose pour commencer que la commande est simplement un nom de programme à exécuter dans un processus séparé. Le shell doit attendre
que le programme se termine avant de rendre la main à l'utilisateur.

### Exercice 2

On suppose maintenant que la ligne de commande peut contenir un nombre arbitraire d'arguments, séparés par un ou plusieurs espaces.

### Exercice 3

Réécrire la commande `cd`.

## Etape 2 : Commandes internes supplémentaires

Réécrivez les commandes internes suivantes : `history`, `exit`, `touch`, `quit`, `cat`. Incorporer la fonction `copy` que vous avez
écrite lors du TP1.

## Etape 3 : Gestion du path

Dans cette partie, notre shell doit pouvoir trouver le chemin d'une commande externe avant de l'exécuter. Pour ce faire, les fonctions
`execvp`, `execlp` sont proscrites, car elles cherchent d'elles-mêmes le chemin.

## Etape 4 : Pipes et redirections

Vous devez ajouter le mécanisme de redirection `> (1>, 2>)` et les pipes (`|`) dans le shell.

## Etape 5 : Job Control Shell

Le but de cette partie est d'incorporer des mécanismes de Job control à notre shell. Le job control est la capacité qu'a un shell d'arrêter,
de suspendre, de mettre en tâche de fond(background), de mettre en avant(foreground) ou même de continuer l'exécution d'une
commande. Un job est une commande ou une série de commandes séparées par des pipes `|` (exemple : `cat toto.txt | grep tata`). Un job est associé
à une entrée de la ligne de commande ; i.e que même si une entrée contient une série de commandes pipelinées, elle constitue un unique
job. Ainsi, on souhaiterait pouvoir suspendre et arrêter un job et avoir les équivalents des commandes bash suivantes : 
- `wait`
- `fg`
- `bg`
- `jobs`
- `kill`
- `ps`

## Bonus

Implémenter la commande `find`.

## Remarques

Le projet se fera par groupe de 2 étudiants. L'évaluation du projet se fera en deux étapes : 
- à la troisième séance, on évaluera l'implémentation de la partie préliminaire. Il faudra donc avoir un code source dénommé
`préliminaire.c` pour cette partie.
- à la séance de soutenance, on évaluera le projet dans son entièreté. Le code final principal sera dénommé `shell_final.c`.

La note finale dépendra de ces deux évaluations sachant que la phase préliminaire compte pour 1/3 de la note finale. Il faudra fournir
en fin de projet un rapport détaillant votre analyse ainsi que les choix (structures de données, algorithmes ...) utilisés. Le rapport
devra être nommé Nom1_Nom2.pdf. En outre, il faudra utiliser obligatoirement le gestionnaire de version **Git** ; chaque groupe devra
envoyer le lien du code source de son projet et devra le rendre public.
