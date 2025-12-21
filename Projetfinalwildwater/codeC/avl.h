/*
 //avl.h - En-tete pour l'arbre AVL 
 * Cet arbre AVL permet de stocker les usines et leurs donnees
 * 
 * Convention d'equilibre: eq = hauteur(droite) - hauteur(gauche) 

 */

#ifndef AVL_H
#define AVL_H

#include <stdio.h>

/* Structure pour une usine de traitement */
typedef struct Usine {
    char identifiant[50];      
    double capacite_max;       
    double volume_capte;       
    double volume_traite;      
} Usine;

// Noeud de l'arbre AVL  
typedef struct NoeudAVL {
    Usine usine;
    int eq;                    
    struct NoeudAVL *fg;       
    struct NoeudAVL *fd;      
} NoeudAVL;

// Fonctions 
int max(int a, int b);
int min(int a, int b);
int max3(int a, int b, int c);
int min3(int a, int b, int c);

/* Creation d'un noeud */
NoeudAVL* creerNoeud(Usine usine);

/* Rotations pour equilibrer l'AVL */
NoeudAVL* rotationGauche(NoeudAVL *a);
NoeudAVL* rotationDroite(NoeudAVL *a);
NoeudAVL* doubleRotationGauche(NoeudAVL *a);
NoeudAVL* doubleRotationDroite(NoeudAVL *a);

// Equilibrage 
NoeudAVL* equilibrerAVL(NoeudAVL *a);

//Operations principales 
NoeudAVL* insererAVL(NoeudAVL *a, Usine usine, int *h);
NoeudAVL* rechercherAVL(NoeudAVL *racine, char *identifiant);

/* Parcour et liberation */
void parcoursInverseAVL(NoeudAVL *racine, FILE *fichier, int mode);
void libererAVL(NoeudAVL *racine);
int compterNoeuds(NoeudAVL *racine);

#endif
