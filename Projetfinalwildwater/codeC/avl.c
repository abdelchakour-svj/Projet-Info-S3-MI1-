/*

  avl.c - Implementation de l'arbre AVL
  

   Cet AVL stocke les usines triees par identifiant. Lors de l'insertion,
  si une usine existe deja, ses volumes s'ajoute (captage et traitement).
  equilibre: eq = hauteur(fd) - hauteur(fg)
 
 */


 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

//  Fonctions utilitaires les calculs d equilibre


int max(int a, int b) {
    return (a > b) ? a : b;
}

int min(int a, int b) {
    return (a < b) ? a : b;
}

int max3(int a, int b, int c) {
    return max(max(a, b), c);
}


int min3(int a, int b, int c) {
    return min(min(a, b), c);
}



// Cree un nouveau noeud avec les donnees de l'usine 
NoeudAVL* creerNoeud(Usine usine) {
    NoeudAVL *nouveau = (NoeudAVL*)malloc(sizeof(NoeudAVL));
    if (nouveau == NULL) {
        fprintf(stderr, "Erreur: allocation memoire echouee\n");
        exit(EXIT_FAILURE);
    }
    nouveau->usine = usine;
    nouveau->fg = NULL;
    nouveau->fd = NULL;
    nouveau->eq = 0;  
    return nouveau;
}

// Rotations  gauche et droite


NoeudAVL* rotationGauche(NoeudAVL *a) {
    NoeudAVL *pivot = a->fd;
    int eq_a = a->eq;
    int eq_p = pivot->eq;

    // Effectuer la rotation 
    a->fd = pivot->fg;
    pivot->fg = a;


    a->eq = eq_a - max(eq_p, 0) - 1;
    pivot->eq = min3(eq_a - 2, eq_a + eq_p - 2, eq_p - 1);

    return pivot;
}


NoeudAVL* rotationDroite(NoeudAVL *a) {
    NoeudAVL *pivot = a->fg;
    int eq_a = a->eq;
    int eq_p = pivot->eq;

    a->fg = pivot->fd;
    pivot->fd = a;

 
    a->eq = eq_a - min(eq_p, 0) + 1;
    pivot->eq = max3(eq_a + 2, eq_a + eq_p + 2, eq_p + 1);

    return pivot;
}

/*
 Double rotation gauche (rotation droite-gauche)
  Utilisee quand eq >= 2 et pivot->eq < 0
 */
NoeudAVL* doubleRotationGauche(NoeudAVL *a) {
    a->fd = rotationDroite(a->fd);
    return rotationGauche(a);
}

/*
 Double rotation droite (rotation gauche-droite)
  Utilisee quand eq <= -2 et pivot->eq > 0
 */
NoeudAVL* doubleRotationDroite(NoeudAVL *a) {
    a->fg = rotationGauche(a->fg);
    return rotationDroite(a);
}

// Equilibrage 

/*
  Equilibre l'arbre AVL si necessaire
  Applique les rotations appropriees selon le facteur d'equilibre
 */
NoeudAVL* equilibrerAVL(NoeudAVL *a) {
    if (a->eq >= 2) {
        
        if (a->fd->eq >= 0) {
            return rotationGauche(a);
        } else {
            return doubleRotationGauche(a);
        }
    } else if (a->eq <= -2) {
        
        if (a->fg->eq <= 0) {
            return rotationDroite(a);
        } else {
            return doubleRotationDroite(a);
        }
    }
    return a;  
}

//      Insertion  

/*
 Insere une usine dans l'AVL et reequilibre si necessaire
 h: pointeur pour indiquer si la hauteur a change
  La capacite max n'est mise a jour que si la nouvelle valeur est non nulle.
 Si l'usine existe deja, on cumule les volumes captes et traites.
 */
NoeudAVL* insererAVL(NoeudAVL *a, Usine usine, int *h) {
    int cmp;

   
    if (a == NULL) {
        *h = 1;  
        return creerNoeud(usine);
    }

   
    cmp = strcmp(usine.identifiant, a->usine.identifiant);

    if (cmp < 0) {
        
        a->fg = insererAVL(a->fg, usine, h);
        *h = -*h;  
    } else if (cmp > 0) {
        
        a->fd = insererAVL(a->fd, usine, h);
    } else {
        
        if (usine.capacite_max > 0) {
            a->usine.capacite_max = usine.capacite_max;
        }
        a->usine.volume_capte += usine.volume_capte;
        a->usine.volume_traite += usine.volume_traite;
        *h = 0;
        return a;
    }

    // Mise a jour du facteur d'equilibre et reequilibrage 
    if (*h != 0) {
        a->eq += *h;
        a = equilibrerAVL(a);
        *h = (a->eq == 0) ? 0 : 1;
    }

    return a;
}

//  Recherche  

// Recherche une usine grace a son  identifiant , si elle n existe pas return NULL 
NoeudAVL* rechercherAVL(NoeudAVL *racine, char *identifiant) {
    int cmp;

    if (racine == NULL)
        return NULL;

    cmp = strcmp(identifiant, racine->usine.identifiant);

    if (cmp == 0)
        return racine;
    else if (cmp < 0)
        return rechercherAVL(racine->fg, identifiant);
    else
        return rechercherAVL(racine->fd, identifiant);
}

//       Parcours  :

/* 
  Parcours en ordre inverse (droite, racine, gauche ) 
  usien trier par identifiant 
  Mode: 1=max, 2=src, 3=real, 4=all
 */
void parcoursInverseAVL(NoeudAVL *racine, FILE *fichier, int mode) {
    double valMax, valSrc, valReal;
    
    if (racine == NULL)
        return;


    parcoursInverseAVL(racine->fd, fichier, mode);

    
    valMax = racine->usine.capacite_max / 1000.0;
    valSrc = racine->usine.volume_capte / 1000.0;
    valReal = racine->usine.volume_traite / 1000.0;

    // Ecrire selon le mode 
    if (mode == 1) {
        fprintf(fichier, "%s;%.6f\n", racine->usine.identifiant, valMax);
    } else if (mode == 2) {
        fprintf(fichier, "%s;%.6f\n", racine->usine.identifiant, valSrc);
    } else if (mode == 3) {
        fprintf(fichier, "%s;%.6f\n", racine->usine.identifiant, valReal);
    } else if (mode == 4) {
        fprintf(fichier, "%s;%.6f;%.6f;%.6f \n", 
                racine->usine.identifiant, valReal, valSrc - valReal, valMax - valSrc);
    }

    
    parcoursInverseAVL(racine->fg, fichier, mode);
}

// liberer la memoire :

 
void libererAVL(NoeudAVL *racine) {
    if (racine == NULL)
        return;
    libererAVL(racine->fg);
    libererAVL(racine->fd);
    free(racine);
}


int compterNoeuds(NoeudAVL *racine) {
    if (racine == NULL)
        return 0;
    return 1 + compterNoeuds(racine->fg) + compterNoeuds(racine->fd);
}
