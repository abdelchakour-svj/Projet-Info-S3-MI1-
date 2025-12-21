
// Structure de l'arbre de distribution avec liste chainee d'enfants (Chainon)
// et AVL d'index pour recherche rapide.
 

#ifndef ARBRE_DISTRIB_H
#define ARBRE_DISTRIB_H

// anticipee 
struct chainon;

 
 // Arbre de distribution
 

typedef struct arbre {
    char nom[100];              
    float litre;                
    float fuite_cumule;         
    int nombre_enfant;          
    struct chainon *enfant;     
} Arbre;


 //Chainon pour la liste chainee des enfants
 
typedef struct chainon {
    Arbre *a;                   // Pointeur vers l'enfant 
    struct chainon *suivant;   
} Chainon;


 // AVL d'index pour retrouver rapidement un noeud de l'arbre

 
typedef struct avl_index {
    int eq;                     
    struct avl_index *fg;      
    struct avl_index *fd;      
    char nom[100];              
    Arbre *adresse;             
} AVL_Index;

//       Fonctions pour l'arbre de distribution 


Arbre* creerArbre(char *nom, float fuite);


void ajouterEnfant(Arbre *parent, Arbre *enfant);

//Fonctions pour l'AVL d'index 


AVL_Index* creerAVLIndex(char *nom, Arbre *adresse);

/* Insere un noeud dans l'AVL  */
AVL_Index* insererAVLIndex(AVL_Index *racine, char *nom, Arbre *adresse, int *h);

/* Recherche un noeud Arbre par son nom grace à l'AVL */
Arbre* rechercherAVLIndex(AVL_Index *racine, char *nom);

//      Calcul des fuites 

// Calcule les fuites totales dans l'arbre de distribution 
float calculerFuites(Arbre *racine, float volume_initial);

// Liberation memoire 


void libererArbre(Arbre *racine);

void libererAVLIndex(AVL_Index *racine);

#endif
