
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

