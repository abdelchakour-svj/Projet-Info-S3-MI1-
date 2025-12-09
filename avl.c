#include ... 

typedef struct AVL_struct{
   int valeur;
   int eq;
   struct AVL_struct *fg;
   struct AVL_struct *fd;
// ajt id usine 
}AVL;



AVL* creationAVL(int e)
{
    AVL* nouveau = (AVL* )malloc(sizeof(AVL));
    if (nouveau == NULL)
    {
        exit(EXIT_FAILURE); 
    }
    nouveau->value = e; 
    nouveau->fg = NULL; 
    nouveau->fd = NULL; 
    nouveau->eq = 0;    
    return nouveau;
}

