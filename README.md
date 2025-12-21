



https://github.com/user-attachments/assets/351a8f59-a6b2-44a9-a889-cce7d6353229

# Projet C-Wildwater

## Description

Ce projet permet d'analyser les données d'un système de distribution d'eau.
Il génère des histogrammes des usines de traitement et calcule les fuites
sur le réseau de distribution.

## Structure du projet

```
c-wildwater/
├── c-wildwater.sh      # Script Shell principal (point d'entrée)
├── README.md           # Ce fichier
├── codeC/              # Code source en C
│   ├── main.c          # Programme principal
│   ├── avl.c           # Implémentation de l'arbre AVL
│   ├── avl.h           # En-tête de l'AVL
│   ├── arbre_distrib.c # Arbre de distribution (pour calcul fuites)
│   ├── arbre_distrib.h # En-tête de l'arbre de distribution
│   └── Makefile        # Fichier de compilation
├── graphs/             # Graphiques générés (PNG)
└── tests/              # Fichiers de données générés
```

## Compilation

La compilation se fait automatiquement lors de la première exécution du script.
Pour compiler manuellement :

```bash
cd codeC
make
```

Pour nettoyer les fichiers compilés :

```bash
cd codeC
make clean
```

## Utilisation

### Syntaxe générale

```bash
./c-wildwater.sh <fichier_donnees> <commande> <option>
```

### Génération d'histogrammes

```bash
# Capacité maximale des usines
./c-wildwater.sh donnees.dat histo max
 OU ./c-wildwater.sh donneesv3.dat histo max


# Volume capté par les sources
./c-wildwater.sh donnees.dat histo src
 OU ./c-wildwater.sh donneesv3.dat histo src


# Volume réellement traité
./c-wildwater.sh donnees.dat histo real
 OU ./c-wildwater.sh donneesv3.dat histo real


# Histogramme combiné (all)
./c-wildwater.sh donnees.dat histo all
 OU ./c-wildwater.sh donneesv3.dat histo all
```

### Calcul des fuites d'une usine

```bash
./c-wildwater.sh donnees.dat leaks "Plant #JA200000I"
```

**Note:** L'identifiant de l'usine doit être exact et entre guillemets.

## Fichiers de sortie

### Histogrammes

- `tests/histo_<mode>.dat` : Données au format CSV
- `graphs/histo_<mode>_high.png` : Graphique des 10 plus grandes usines
- `graphs/histo_<mode>_low.png` : Graphique des 50 plus petites usines

### Fuites

- `tests/leaks.dat` : Historique des fuites calculées

## Format des données d'entrée

Le fichier d'entrée doit être au format CSV avec le séparateur `;`.
Chaque ligne représente un tronçon du réseau de distribution :

- Colonne 1 : Identifiant de l'usine (ou `-`)
- Colonne 2 : Identifiant amont du tronçon
- Colonne 3 : Identifiant aval du tronçon
- Colonne 4 : Volume ou capacité (en milliers de m³)
- Colonne 5 : Pourcentage de fuites

## Structures de données utilisées

### AVL (Arbre AVL équilibré)

Utilisé pour stocker les usines avec leurs données de volume.
Permet une recherche et insertion en O(log n).

### Arbre de distribution

Arbre n-aire (nombre d'enfants variable) représentant le réseau
de distribution depuis une usine jusqu'aux usagers.
Utilisé pour le calcul des fuites.

## Auteurs
AAMIR Talal
ARSLAN Emir
WADOUD Abdel
Projet préING2 - CY Tech 

## Dépendances

- GCC (compilateur C)
- Make
- Gnuplot ( génération des graphiques)
- Bash





