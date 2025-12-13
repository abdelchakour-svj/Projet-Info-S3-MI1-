#include ... 

typedef struct AVL_struct{
   int valeur;
   int eq;
   struct AVL_struct *fg;
   struct AVL_struct *fd;
// ajt id usine 
}AVL;

AVLNode* creer_noeud_avl(const char *identifiant) {
    AVLNode *noeud = (AVLNode*)malloc(sizeof(AVLNode));
    if (noeud == NULL) {
        fprintf(stderr, "Erreur allocation mémoire\n");
        return NULL;
    }
    
    noeud->identifiant = strdup(identifiant);
    noeud->capacite_max = 0.0;
    noeud->volume_capte = 0.0;
    noeud->volume_traite = 0.0;
    noeud->gauche = NULL;
    noeud->droite = NULL;
    noeud->hauteur = 1;
    
    return noeud;
}}

