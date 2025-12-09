
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

