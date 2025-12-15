/*
 * avl.c - Implementation de l'arbre AVL
 * Projet C-Wildwater
 * 
 * L'AVL est un arbre binaire de recherche equilibre.
 * L'equilibrage garantit une complexite O(log n).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

/* Retourne le maximum de deux entiers */
int max(int a, int b) {
    if (a > b)
        return a;
    return b;
}

/* Retourne la hauteur d'un noeud (0 si NULL) */
int hauteur(NoeudAVL *noeud) {
    if (noeud == NULL)
        return 0;
    return noeud->hauteur;
}

/* Calcule le facteur d'equilibre d'un noeud */
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

/* Rotation droite autour du noeud y */
NoeudAVL* rotationDroite(NoeudAVL *y) {
    NoeudAVL *x = y->gauche;
    NoeudAVL *T2 = x->droite;

    /* Effectuer la rotation */
    x->droite = y;
    y->gauche = T2;

    /* Mettre a jour les hauteurs */
    y->hauteur = max(hauteur(y->gauche), hauteur(y->droite)) + 1;
    x->hauteur = max(hauteur(x->gauche), hauteur(x->droite)) + 1;

    return x;
}

/* Rotation gauche autour du noeud x */
NoeudAVL* rotationGauche(NoeudAVL *x) {
    NoeudAVL *y = x->droite;
    NoeudAVL *T2 = y->gauche;

    /* Effectuer la rotation */
    y->gauche = x;
    x->droite = T2;

    /* Mettre a jour les hauteurs */
    x->hauteur = max(hauteur(x->gauche), hauteur(x->droite)) + 1;
    y->hauteur = max(hauteur(y->gauche), hauteur(y->droite)) + 1;

    return y;
}

/* Insere une usine dans l'AVL et reequilibre si necessaire */
NoeudAVL* insererAVL(NoeudAVL *racine, Usine usine) {
    int eq;
    int cmp;

    /* Cas de base: arbre vide */
    if (racine == NULL)
        return creerNoeud(usine);

    /* Comparer les identifiants pour trouver la position */
    cmp = strcmp(usine.identifiant, racine->usine.identifiant);

    if (cmp < 0) {
        /* Inserer a gauche */
        racine->gauche = insererAVL(racine->gauche, usine);
    } else if (cmp > 0) {
        /* Inserer a droite */
        racine->droite = insererAVL(racine->droite, usine);
    } else {
        /* Usine deja presente: mettre a jour les valeurs */
        /* Ne mettre a jour capacite_max que si la nouvelle valeur est non nulle */
        if (usine.capacite_max > 0) {
            racine->usine.capacite_max = usine.capacite_max;
        }
        racine->usine.volume_capte += usine.volume_capte;
        racine->usine.volume_traite += usine.volume_traite;
        return racine;
    }

    /* Mettre a jour la hauteur du noeud courant */
    racine->hauteur = 1 + max(hauteur(racine->gauche), hauteur(racine->droite));

    /* Verifier l'equilibre et reequilibrer si necessaire */
    eq = equilibre(racine);

    /* Cas Gauche-Gauche */
    if (eq > 1 && strcmp(usine.identifiant, racine->gauche->usine.identifiant) < 0)
        return rotationDroite(racine);

    /* Cas Droite-Droite */
    if (eq < -1 && strcmp(usine.identifiant, racine->droite->usine.identifiant) > 0)
        return rotationGauche(racine);

    /* Cas Gauche-Droite */
    if (eq > 1 && strcmp(usine.identifiant, racine->gauche->usine.identifiant) > 0) {
        racine->gauche = rotationGauche(racine->gauche);
        return rotationDroite(racine);
    }

    /* Cas Droite-Gauche */
    if (eq < -1 && strcmp(usine.identifiant, racine->droite->usine.identifiant) < 0) {
        racine->droite = rotationDroite(racine->droite);
        return rotationGauche(racine);
    }

    return racine;
}

/* Recherche une usine par son identifiant */
NoeudAVL* rechercherAVL(NoeudAVL *racine, char *identifiant) {
    int cmp;

    if (racine == NULL)
        return NULL;

    cmp = strcmp(identifiant, racine->usine.identifiant);

    if (cmp == 0)
        return racine;
    else if (cmp < 0)
        return rechercherAVL(racine->gauche, identifiant);
    else
        return rechercherAVL(racine->droite, identifiant);
}

/* 
 * Parcours en ordre inverse (droite, racine, gauche) pour tri alphabetique inverse
 * Mode: 1=max, 2=src, 3=real, 4=all
 */
void parcoursInverseAVL(NoeudAVL *racine, FILE *fichier, int mode) {
    double valMax, valSrc, valReal;
    
    if (racine == NULL)
        return;

    /* Parcours droite d'abord pour ordre inverse */
    parcoursInverseAVL(racine->droite, fichier, mode);

    /* Conversion en millions de m3 (diviser par 1000) */
    valMax = racine->usine.capacite_max / 1000.0;
    valSrc = racine->usine.volume_capte / 1000.0;
    valReal = racine->usine.volume_traite / 1000.0;

    /* Ecrire selon le mode */
    if (mode == 1) {
        fprintf(fichier, "%s;%.6f\n", racine->usine.identifiant, valMax);
    } else if (mode == 2) {
        fprintf(fichier, "%s;%.6f\n", racine->usine.identifiant, valSrc);
    } else if (mode == 3) {
        fprintf(fichier, "%s;%.6f\n", racine->usine.identifiant, valReal);
    } else if (mode == 4) {
        /* Mode bonus: toutes les valeurs */
        fprintf(fichier, "%s;%.6f;%.6f;%.6f\n", 
                racine->usine.identifiant, valReal, valSrc - valReal, valMax - valSrc);
    }

    /* Puis parcours gauche */
    parcoursInverseAVL(racine->gauche, fichier, mode);
}

/* Libere la memoire de l'arbre */
void libererAVL(NoeudAVL *racine) {
    if (racine == NULL)
        return;
    libererAVL(racine->gauche);
    libererAVL(racine->droite);
    free(racine);
}

/* Compte le nombre de noeuds dans l'arbre */
int compterNoeuds(NoeudAVL *racine) {
    if (racine == NULL)
        return 0;
    return 1 + compterNoeuds(racine->gauche) + compterNoeuds(racine->droite);
}
