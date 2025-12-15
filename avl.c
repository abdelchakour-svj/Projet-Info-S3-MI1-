#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

/* Retourne le max entre aet b */
int max(int a, int b) {
    if (a > b)
        return a;
    return b;
}

/* Retourne la hauteur d un noeud  */
int hauteur(NoeudAVL *noeud) {
    if (noeud == NULL)
        return 0;
    return noeud->hauteur;
}

/* facteur d'equilibre dun noeud */
int equilibre(NoeudAVL *noeud) {
    if (noeud == NULL)
        return 0;
    return hauteur(noeud->gauche) - hauteur(noeud->droite);
}

/* Cree un nouveau noeud avec les donnees de l'usine */
NoeudAVL* creerNoeud(Usine usine) {
    NoeudAVL *nouveau = (NoeudAVL*)malloc(sizeof(NoeudAVL));
    if (nouveau == NULL) {
        fprintf(stderr, "Erreur: allocation memoire echouee\n");
        return NULL;
    }
    nouveau->usine = usine;
    nouveau->gauche = NULL;
    nouveau->droite = NULL;
    nouveau->hauteur = 1;
    return nouveau;
}
