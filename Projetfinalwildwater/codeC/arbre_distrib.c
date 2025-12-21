//L'arbre de distribution permet de calculer les fuites d'eau dans tout le reseau aval d'une usine.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arbre_distrib.h"

// structure utilisées (arbre_distrib.h) : 
// Arbre: noeud reseau de distribution
//Chainon: liste chainee des enfants d'un noeud
//AVL_Index: AVL pour recherche rapide sur un noeud grace a son nom.

// static permet de garder cette fonction dans ce fichier et eviter de melanger les fonctions

static int maxInt(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

static int minInt(int a,int b) {
    if (a < b) {
        return a;
    }
    return b;
}

static int max3Int(int a, int b, int c) {
    int maxAB = maxInt(a, b);  

    if (maxAB > c) {
        return maxAB;
    }
    return c;
}

static int min3Int(int a, int b, int c) {
    int minAB = minInt(a, b); 

    if (minAB < c) {
        return minAB;
    }
    return c;
}

//Fonctions pour l'arbre de distribution


Arbre* creerArbre(char *nom, float fuite) {
    Arbre *nouveau = (Arbre*)malloc(sizeof(Arbre));
    if (nouveau == NULL) {
        fprintf(stderr, "Erreur : allocation memoire echouee pour Arbre\n");
        exit(EXIT_FAILURE);
    }
    strncpy(nouveau->nom, nom, 99);
    nouveau->nom[99] = '\0';
    nouveau->litre = 0.0f;
    nouveau->fuite_cumule = fuite;
    nouveau->nombre_enfant = 0;
    nouveau->enfant = NULL;
    return nouveau;
}


static Chainon* creerChainon(Arbre *a) {
    Chainon *nouveau = (Chainon*)malloc(sizeof(Chainon));
    if (nouveau == NULL) {
        fprintf(stderr, "Erreur: allocation memoire echouee pour Chainon\n");
        exit(EXIT_FAILURE);
    }
    nouveau->a = a;
    nouveau->suivant = NULL;
    return nouveau;
}



void ajouterEnfant(Arbre *parent, Arbre *enfant) {
    Chainon *nouveau;
    
    if (parent == NULL || enfant == NULL)
        return;


    nouveau = creerChainon(enfant);

    
    nouveau->suivant = parent->enfant;
    parent->enfant = nouveau;
    parent->nombre_enfant++;
}

//  Fonctions pour l'AVL   

AVL_Index* creerAVLIndex(char *nom, Arbre *adresse) {
    AVL_Index *nouveau = (AVL_Index*)malloc(sizeof(AVL_Index));
    if (nouveau == NULL) {
        fprintf(stderr, "Erreur: allocation memoire echouee pour AVL_Index\n");
        exit(EXIT_FAILURE);
    }
    strncpy(nouveau->nom, nom, 99);
    nouveau->nom[99] = '\0';
    nouveau->adresse = adresse;
    nouveau->eq = 0;
    nouveau->fg = NULL;
    nouveau->fd = NULL;
    return nouveau;
}

 
static AVL_Index* rotationGaucheIndex(AVL_Index *a) {
    AVL_Index *pivot = a->fd;
    int eq_a = a->eq;
    int eq_p = pivot->eq;

    a->fd = pivot->fg;
    pivot->fg = a;

    
    a->eq = eq_a - maxInt(eq_p, 0) - 1;
    pivot->eq = min3Int(eq_a - 2, eq_a + eq_p - 2, eq_p - 1);

    return pivot;
}


static AVL_Index* rotationDroiteIndex(AVL_Index *a) {
    AVL_Index *pivot = a->fg;
    int eq_a = a->eq;
    int eq_p = pivot->eq;

    a->fg = pivot->fd;
    pivot->fd = a;


    a->eq = eq_a - minInt(eq_p, 0) + 1;
    pivot->eq = max3Int(eq_a + 2, eq_a + eq_p + 2, eq_p + 1);

    return pivot;
}


static AVL_Index* doubleRotationGaucheIndex(AVL_Index *a) {
    a->fd = rotationDroiteIndex(a->fd);
    return rotationGaucheIndex(a);
}


static AVL_Index* doubleRotationDroiteIndex(AVL_Index *a) {
    a->fg = rotationGaucheIndex(a->fg);
    return rotationDroiteIndex(a);
}


static AVL_Index* equilibrerAVLIndex(AVL_Index *a) {
    if (a->eq >= 2) {
       
        if (a->fd->eq >= 0) {
            return rotationGaucheIndex(a);
        } else {
            return doubleRotationGaucheIndex(a);
        }
    } else if (a->eq <= -2) {
 
        if (a->fg->eq <= 0) {
            return rotationDroiteIndex(a);
        } else {
            return doubleRotationDroiteIndex(a);
        }
    }
    return a;
}

// h : pointeur pour hauteur
AVL_Index* insererAVLIndex(AVL_Index *racine, char *nom, Arbre *adresse, int *h) {
    int cmp;

    if (racine == NULL) {
        *h = 1;
        return creerAVLIndex(nom, adresse);
    }

    cmp = strcmp(nom, racine->nom);

    if (cmp < 0) {
      
        racine->fg = insererAVLIndex(racine->fg, nom, adresse, h);
        *h = -*h;  
    } else if (cmp > 0) {
     
        racine->fd = insererAVLIndex(racine->fd, nom, adresse, h);
    } else {
    
        racine->adresse = adresse;
        *h = 0;
        return racine;
    }

   
    if (*h != 0) {
        racine->eq += *h;
        racine = equilibrerAVLIndex(racine);
        *h = (racine->eq == 0) ? 0 : 1;
    }

    return racine;
}


Arbre* rechercherAVLIndex(AVL_Index *racine, char *nom) {
    int cmp;

    if (racine == NULL)
        return NULL;

    cmp = strcmp(nom, racine->nom);

    if (cmp == 0)
        return racine->adresse;
    else if (cmp < 0)
        return rechercherAVLIndex(racine->fg, nom);
    else
        return rechercherAVLIndex(racine->fd, nom);
}

// Calcul des fuites 
//Calcule les fuites totales dans l'arbre de distribution

// on stock le volume dans le noeud 
// on calcule les fuite
// le volume est reparti entre les enfants 
// appelle recusive pour chaque enfant

float calculerFuites(Arbre *racine, float volume_initial) {
    float fuites = 0.0f;
    float volume_apres_fuite;
    float volume_par_enfant;
    Chainon *c;

    if (racine == NULL)
        return 0.0f;


    racine->litre = volume_initial;


    fuites = volume_initial * (racine->fuite_cumule / 100.0f);
    volume_apres_fuite = volume_initial - fuites;


    if (racine->nombre_enfant > 0) {
        volume_par_enfant = volume_apres_fuite / racine->nombre_enfant;
        
     
        c = racine->enfant;
        while (c != NULL) {
            fuites += calculerFuites(c->a, volume_par_enfant);
            c = c->suivant;
        }
    }

    return fuites;
}

// Libérer memoire 


static void libererChainons(Chainon *c) {
    Chainon *suivant;
    while (c != NULL) {
        suivant = c->suivant;
 
        libererArbre(c->a);
        free(c);
        c = suivant;
    }
}


void libererArbre(Arbre *racine) {
    if (racine == NULL)
        return;
    

    libererChainons(racine->enfant);
    

    free(racine);
}


void libererAVLIndex(AVL_Index *racine) {
    if (racine == NULL)
        return;
    libererAVLIndex(racine->fg);
    libererAVLIndex(racine->fd);
    free(racine);
}
