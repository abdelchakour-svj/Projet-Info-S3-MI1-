#!/bin/bash

#On enregistre l'heure de depart pour savoir combien de temps le script tourne
DEBUT=$(date +%s%3N)


#On definit où sont ranges les dossiers pour ne pas se perdre dans les chemins
#SCRIPT_DIR est le dossier racine où se trouve ce fichier
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CODE_C_DIR="$SCRIPT_DIR/codeC"
GRAPHS_DIR="$SCRIPT_DIR/graphs"
TESTS_DIR="$SCRIPT_DIR/tests"
TEMP_DIR="$SCRIPT_DIR/tmp"


#Petit calcul pour afficher le temps ecoule depuis le debut en millisecondes
afficher_duree() {
    FIN=$(date +%s%3N)
    DUREE=$((FIN - DEBUT))
    echo ""
    echo "Duree totale d'execution : ${DUREE} ms"
}


#Fonction pour stopper le script proprement si un truc se passe mal
#Elle affiche pourquoi ça a plante et donne le temps total
erreur() {
    echo "Erreur : $1" >&2
    afficher_duree
    exit 1
}


#On verifie que l'utilisateur a tape les bonnes infos au lancement
#Il faut au moins le fichier de donnees et une commande (histo ou leaks)
if [ "$#" -lt 2 ]; then
    echo "Usage : $0 <fichier.dat> <commande> [options]"
    erreur "Nombre d'arguments insuffisant"
fi

FICHIER_DONNEES="$1"
COMMANDE="$2"
OPTION="$3"

if [ ! -f "$FICHIER_DONNEES" ]; then
    erreur "Le fichier '$FICHIER_DONNEES' est introuvable"
fi


#Preparation de l'espace de travail
#On cree les dossiers s'ils n'existent pas encore pour ranger les resultats
mkdir -p "$GRAPHS_DIR" "$TESTS_DIR" "$TEMP_DIR"


#On s'assure que le programme C est bien compile
#Si l'executable n'est pas la, on lance la compilation avec make
cd "$CODE_C_DIR" || erreur "Impossible d'acceder au repertoire codeC"
if [ ! -f "wildwater" ]; then
    echo "Compilation du programme C..."
    make || erreur "La compilation a echoue"
fi
cd "$SCRIPT_DIR" || erreur "Retour au dossier principal impossible"


#PARTIE HISTO : On traite les donnees pour faire des graphiques
#On va d'abord filtrer le gros fichier pour ne garder que ce qui nous interesse
if [ "$COMMANDE" = "histo" ]; then
    [ "$#" -ne 3 ] && erreur "Il manque l'option (max, src, real ou all)"
    
    DONNEES_FILTREES="$TEMP_DIR/donnees_filtrees.csv"
    FICHIER_SORTIE="$TESTS_DIR/vol_$OPTION.dat"

    #On utilise grep pour chercher les lignes des usines et des sources
    #L'idee est de nettoyer le fichier avant de le donner au programme C
    grep -E "^-;(Plant #|Module #|Unit #|Facility complex #)" "$FICHIER_DONNEES" | \
        grep -E ";-;[0-9]+;-$" > "$TEMP_DIR/usines.csv"
    
    grep -E "^-;(Source #|Well #|Well field #|Spring #|Fountain #|Resurgence #)" "$FICHIER_DONNEES" | \
        grep -E ";(Plant #|Module #|Unit #|Facility complex #)" > "$TEMP_DIR/captages.csv"
    
    cat "$TEMP_DIR/usines.csv" "$TEMP_DIR/captages.csv" > "$DONNEES_FILTREES"
    [ ! -s "$DONNEES_FILTREES" ] && erreur "Le filtrage n'a rien donne"

    #Maintenant on lance le programme C qui va faire les gros calculs de volumes
    "$CODE_C_DIR/wildwater" histo "$OPTION" "$DONNEES_FILTREES" "$FICHIER_SORTIE" || erreur "Probleme dans le programme C"
    
    #On trie les resultats pour n'afficher que le top du top sur les graphiques
    #On prepare un fichier pour les 50 plus petites et un pour les 10 plus grandes
    FICHIER_PETITES="$TEMP_DIR/petites_$OPTION.dat"
    FICHIER_GRANDES="$TEMP_DIR/grandes_$OPTION.dat"

    awk -F';' 'NR>1 && ($2+0) > 0 { gsub(/\r/,"",$0); print }' "$FICHIER_SORTIE" \
    | LC_ALL=C sort -t';' -k2,2g | head -50 > "$FICHIER_PETITES"

    awk -F';' 'NR>1 && ($2+0) > 0 { gsub(/\r/,"",$0); print }' "$FICHIER_SORTIE" \
    | LC_ALL=C sort -t';' -k2,2gr | head -10 > "$FICHIER_GRANDES"
    
    #C'est ici qu'on dessine les graphiques en PNG avec Gnuplot
    #On configure les axes, les couleurs et on enregistre les images
    gnuplot <<EOF
set terminal png size 1400,900
set datafile separator ";"
set style data histograms
set style fill solid border -1
set xtics rotate by -45 font ",8"
set grid y
set output "$GRAPHS_DIR/vol_${OPTION}_small.png"
plot "$FICHIER_PETITES" using 2:xtic(1) notitle with histograms lc rgb "blue"
set output "$GRAPHS_DIR/vol_${OPTION}_big.png"
plot "$FICHIER_GRANDES" using 2:xtic(1) notitle with histograms lc rgb "red"
EOF
    
    #On fait le ménage dans le dossier temporaire apres avoir fini
    rm -f "$DONNEES_FILTREES" "$FICHIER_PETITES" "$FICHIER_GRANDES" "$TEMP_DIR"/*.csv


#PARTIE LEAKS : On cherche les fuites pour une usine precise
#On regarde ce qui entre et ce qui sort pour voir la difference
elif [ "$COMMANDE" = "leaks" ]; then
    [ "$#" -ne 3 ] && erreur "Tu dois donner l'ID de l'usine"

    IDENTIFIANT_USINE="$3"
    FICHIER_SORTIE="$TESTS_DIR/leaks.dat"
    DONNEES_FILTREES="$TEMP_DIR/donnees_usine.csv"
    
    #On cree le fichier de sortie avec une ligne de titre si besoin
    [ ! -f "$FICHIER_SORTIE" ] && echo "identifier;Leak volume (M.m3.year-1)" > "$FICHIER_SORTIE"

    #On extrait uniquement ce qui concerne l'usine choisie
    #D'un cote les sources (entree) et de l'autre la distribution (sortie)
    grep -F "$IDENTIFIANT_USINE" "$FICHIER_DONNEES" | \
        grep -E "^-;(Source #|Well #|Well field #|Spring #|Fountain #|Resurgence #)" > "$TEMP_DIR/captages_usine.csv"
    
    grep -F "$IDENTIFIANT_USINE" "$FICHIER_DONNEES" | grep -v "^-;" > "$TEMP_DIR/distribution_usine.csv"
    
    cat "$TEMP_DIR/captages_usine.csv" "$TEMP_DIR/distribution_usine.csv" > "$DONNEES_FILTREES"
    
    #Le programme C fait la soustraction pour trouver le volume de fuite
    "$CODE_C_DIR/wildwater" leaks "$IDENTIFIANT_USINE" "$DONNEES_FILTREES" "$FICHIER_SORTIE" || erreur "Calcul des fuites echoue"
    
    rm -f "$DONNEES_FILTREES" "$TEMP_DIR"/*.csv
    tail -1 "$FICHIER_SORTIE"


else
    erreur "Cette commande n'existe pas"
fi


#Tout s'est bien passe, on affiche le temps total et on quitte
afficher_duree
exit 0
