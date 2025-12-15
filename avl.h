/*
 * avl.h - En-tête pour l'arbre AVL
 * Projet C-Wildwater - Gestion des usines de traitement d'eau
 * 
 * Cet arbre AVL permet de stocker les usines et leurs données
 * avec une complexité O(log n) pour la recherche et l'insertion.
 */

#ifndef AVL_H
#define AVL_H

/* Structure pour une usine de traitement */
typedef struct Usine {
    char identifiant[50];      /* Identifiant unique de l'usine */
    double capacite_max;       /* Capacite maximale de traitement (k.m3) */
    double volume_capte;       /* Volume total capte par les sources (k.m3) */
    double volume_traite;      /* Volume reellement traite (k.m3) */
} Usine;

/* Noeud de l'arbre AVL */
typedef struct NoeudAVL {
    Usine usine;
    int hauteur;
    struct NoeudAVL *gauche;
    struct NoeudAVL *droite;
} NoeudAVL;

/* Fonctions de base pour l'AVL */
NoeudAVL* creerNoeud(Usine usine);
int hauteur(NoeudAVL *noeud);
int max(int a, int b);
int equilibre(NoeudAVL *noeud);

/* Rotations pour equilibrer l'AVL */
NoeudAVL* rotationDroite(NoeudAVL *y);
NoeudAVL* rotationGauche(NoeudAVL *x);

/* Operations principales */
NoeudAVL* insererAVL(NoeudAVL *racine, Usine usine);
NoeudAVL* rechercherAVL(NoeudAVL *racine, char *identifiant);

/* Parcours et liberation */
void parcoursInverseAVL(NoeudAVL *racine, FILE *fichier, int mode);
void libererAVL(NoeudAVL *racine);
int compterNoeuds(NoeudAVL *racine);

#endif
