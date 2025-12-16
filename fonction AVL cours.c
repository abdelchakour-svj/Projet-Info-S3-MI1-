#include <stdio.h>
#include <stdlib.h>

typedef struct avl_struct
{
    int value;             // La valeur du nœud
    int eq;                // Facteur d'équilibre (balance factor)
    struct avl_struct *fg; // Pointeur vers le fils gauche
    struct avl_struct *fd; // Pointeur vers le fils droit
} AVL;


AVL* creerAVL(int e)
{
    // Alloue de la mémoire pour un nouveau nœud
    AVL* new = (AVL* )malloc(sizeof(AVL));
    if (new == NULL)
    {
        exit(EXIT_FAILURE); // Arrêt immédiat en cas d'erreur d'allocation
    }
    new->value = e; // Initialisation de la valeur
    new->fg = NULL; // Pas de fils gauche
    new->fd = NULL; // Pas de fils droit
    new->eq = 0;    // Facteur d'équilibre initialisé à 0
    return new;
}
AVL* rotationGauche(AVL* a)
{
    AVL* pivot = a->fd; // Le fils droit devient le pivot
    int eq_a = a->eq, eq_p = pivot->eq;

    a->fd = pivot->fg; // Le sous-arbre gauche du pivot devient le fils droit de `a`
    pivot->fg = a;     // `a` devient le fils gauche du pivot

    // Mise à jour des facteurs d'équilibre
    a->eq = eq_a - max(eq_p, 0) - 1;
    pivot->eq = min3(eq_a - 2, eq_a + eq_p - 2, eq_p - 1);

    return pivot; // Le pivot devient la nouvelle racine
}

AVL* rotationDroite(AVL* a)
{
    AVL* pivot = a->fg; // Le fils gauche devient le pivot
    int eq_a = a->eq, eq_p = pivot->eq;

    a->fg = pivot->fd; // Le sous-arbre droit du pivot devient le fils gauche de `a`
    pivot->fd = a;     // `a` devient le fils droit du pivot

    // Mise à jour des facteurs d'équilibre
    a->eq = eq_a - min(eq_p, 0) + 1;
    pivot->eq = max3(eq_a + 2, eq_a + eq_p + 2, eq_p + 1);

    return pivot; // Le pivot devient la nouvelle racine
}
AVL* doubleRotationGauche(AVL* a)
{
    a->fd = rotationDroite(a->fd);
    return rotationGauche(a);
}
AVL* doubleRotationDroite(AVL* a)
{
    a->fg = rotationGauche(a->fg);
    return rotationDroite(a);
}
AVL* equilibrerAVL(AVL* a)
{
    if (a->eq >= 2)
    { // Cas où l'arbre est déséquilibré à droite
        if (a->fd->eq >= 0)
        {
            return rotationGauche(a); // Rotation simple gauche
        }
        else
        {
            return doubleRotationGauche(a); // Double rotation gauche
        }
    }
    else if (a->eq <= -2)
    { // Cas où l'arbre est déséquilibré à gauche
        if (a->fg->eq <= 0)
        {
            return rotationDroite(a); // Rotation simple droite
        }
        else
        {
            return doubleRotationDroite(a); // Double rotation droite
        }
    }
    return a; // Aucun rééquilibrage nécessaire
}
AVL* insertionAVL(AVL* a, int e, int *h)
{
    if (a == NULL)
    {           // Si l'arbre est vide, crée un nouveau nœud
        *h = 1; // La hauteur a augmenté
        return creerAVL(e);
    }
    else if (e < a->value)
    { // Si l'élément est plus petit, insérer à gauche
        a->fg = insertionAVL(a->fg, e, h);
        *h = -*h; // Inverse l'impact de la hauteur
    }
    else if (e > a->value)
    { // Si l'élément est plus grand, insérer à droite
        a->fd = insertionAVL(a->fd, e, h);
    }
    else
    { // Élément déjà présent
        *h = 0;
        return a;
    }

    // Mise à jour du facteur d'équilibre et rééquilibrage si nécessaire
    if (*h != 0)
    {
        a->eq += *h;
        a = equilibrerAVL(a);
        *h = (a->eq == 0) ? 0 : 1; // Mise à jour de la hauteur
    }
    return a;
}
AVL* suppMinAVL(AVL* a, int *h, int *pe)
{
    AVL* temp;
    if (a->fg == NULL)
    {                   // Trouvé le plus petit élément
        *pe = a->value; // Sauvegarde la valeur
        *h = -1;        // Réduction de la hauteur
        temp = a;
        a = a->fd; // Le sous-arbre droit devient la racine
        free(temp);
        return a;
    }
    else
    {
        a->fg = suppMinAVL(a->fg, h, pe); // Recherche récursive à gauche
        *h = -*h;
    }

    // Mise à jour et rééquilibrage après suppression
    if (*h != 0)
    {
        a->eq += *h;
        a = equilibrerAVL(a);
        *h = (a->eq == 0) ? -1 : 0;
    }
    return a;
}
AVL* suppressionAVL(AVL* a, int e, int *h)
{
    AVL* temp;
    if (a == NULL)
    { // Élément introuvable
        *h = 0; //attenttion faute dans le CM
        return a;
    }
    if (e > a->value)
    { // Recherche dans le sous-arbre droit
        a->fd = suppressionAVL(a->fd, e, h);
    }
    else if (e < a->value)
    { // Recherche dans le sous-arbre gauche
        a->fg = suppressionAVL(a->fg, e, h);
        *h = -*h;
    }
    else if (a->fd != NULL)
    { // Si le nœud a un fils droit
        a->fd = suppMinAVL(a->fd, h, &(a->value));
    }
    else
    { // Cas du nœud feuille ou avec un seul fils gauche
        temp = a;
        a = a->fg;
        free(temp);
        *h = -1;
        return a;
    }
    if (a==NULL)
    {
        return a;
    }
    // Mise à jour et rééquilibrage après suppression
    if (*h != 0)
    {
        a->eq += *h;
        a = equilibrerAVL(a);
        *h = (a->eq == 0) ? -1 : 0;
    }
    return a;
}
