/*
 * arbre_distrib.h - En-tête pour l'arbre de distribution
 * Projet C-Wildwater
 * 
 * Cet arbre represente le reseau de distribution d'eau
 * depuis une usine jusqu'aux usagers.
 * Chaque noeud peut avoir plusieurs enfants (liste chainee).
 */

#ifndef ARBRE_DISTRIB_H
#define ARBRE_DISTRIB_H

/* Noeud de l'arbre de distribution */
typedef struct NoeudDistrib {
    char identifiant[50];           /* Identifiant du noeud */
    double pourcentage_fuite;       /* Pourcentage de fuite du troncon parent vers ce noeud */
    double volume_entrant;          /* Volume d'eau qui entre dans ce noeud */
    struct NoeudDistrib *enfant;    /* Premier enfant (liste chainee) */
    struct NoeudDistrib *frere;     /* Prochain frere (liste chainee) */
} NoeudDistrib;

/* Noeud pour l'AVL d'index (pour retrouver rapidement un noeud) */
typedef struct NoeudIndex {
    char identifiant[50];
    NoeudDistrib *pointeur;         /* Pointeur vers le noeud de distribution */
    int hauteur;
    struct NoeudIndex *gauche;
    struct NoeudIndex *droite;
} NoeudIndex;

/* Fonctions pour l'arbre de distribution */
NoeudDistrib* creerNoeudDistrib(char *id, double fuite);
void ajouterEnfant(NoeudDistrib *parent, NoeudDistrib *enfant);

/* Fonctions pour l'AVL d'index */
NoeudIndex* creerNoeudIndex(char *id, NoeudDistrib *pointeur);
NoeudIndex* insererIndex(NoeudIndex *racine, char *id, NoeudDistrib *pointeur);
NoeudDistrib* rechercherIndex(NoeudIndex *racine, char *id);

/* Calcul des fuites */
double calculerFuites(NoeudDistrib *racine, double volume_initial);

/* Liberation memoire */
void libererArbreDistrib(NoeudDistrib *racine);
void libererIndex(NoeudIndex *racine);

#endif
