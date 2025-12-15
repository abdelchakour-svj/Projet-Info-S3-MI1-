/*
 * main.c - Programme principal
 * Projet C-Wildwater
 * 
 * Ce programme traite les donnees du reseau de distribution d'eau.
 * Il peut generer des histogrammes ou calculer les fuites d'une usine.
 * 
 * Usage:
 *   ./wildwater histo <mode> <fichier_entree> <fichier_sortie>
 *   ./wildwater leaks <id_usine> <fichier_entree> <fichier_sortie>
 * 
 * Modes pour histo: max, src, real, all
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"
#include "arbre_distrib.h"

#define TAILLE_LIGNE 256

/* 
 * Traitement pour generer l'histogramme des usines
 * mode: 1=max, 2=src, 3=real, 4=all
 */
int traiterHistogramme(char *fichierEntree, char *fichierSortie, int mode) {
    FILE *fIn, *fOut;
    char ligne[TAILLE_LIGNE];
    char col1[50], col2[50], col3[50], col4[50], col5[50];
    NoeudAVL *racine = NULL;
    Usine usine;
    int nbChamps;
    double volumeCapte, pourcentageFuite;

    /* Ouvrir le fichier d'entree */
    fIn = fopen(fichierEntree, "r");
    if (fIn == NULL) {
        fprintf(stderr, "Erreur: impossible d'ouvrir %s\n", fichierEntree);
        return 1;
    }

    /* Lire chaque ligne du fichier */
    while (fgets(ligne, TAILLE_LIGNE, fIn) != NULL) {
        /* Initialiser les colonnes */
        col1[0] = '\0';
        col2[0] = '\0';
        col3[0] = '\0';
        col4[0] = '\0';
        col5[0] = '\0';

        /* Parser la ligne avec sscanf */
        nbChamps = sscanf(ligne, "%49[^;];%49[^;];%49[^;];%49[^;];%49[^\n]",
                          col1, col2, col3, col4, col5);

        if (nbChamps < 2)
            continue;

        /* Verifier si c'est une ligne d'usine (description de capacite) */
        /* Format: -;Usine;-;capacite;- */
        if (strcmp(col1, "-") == 0 && strcmp(col3, "-") == 0 && 
            strcmp(col5, "-") == 0 && strlen(col4) > 0) {
            /* C'est une ligne de description d'usine */
            /* Verifier que c'est bien une usine (Plant, Module, Unit, Facility) */
            if (strstr(col2, "Plant") != NULL || strstr(col2, "Module") != NULL ||
                strstr(col2, "Unit") != NULL || strstr(col2, "Facility") != NULL) {
                memset(&usine, 0, sizeof(Usine));
                strncpy(usine.identifiant, col2, 49);
                usine.capacite_max = atof(col4);
                usine.volume_capte = 0.0;
                usine.volume_traite = 0.0;
                racine = insererAVL(racine, usine);
            }
        }
        /* Verifier si c'est une ligne source -> usine (captage) */
        /* Format: -;Source;Usine;volume;pourcentage */
        else if (strcmp(col1, "-") == 0 && strlen(col4) > 0 && strcmp(col4, "-") != 0 &&
                 strlen(col5) > 0 && strcmp(col5, "-") != 0) {
            /* C'est une ligne de captage */
            /* Verifier que col2 est une source et col3 est une usine */
            if ((strstr(col2, "Source") != NULL || strstr(col2, "Well") != NULL ||
                 strstr(col2, "Spring") != NULL || strstr(col2, "Fountain") != NULL ||
                 strstr(col2, "Resurgence") != NULL) &&
                (strstr(col3, "Plant") != NULL || strstr(col3, "Module") != NULL ||
                 strstr(col3, "Unit") != NULL || strstr(col3, "Facility") != NULL)) {
                
                volumeCapte = atof(col4);
                pourcentageFuite = atof(col5);

                /* Chercher ou creer l'usine */
                memset(&usine, 0, sizeof(Usine));
                strncpy(usine.identifiant, col3, 49);
                usine.capacite_max = 0.0;
                usine.volume_capte = volumeCapte;
                /* Volume traite = volume capte - fuites */
                usine.volume_traite = volumeCapte * (1.0 - pourcentageFuite / 100.0);
                
                racine = insererAVL(racine, usine);
            }
        }
    }

    fclose(fIn);

    /* Ouvrir le fichier de sortie */
    fOut = fopen(fichierSortie, "w");
    if (fOut == NULL) {
        fprintf(stderr, "Erreur: impossible de creer %s\n", fichierSortie);
        libererAVL(racine);
        return 1;
    }

    /* Ecrire l'en-tete selon le mode */
    if (mode == 1) {
        fprintf(fOut, "identifier;max volume (M.m3.year-1)\n");
    } else if (mode == 2) {
        fprintf(fOut, "identifier;source volume (M.m3.year-1)\n");
    } else if (mode == 3) {
        fprintf(fOut, "identifier;real volume (M.m3.year-1)\n");
    } else if (mode == 4) {
        fprintf(fOut, "identifier;real volume;lost volume;available capacity\n");
    }

    /* Ecrire les donnees en ordre alphabetique inverse */
    parcoursInverseAVL(racine, fOut, mode);

    fclose(fOut);
    
    printf("Traitement histogramme termine avec succes\n");
    
    libererAVL(racine);
    return 0;
}

/*
 * Traitement pour calculer les fuites d'une usine
 */
int traiterFuites(char *fichierEntree, char *fichierSortie, char *idUsine) {
    FILE *fIn, *fOut;
    char ligne[TAILLE_LIGNE];
    char col1[50], col2[50], col3[50], col4[50], col5[50];
    int nbChamps;
    int usine_trouvee = 0;
    double volume_initial = 0.0;
    double fuites_totales = 0.0;
    double pourcentage;

    NoeudDistrib *racineArbre = NULL;
    NoeudIndex *racineIndex = NULL;
    NoeudDistrib *parent, *nouveau;

    /* Ouvrir le fichier d'entree */
    fIn = fopen(fichierEntree, "r");
    if (fIn == NULL) {
        fprintf(stderr, "Erreur: impossible d'ouvrir %s\n", fichierEntree);
        return 1;
    }

    /* Premiere passe: trouver l'usine et calculer le volume initial */
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

        /* Chercher les lignes source -> usine pour cette usine */
        if (strcmp(col1, "-") == 0 && strcmp(col3, idUsine) == 0 &&
            strlen(col4) > 0 && strcmp(col4, "-") != 0 &&
            strlen(col5) > 0 && strcmp(col5, "-") != 0) {
            /* Ajouter le volume traite (apres fuites de captage) */
            double vol = atof(col4);
            double fuite = atof(col5);
            volume_initial += vol * (1.0 - fuite / 100.0);
            usine_trouvee = 1;
        }
    }

    /* Si l'usine n'est pas trouvee */
    if (!usine_trouvee) {
        fclose(fIn);
        /* Ecrire -1 dans le fichier de sortie */
        fOut = fopen(fichierSortie, "a");
        if (fOut == NULL) {
            fprintf(stderr, "Erreur: impossible d'ouvrir %s\n", fichierSortie);
            return 1;
        }
        fprintf(fOut, "%s;-1\n", idUsine);
        fclose(fOut);
        return 0;
    }

    /* Revenir au debut du fichier */
    rewind(fIn);

    /* Creer le noeud racine (l'usine) */
    racineArbre = creerNoeudDistrib(idUsine, 0.0);
    racineIndex = insererIndex(racineIndex, idUsine, racineArbre);

    /* Deuxieme passe: construire l'arbre de distribution */
    while (fgets(ligne, TAILLE_LIGNE, fIn) != NULL) {
        col1[0] = '\0';
        col2[0] = '\0';
        col3[0] = '\0';
        col4[0] = '\0';
        col5[0] = '\0';

        nbChamps = sscanf(ligne, "%49[^;];%49[^;];%49[^;];%49[^;];%49[^\n]",
                          col1, col2, col3, col4, col5);

        if (nbChamps < 3)
            continue;

        /* Traiter les lignes de distribution de cette usine */
        /* Usine -> Storage, Storage -> Junction, Junction -> Service, Service -> Cust */
        
        /* Verifier si cette ligne concerne notre usine */
        /* col1 contient l'usine (sauf pour usine->storage ou col1 = "-") */
        if ((strcmp(col1, idUsine) == 0) || 
            (strcmp(col1, "-") == 0 && strcmp(col2, idUsine) == 0)) {
            
            /* Recuperer le pourcentage de fuite */
            if (strlen(col5) > 0 && strcmp(col5, "-") != 0) {
                pourcentage = atof(col5);
            } else {
                pourcentage = 0.0;
            }

            /* Chercher le parent dans l'index */
            parent = rechercherIndex(racineIndex, col2);
            
            if (parent != NULL && strlen(col3) > 0 && strcmp(col3, "-") != 0) {
                /* Creer le nouveau noeud */
                nouveau = creerNoeudDistrib(col3, pourcentage);
                if (nouveau != NULL) {
                    ajouterEnfant(parent, nouveau);
                    racineIndex = insererIndex(racineIndex, col3, nouveau);
                }
            }
        }
    }

    fclose(fIn);

    /* Calculer les fuites */
    fuites_totales = calculerFuites(racineArbre, volume_initial);

    /* Convertir en millions de m3 */
    fuites_totales = fuites_totales / 1000.0;

    /* Ecrire dans le fichier de sortie (mode append) */
    fOut = fopen(fichierSortie, "a");
    if (fOut == NULL) {
        fprintf(stderr, "Erreur: impossible d'ouvrir %s\n", fichierSortie);
        libererArbreDistrib(racineArbre);
        libererIndex(racineIndex);
        return 1;
    }

    fprintf(fOut, "%s;%.6f\n", idUsine, fuites_totales);
    fclose(fOut);

    /* Liberer la memoire */
    libererArbreDistrib(racineArbre);
    libererIndex(racineIndex);

    printf("Fuites calculees pour %s: %.6f M.m3\n", idUsine, fuites_totales);
    return 0;
}

/* Fonction principale */
int main(int argc, char *argv[]) {
    int mode;

    /* Verifier le nombre d'arguments */
    if (argc < 5) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s histo <mode> <fichier_entree> <fichier_sortie>\n", argv[0]);
        fprintf(stderr, "  %s leaks <id_usine> <fichier_entree> <fichier_sortie>\n", argv[0]);
        fprintf(stderr, "Modes: max, src, real, all\n");
        return 1;
    }

    /* Traitement histogramme */
    if (strcmp(argv[1], "histo") == 0) {
        /* Determiner le mode */
        if (strcmp(argv[2], "max") == 0) {
            mode = 1;
        } else if (strcmp(argv[2], "src") == 0) {
            mode = 2;
        } else if (strcmp(argv[2], "real") == 0) {
            mode = 3;
        } else if (strcmp(argv[2], "all") == 0) {
            mode = 4;
        } else {
            fprintf(stderr, "Erreur: mode inconnu '%s'\n", argv[2]);
            fprintf(stderr, "Modes valides: max, src, real, all\n");
            return 1;
        }
        return traiterHistogramme(argv[3], argv[4], mode);
    }
    /* Traitement fuites */
    else if (strcmp(argv[1], "leaks") == 0) {
        return traiterFuites(argv[3], argv[4], argv[2]);
    }
    else {
        fprintf(stderr, "Erreur: commande inconnue '%s'\n", argv[1]);
        fprintf(stderr, "Commandes valides: histo, leaks\n");
        return 1;
    }
}
