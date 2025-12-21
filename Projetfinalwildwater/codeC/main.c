/*
 * ce programme peut generer des histogrammes ou calculer les fuites d'une usine.
 * leaks " id"
 * Modes pour histo: max, src, real
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"
#include "arbre_distrib.h"

#define TAILLE_LIGNE 256

/* 
 * Traitement histogramme: lit le fichier filtrer par le   Shell,
 * construit un AVL des usines en cumulant les volumes captes et traites,
 * Mode: 1=capacite max, 2=volume capte, 3=volume traite, 4=les trois
 * mode: 1=max, 2=src, 3=real, 4=all
 */
int traiterHistogramme(char *fichierEntree, char *fichierSortie, int mode) {
    FILE *fIn, *fOut;
    char ligne[TAILLE_LIGNE];
    char col1[50], col2[50], col3[50], col4[50], col5[50];
    NoeudAVL *racine = NULL;
    Usine usine;
    int nbChamps;
    int h;
    double volumeCapte, pourcentageFuite;

    
    fIn = fopen(fichierEntree, "r");
    if (fIn == NULL) {
        fprintf(stderr, "Erreur: impossible d'ouvrir %s\n", fichierEntree);
        return 1;
    }

    
    while (fgets(ligne, TAILLE_LIGNE, fIn) != NULL) {
        col1[0] = '\0';
        col2[0] = '\0';
        col3[0] = '\0';
        col4[0] = '\0';
        col5[0] = '\0';

        nbChamps = sscanf(ligne, "%49[^;];%49[^;];%49[^;];%49[^;];%49[^\n]",
                          col1, col2, col3, col4, col5);

        if (nbChamps < 2)
            continue;

        // Ligne d'usine: -;Usine;-;capacite;- 
        if (strcmp(col1, "-") == 0 && strcmp(col3, "-") == 0 && 
            strcmp(col5, "-") == 0 && strlen(col4) > 0) {
            if (strstr(col2, "Plant") != NULL || strstr(col2, "Module") != NULL ||
                strstr(col2, "Unit") != NULL || strstr(col2, "Facility") != NULL) {
                memset(&usine, 0, sizeof(Usine));
                strncpy(usine.identifiant, col2, 49);
                usine.identifiant[49] = '\0';
                usine.capacite_max = atof(col4);
                usine.volume_capte = 0.0;
                usine.volume_traite = 0.0;
                h = 0;
                racine = insererAVL(racine, usine, &h);
            }
        }
        // Ligne de captage: -;Source;Usine;volume;pourcentage 
        else if (strcmp(col1, "-") == 0 && strlen(col4) > 0 && strcmp(col4, "-") != 0 &&
                 strlen(col5) > 0 && strcmp(col5, "-") != 0) {
            if ((strstr(col2, "Source") != NULL || strstr(col2, "Well") != NULL ||
                 strstr(col2, "Spring") != NULL || strstr(col2, "Fountain") != NULL ||
                 strstr(col2, "Resurgence") != NULL) &&
                (strstr(col3, "Plant") != NULL || strstr(col3, "Module") != NULL ||
                 strstr(col3, "Unit") != NULL || strstr(col3, "Facility") != NULL)) {
                
                volumeCapte = atof(col4);
                pourcentageFuite = atof(col5);

                memset(&usine, 0, sizeof(Usine));
                strncpy(usine.identifiant, col3, 49);
                usine.identifiant[49] = '\0';
                usine.capacite_max = 0.0;
                usine.volume_capte = volumeCapte;
                usine.volume_traite = volumeCapte * (1.0 - pourcentageFuite / 100.0);
                
                h = 0;
                racine = insererAVL(racine, usine, &h);
            }
        }
    }

    fclose(fIn);

    
    fOut = fopen(fichierSortie, "w");
    if (fOut == NULL) {
        fprintf(stderr, "Erreur:impossible de creer %s\n", fichierSortie);
        libererAVL(racine);
        return 1 ;
    }

    /* ecrire l'en-tete */
    if (mode == 1) {
        fprintf(fOut, "identifier;max volume(M.m3.year-1)\n");
    } else if (mode == 2) {
        fprintf(fOut, "identifier;source volume (M.m3.year-1)\n");
    } else if (mode == 3) {
        fprintf(fOut, "identifier;real volume (M.m3.year-1)\n");
    } else if (mode == 4) {
        fprintf(fOut, "identifier;real volume;lost volume;available capacity\n");
    }

    parcoursInverseAVL(racine, fOut, mode);

    fclose(fOut);
    printf("Traitement histogramme terminer avec succes\n");
    libererAVL(racine );
    return 0;
}

/*
 * Traitement pour calculer les fuites d'une usine

 * - Un Arbre pour representer le reseau de distribution
 * aval de l'usine,puis calcule recursivement les pertes d'eau
 * - Un AVLIndex pour retrouver rapidement les noeuds par leurs nom
* puis ajouter enfants
 */
int traiterFuites(char *fichierEntree, char *fichierSortie, char *idUsine) {
    FILE *fIn, *fOut;
    char ligne[TAILLE_LIGNE];
    char col1[50], col2[50], col3[50], col4[50], col5[50];
    int nbChamps;
    int usine_trouvee = 0;
    int h;
    float volume_initial = 0.0f;
    float fuites_totales = 0.0f;
    float pourcentage;

    /* Arbre de distribution et AVL d'index */
    Arbre *racineArbre = NULL;
    AVL_Index *racineIndex = NULL;
    Arbre *parent, *nouveau;

    /* Ouvrir le fichier d'entree */
    fIn = fopen(fichierEntree, "r");
    if (fIn == NULL) {
        fprintf(stderr, "Erreur: impossible d'ouvrir %s\n", fichierEntree);
        return 1;
    }

    /*1) calculer le volume initial  */
    /* On cherche les lignes source -> usine pour notre usine */
    while (fgets(ligne, TAILLE_LIGNE, fIn) != NULL) {
        col1[0] = '\0';
        col2[0] = '\0';
        col3[0] = '\0';
        col4[0] = '\0';
        col5[0] = '\0';

        nbChamps = sscanf(ligne, "%49[^;];%49[^;];%49[^;];%49[^;];%49[^\n]",
                          col1, col2, col3, col4, col5);

        if (nbChamps < 2)
            continue;

        /* Ligne source ->usine: -;Source;Usine;volume;pourcentage */
        if (strcmp(col1, "-") == 0 && strcmp(col3, idUsine) == 0 &&
            strlen(col4) > 0 && strcmp(col4, "-") != 0 &&
            strlen(col5) > 0 && strcmp(col5, "-") != 0) {
            float vol = (float)atof(col4);
            float fuite = (float)atof(col5);
            volume_initial += vol * (1.0f - fuite / 100.0f);
            usine_trouvee = 1;
        }
    }

    /* Si n existe pas , ecrire -1 */
    if (!usine_trouvee) {
        fclose(fIn);
        fOut = fopen(fichierSortie, "a");
        if (fOut == NULL) {
            fprintf(stderr, "Erreur: impossible d'ouvrir %s\n", fichierSortie);
            return 1;
        }
        fprintf(fOut, "%s;-1\n", idUsine);
        fclose(fOut);
        return 0;
    }

    /* 2) construire l'arbre de distribution  */
    rewind(fIn);

    //Creer le noeud racine (l'usine elle-meme) 
    racineArbre = creerArbre(idUsine, 0.0f);
    h = 0;
    racineIndex = insererAVLIndex(racineIndex, idUsine, racineArbre, &h);

    while (fgets(ligne, TAILLE_LIGNE, fIn) != NULL) {
        col1[0] = '\0';
        col2[0]= '\0';
        col3[0]= '\0';
        col4[0]= '\0';
        col5[0]= '\0';

        nbChamps = sscanf(ligne, "%49[^;];%49[^;];%49[^;];%49[^;];%49[^\n]",
                          col1, col2, col3, col4, col5);

        if (nbChamps < 3)
            continue;

        /* 
           *Verifier si cette ligne concerne notre usine:
         * - col1 contient l'usine (pour distribution)
         * -OU bien col1 = "-" et col2 = usine (pour usine-> stockage)
         */
        if ((strcmp(col1, idUsine) == 0) || 
            (strcmp(col1, "-") == 0 && strcmp(col2, idUsine) == 0)) {
            
            /* Recuperer le pourcentage de fuite */
            if (strlen(col5) > 0 && strcmp(col5, "-") != 0) {
                pourcentage = (float)atof(col5);
            } else {
                pourcentage = 0.0f;
            }

            //Chercher le parent dans l'AVL d'index 
            parent = rechercherAVLIndex(racineIndex, col2);
            
            if (parent != NULL && strlen(col3) > 0 && strcmp(col3, "-") != 0) {
                
                nouveau = creerArbre(col3, pourcentage);
                
           
                ajouterEnfant(parent, nouveau);
                
                /* Ajouter au AVL d'index pour pouvoir le retrouver */
                h = 0;
                racineIndex =insererAVLIndex(racineIndex, col3, nouveau, &h);
            }
        }
    }

    fclose(fIn );

    //   Calculer les fuites 
    fuites_totales = calculerFuites(racineArbre, volume_initial);

    
    fuites_totales = fuites_totales /1000.0f;

    /*Ecrire le resultat */
    fOut = fopen(fichierSortie, "a");
    if (fOut == NULL) {
        fprintf(stderr, "Erreur: impossible d'ouvrir %s \n", fichierSortie);
        libererArbre(racineArbre);
        libererAVLIndex(racineIndex);
        return 1;
    }

    fprintf(fOut, "%s;%.6f\n", idUsine, fuites_totales);
    fclose(fOut) ;

    /* Liberer la memoire*/
    libererArbre(racineArbre);
    libererAVLIndex(racineIndex );

    printf("Fuites calculer pour %s: %.6f M.m3\n", idUsine, fuites_totales);
    return 0;
}

// Fonction principale : analyse des arguments
int main(int argc, char *argv[]) {
    int mode;

    if (argc < 5 ) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s histo <mode> <fichier_entree> <fichier_sortie>\n", argv[0]);
        fprintf(stderr, "  %s leaks <id_usine> <fichier_entree> <fichier_sortie>\n", argv[0]);
        fprintf(stderr, "Modes: max, src, real, all \n");
        return 1 ;
    }

    if (strcmp(argv[1], "histo") == 0 ) {
        if (strcmp(argv[2], "max") == 0) mode = 1;
        else if (strcmp(argv[2], "src") == 0) mode = 2;
        else if (strcmp(argv[2], "real") == 0) mode = 3;
        else if (strcmp(argv[2],"all") == 0) mode = 4;
        else {
            fprintf(stderr, "erreur:mode inconnu '%s'\n", argv[2]);
            return 1;
        }
        return traiterHistogramme(argv[3], argv[4], mode);
    }
    else if (strcmp (argv[1], "leaks") == 0) {
        return traiterFuites(argv[3], argv[4], argv[2]);
    }
    else {
        fprintf(stderr, "Erreur: commande inconnue '%s'\n",argv[1]);
        return 1 ;
    }
}
