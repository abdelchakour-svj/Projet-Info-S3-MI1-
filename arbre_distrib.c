#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arbre_distrib.h"


// static permet de garder cette fonction dans ce fichier et eviter de melanger les fonctions

static int maxInt(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

static int minInt(int a, int b) {
    if (a < b) {
        return a;
    }
    return b;
}

static int max3Int(int a, int b, int c) {
    int maxAB = maxInt(a, b);  // maximum entre a et b

    if (maxAB > c) {
        return maxAB;
    }
    return c;
}

static int min3Int(int a, int b, int c) {
    int minAB = minInt(a, b);  // minimum entre a et b

    if (minAB < c) {
        return minAB;
    }
    return c;
}


/* Cree un nouveau noeud de distribution */
NoeudDistrib* creerNoeudDistrib(char *id, double fuite) {
    NoeudDistrib *nouveau = (NoeudDistrib*)malloc(sizeof(NoeudDistrib));
    if (nouveau == NULL) {
        fprintf(stderr, "Erreur: allocation memoire echouee\n");
        return NULL;
    }
    strncpy(nouveau->identifiant, id, 49);
    nouveau->identifiant[49] = '\0';
    nouveau->pourcentage_fuite = fuite;
    nouveau->volume_entrant = 0.0;
    nouveau->enfant = NULL;
    nouveau->frere = NULL;
    return nouveau;
}

/* Ajoute un enfant a un noeud parent */
void ajouterEnfant(NoeudDistrib *parent, NoeudDistrib *enfant) {
    if (parent == NULL || enfant == NULL)
        return;

    if (parent->enfant == NULL) {
        /* Premier enfant */
        parent->enfant = enfant;
    } else {
        /* Ajouter en tete de la liste des freres */
        enfant->frere = parent->enfant;
        parent->enfant = enfant;
    }
}

/* ========== AVL d'index pour recherche rapide ========== */

/* Retourne le max de deux entiers */
static int maxIndex(int a, int b) {
    return (a > b) ? a : b;
}

/* Retourne la hauteur d'un noeud d'index */
static int hauteurIndex(NoeudIndex *n) {
    if (n == NULL)
        return 0;
    return n->hauteur;
}

/* Cree un nouveau noeud d'index */
NoeudIndex* creerNoeudIndex(char *id, NoeudDistrib *pointeur) {
    NoeudIndex *nouveau = (NoeudIndex*)malloc(sizeof(NoeudIndex));
    if (nouveau == NULL) {
        fprintf(stderr, "Erreur: allocation memoire echouee\n");
        return NULL;
    }
    strncpy(nouveau->identifiant, id, 49);
    nouveau->identifiant[49] = '\0';
    nouveau->pointeur = pointeur;
    nouveau->hauteur = 1;
    nouveau->gauche = NULL;
    nouveau->droite = NULL;
    return nouveau;
}

/* Rotation droite pour l'AVL d'index */
static NoeudIndex* rotationDroiteIndex(NoeudIndex *y) {
    NoeudIndex *x = y->gauche;
    NoeudIndex *T2 = x->droite;

    x->droite = y;
    y->gauche = T2;

    y->hauteur = maxIndex(hauteurIndex(y->gauche), hauteurIndex(y->droite)) + 1;
    x->hauteur = maxIndex(hauteurIndex(x->gauche), hauteurIndex(x->droite)) + 1;

    return x;
}

/* Rotation gauche pour l'AVL d'index */
static NoeudIndex* rotationGaucheIndex(NoeudIndex *x) {
    NoeudIndex *y = x->droite;
    NoeudIndex *T2 = y->gauche;

    y->gauche = x;
    x->droite = T2;

    x->hauteur = maxIndex(hauteurIndex(x->gauche), hauteurIndex(x->droite)) + 1;
    y->hauteur = maxIndex(hauteurIndex(y->gauche), hauteurIndex(y->droite)) + 1;

    return y;
}

/* Calcule le facteur d'equilibre */
static int equilibreIndex(NoeudIndex *n) {
    if (n == NULL)
        return 0;
    return hauteurIndex(n->gauche) - hauteurIndex(n->droite);
}

/* Insere un noeud dans l'AVL d'index */
NoeudIndex* insererIndex(NoeudIndex *racine, char *id, NoeudDistrib *pointeur) {
    int cmp;
    int eq;

    if (racine == NULL)
        return creerNoeudIndex(id, pointeur);

    cmp = strcmp(id, racine->identifiant);

    if (cmp < 0) {
        racine->gauche = insererIndex(racine->gauche, id, pointeur);
    } else if (cmp > 0) {
        racine->droite = insererIndex(racine->droite, id, pointeur);
    } else {
        /* Deja present, mettre a jour le pointeur */
        racine->pointeur = pointeur;
        return racine;
    }

    /* Mettre a jour la hauteur */
    racine->hauteur = 1 + maxIndex(hauteurIndex(racine->gauche), hauteurIndex(racine->droite));

    /* Equilibrer */
    eq = equilibreIndex(racine);

    /* Cas Gauche-Gauche */
    if (eq > 1 && strcmp(id, racine->gauche->identifiant) < 0)
        return rotationDroiteIndex(racine);

    /* Cas Droite-Droite */
    if (eq < -1 && strcmp(id, racine->droite->identifiant) > 0)
        return rotationGaucheIndex(racine);

    /* Cas Gauche-Droite */
    if (eq > 1 && strcmp(id, racine->gauche->identifiant) > 0) {
        racine->gauche = rotationGaucheIndex(racine->gauche);
        return rotationDroiteIndex(racine);
    }

    /* Cas Droite-Gauche */
    if (eq < -1 && strcmp(id, racine->droite->identifiant) < 0) {
        racine->droite = rotationDroiteIndex(racine->droite);
        return rotationGaucheIndex(racine);
    }

    return racine;
}

/* Recherche un noeud dans l'AVL d'index */
NoeudDistrib* rechercherIndex(NoeudIndex *racine, char *id) {
    int cmp;

    if (racine == NULL)
        return NULL;

    cmp = strcmp(id, racine->identifiant);

    if (cmp == 0)
        return racine->pointeur;
    else if (cmp < 0)
        return rechercherIndex(racine->gauche, id);
    else
        return rechercherIndex(racine->droite, id);
}

/* 
 * Calcule les fuites totales dans l'arbre de distribution
 * volume_initial: volume d'eau qui entre dans le noeud racine
 * Retourne le total des fuites en M.m3
 */
double calculerFuites(NoeudDistrib *racine, double volume_initial) {
    double fuites = 0.0;
    double volume_apres_fuite;
    double volume_par_enfant;
    int nb_enfants = 0;
    NoeudDistrib *enfant;

    if (racine == NULL)
        return 0.0;

    /* Le volume entrant est celui recu du parent */
    racine->volume_entrant = volume_initial;

    /* Calculer la fuite sur ce troncon */
    fuites = volume_initial * (racine->pourcentage_fuite / 100.0);
    volume_apres_fuite = volume_initial - fuites;

    /* Compter le nombre d'enfants */
    enfant = racine->enfant;
    while (enfant != NULL) {
        nb_enfants++;
        enfant = enfant->frere;
    }

    /* Repartir equitablement le volume entre les enfants */
    if (nb_enfants > 0) {
        volume_par_enfant = volume_apres_fuite / nb_enfants;
        enfant = racine->enfant;
        while (enfant != NULL) {
            fuites += calculerFuites(enfant, volume_par_enfant);
            enfant = enfant->frere;
        }
    }

    return fuites;
}

/* Libere l'arbre de distribution */
void libererArbreDistrib(NoeudDistrib *racine) {
    if (racine == NULL)
        return;
    libererArbreDistrib(racine->enfant);
    libererArbreDistrib(racine->frere);
    free(racine);
}

/* Libere l'AVL d'index */
void libererIndex(NoeudIndex *racine) {
    if (racine == NULL)
        return;
    libererIndex(racine->gauche);
    libererIndex(racine->droite);
    free(racine);
}
