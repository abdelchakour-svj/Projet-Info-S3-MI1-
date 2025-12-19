#!/bin/bash



# Enregistrer le temps
DEBUT=$(date +%s%3N)

# Repertoire du script
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CODE_C_DIR="$SCRIPT_DIR/codeC"
GRAPHS_DIR="$SCRIPT_DIR/graphs"
TESTS_DIR="$SCRIPT_DIR/tests"
TEMP_DIR="$SCRIPT_DIR/tmp"

# Fonction pour afficher l'usage
afficher_usage() {
    echo "Usage : $0 <fichier.dat> <commande> [options]"
    echo ""
    echo "Commandes disponibles :"
    echo "  histo {max|src|real|all}  - Generation d'histo usines"
    echo "  leaks \"<identifiant>\"     - Calcul des fuites"
    echo ""
    echo "Exemples d'utilisation :"
    echo "  $0 wildwater.dat histo max"
    echo "  $0 wildwater.dat histo src"
    echo "  $0 wildwater.dat leaks \"Facility complex #RH400057F\""
}

# Fonction pour afficher une erreur et quitter
erreur() {
    echo "ERREUR: $1" >&2
    afficher_duree
    exit 1
}

# Fonction pour afficher la duree d'execution
afficher_duree() {
    FIN=$(date +%s%3N)
    DUREE=$((FIN - DEBUT))
    echo ""
    echo "=== Duree d'execution: ${DUREE} ms ==="
}

# Verifier le nombre d'arguments
if [ $# -lt 2]; then
    afficher_usage
    erreur "Nombre d'arguments insuffisant"
fi

# Recuperer les arguments
FICHIER_DONNEES="$1"
COMMANDE="$2"
OPTION="$3"

# Verifier s'il y a des arguments supplementaires inattendus
if [ $# -gt 3 ]; then
    erreur "Trop d'arguments fournis"
fi

# Verifier que le fichier de donnees existe
if [ ! -f "$FICHIER_DONNEES" ]; then
    erreur "Le fichier '$FICHIER_DONNEES' n'existe pas"
fi

# Creer les repertoires si necessaire
mkdir -p "$GRAPHS_DIR"
mkdir -p "$TESTS_DIR"
mkdir -p "$TEMP_DIR"


# Compilation du programme C

echo "=== Verification de la compilation ==="

cd "$CODE_C_DIR" || erreur "Impossible d'acceder au repertoire codeC"

# Verifier si l'executable existe, sinon compiler
if [ ! -f "wildwater" ]; then
    echo "Compilation du programme C..."
    make
    if [ $? -ne 0 ]; then
        erreur "La compilation a echoue"
    fi
    echo "Compilation reussie"
else
    echo "L'executable existe deja"
fi

cd "$SCRIPT_DIR" || erreur "Impossible de revenir au repertoire principal"

# Traitement selon la commande


f [ "$COMMANDE" = "histo" ]; then
    
    # Verification que l'option est bien fournie
    if [ "$#" -ne 3 ]; then
        erreur "La commande 'histo' necessite une option (max, src, real )"
    fi
    
    # Verification que l'option est valide
    if [[ "$OPTION" != "max" && "$OPTION" != "src" && "$OPTION" != "real" && "$OPTION" != "all" ]]; then
        erreur "Option invalide : '$OPTION'. Options valides : max, src, real "
    fi
    
    echo ""
    echo " Generation d'histogramme : mode $OPTION "
    
        
        # Definir les noms de fichiers
        DONNEES_FILTREES="$TEMP_DIR/donnees_filtrees.csv"
        FICHIER_SORTIE="$TESTS_DIR/vol_$OPTION.dat"
        
        echo "Fichier de sortie: $FICHIER_SORTIE"
        
        # Filtrage des donnees avec grep et awk
        echo "Filtrage des donnees avec grep/awk..."
        
        # Extraire les lignes des usines (description de capacite)
        # Format: -;Usine;-;capacite;-
        # On utilise grep pour filtrer les lignes qui contiennent Plant, Module, Unit ou Facility
        echo "  -> Extraction des usines..."
        grep -E "^-;(Plant #|Module #|Unit #|Facility complex #)" "$FICHIER_DONNEES" | \
            grep -E ";-;[0-9]+;-$" > "$TEMP_DIR/usines.csv"
        
        # Extraire les lignes de captage (source -> usine)
        # Format: -;Source;Usine;volume;pourcentage
        # Les sources sont: Source, Well, Spring, Fountain, Resurgence
        echo "  -> Extraction des volumes captes par les sources..."
        grep -E "^-;(Source #|Well #|Well field #|Spring #|Fountain #|Resurgence #)" "$FICHIER_DONNEES" | \
        grep -E ";(Plant #|Module #|Unit #|Facility complex #)" > "$TEMP_DIR/captages.csv"
        
        # Compter les lignes extraites
        NB_USINES=$(wc -l < "$TEMP_DIR/usines.csv")
        NB_CAPTAGES=$(wc -l < "$TEMP_DIR/captages.csv")
        echo "  -> $NB_USINES usines trouvees"
        echo "  -> $NB_CAPTAGES captages trouves"
        
        # Combiner les fichiers pour le programme C
        cat "$TEMP_DIR/usines.csv" "$TEMP_DIR/captages.csv" > "$DONNEES_FILTREES"

         # Verification que des donnees ont bien ete extraites
    if [ ! -s "$DONNEES_FILTREES" ]; then
        rm -f "$DONNEES_FILTREES" "$TEMP_DIR"/*.csv
        erreur "Aucune donnee n'a pu etre extraite du fichier"
    fi

    
        
        # Appeler le programme C avec les donnees filtrees
        echo "Execution du programme C..."
        ""$CODE_C_DIR/wildwater" histo "$OPTION" "$DONNEES_FILTREES" "$FICHIER_SORTIE"
        
        if [ $? -ne 0 ]; then
            rm -f "$DONNEES_FILTREES" "$TEMP_DIR"/*.csv"
            erreur "Le programme C a retourne une erreur"
        fi

        echo "Traitement des donnees termine avec succes"
        
        # Verifier que le fichier de sortie a ete cree
        if [ ! -f "$FICHIER_SORTIE" ]; then
            erreur "Le fichier de sortie n'a pas ete cree"
        fi
        
       
        
          echo "Preparation des donnees pour les graphiques..."
    
        FICHIER_PETITES="$TEMP_DIR/petites_$OPTION.dat"
        FICHIER_GRANDES="$TEMP_DIR/grandes_$OPTION.dat""
        
         if [ "$OPTION" = "all" ]; then
        # Mode bonus : trier par le total des 3 colonnes
        awk -F';' 'NR>1 {total=$2+$3+$4; print $0";"total}' "$FICHIER_SORTIE" | \
            sort -t';' -k5 -n | head -50 | \
            awk -F';' '{print $1";"$2";"$3";"$4}' > "$FICHIER_PETITES"
        
        awk -F';' 'NR>1 {total=$2+$3+$4; print $0";"total}' "$FICHIER_SORTIE" | \
            sort -t';' -k5 -nr | head -10 | \
            awk -F';' '{print $1";"$2";"$3";"$4}' > "$FICHIER_GRANDES"
    else
        
		# Enlever CR éventuels + ignorer en-tête + ignorer les zéros
	awk -F';' 'NR>1 && ($2+0) > 0 { gsub(/\r/,"",$0); print }' "$FICHIER_SORTIE" \
	| LC_ALL=C sort -t';' -k2,2g \
	| head -50 > "$FICHIER_PETITES"

	awk -F';' 'NR>1 && ($2+0) > 0 { gsub(/\r/,"",$0); print }' "$FICHIER_SORTIE" \
	| LC_ALL=C sort -t';' -k2,2gr \
	| head -10 > "$FICHIER_GRANDES"

    fi
        
        # Definir le titre selon le mode
        case "$OPTION" in
        max)
            YLABEL="Volume (M.m3.year-1)"
            TITRE="Capacite maximale de traitement"
            ;;
        src)
            YLABEL="Volume (M.m3.year-1)"
            TITRE="Volume capte par les sources"
            ;;
        real)
            YLABEL="Volume (M.m3.year-1)"
            TITRE="Volume reellement traite"
            ;;
        all)
            YLABEL="Volume (M.m3.year-1)"
            TITRE="Donnees combinees (reel, perdu, disponible)"
            ;;
    esac
    echo "Generation des graphiques avec gnuplot..."
    
    if [ "$OPTION" = "all" ]; then
        # Graphiques empiles pour le mode bonus
        gnuplot <<EOF
set terminal png size 1400,900
set datafile separator ";"
set style data histogram
set style histogram rowstacked
set style fill solid border -1
set boxwidth 0.8
set xtics rotate by -45 font ",8"
set grid y
set key outside right top

set output "$GRAPHS_DIR/vol_${OPTION}_small.png"
set title "50 plus petites usines - $TITRE"
set ylabel "$YLABEL"
set xlabel "Identifiant de l'usine"
plot "$FICHIER_PETITES" using 2:xtic(1) title "Volume reel" lc rgb "#6699FF", \
     '' using 3 title "Volume perdu" lc rgb "#FF6666", \
     '' using 4 title "Capacite disponible" lc rgb "#99FF99"

set output "$GRAPHS_DIR/vol_${OPTION}_big.png"
set title "10 plus grandes usines - $TITRE"
set ylabel "$YLABEL"
set xlabel "Identifiant de l'usine"
plot "$FICHIER_GRANDES" using 2:xtic(1) title "Volume reel" lc rgb "#6699FF", \
     '' using 3 title "Volume perdu" lc rgb "#FF6666", \
     '' using 4 title "Capacite disponible" lc rgb "#99FF99"
EOF
    else
        # Graphiques simples pour les modes max, src, real
        gnuplot <<EOF
set terminal png size 1400,900
set datafile separator ";"
set style data histograms
set style fill solid border -1
set boxwidth 0.9
set xtics rotate by -45 font ",8"
set grid y

set output "$GRAPHS_DIR/vol_${OPTION}_small.png"
set title "50 plus petites usines - $TITRE"
set ylabel "$YLABEL"
set xlabel "Identifiant de l'usine"
plot "$FICHIER_PETITES" using 2:xtic(1) notitle with histograms lc rgb "blue"

set output "$GRAPHS_DIR/vol_${OPTION}_big.png"
set title "10 plus grandes usines - $TITRE"
set ylabel "$YLABEL"
set xlabel "Identifiant de l'usine"
plot "$FICHIER_GRANDES" using 2:xtic(1) notitle with histograms lc rgb "red"
EOF
    fi


# a adaapter le truc du fin la parce pas kr utiliser version finale ultime 




























        
        
        
        
        if [ $? -eq 0 ]; then
        echo "Graphiques generes avec succes :"
        echo "  - $GRAPHS_DIR/vol_${OPTION}_small.png (50 plus petites usines)"
        echo "  - $GRAPHS_DIR/vol_${OPTION}_big.png (10 plus grandes usines)"
    else
        erreur "Echec lors de la generation des graphiques avec gnuplot"
    fi
        
        # Nettoyer les fichiers temporaires
        rm -f "$DONNEES_FILTREES" "$FICHIER_PETITES" "$FICHIER_GRANDES" "$TEMP_DIR"/*.csv
    
    echo ""
    echo "=== Traitement termine avec succes ==="
    echo "Fichier de donnees : $FICHIER_SORTIE"
    
    # Commande LEAKS - Calcul des fuites
        elif [ "$COMMANDE" = "leaks" ]; then
        
        ID_USINE="$OPTION"
        echo "Usine: $ID_USINE"
        
        # Definir les fichiers
        FICHIER_SORTIE="$TESTS_DIR/leaks.dat"
        FICHIER_FILTRE="$TEMP_DIR/donnees_usine.csv"
        
        # Filtrage des donnees avec grep pour cette usine specifique
        echo "Filtrage des donnees pour l'usine $ID_USINE..."
        
        # Extraire les captages vers cette usine (source -> usine)
        # Format: -;Source;Usine;volume;pourcentage
        echo "  -> Extraction des captages..."
        grep -F "$ID_USINE" "$FICHIER_DONNEES" | \
            grep -E "^-;(Source|Well|Spring|Fountain|Resurgence)" > "$TEMP_DIR/captages_usine.csv"
        
        # Extraire tous les troncons de distribution de cette usine
        # Ce sont les lignes ou la colonne 1 contient l'identifiant de l'usine
        echo "  -> Extraction des troncons de distribution..."
        grep -F "$ID_USINE" "$FICHIER_DONNEES" | \
            grep -v "^-;" > "$TEMP_DIR/distribution_usine.csv"
        
        # Extraire aussi les troncons usine -> stockage
        # Format: -;Usine;Storage;-;pourcentage
        echo "  -> Extraction des stockages..."
        grep -F "$ID_USINE" "$FICHIER_DONNEES" | \
            grep -E "^-;.*Storage" >> "$TEMP_DIR/distribution_usine.csv"
        
        # Compter les lignes avec awk
        NB_CAPTAGES=$(awk 'END {print NR}' "$TEMP_DIR/captages_usine.csv")
        NB_DISTRIB=$(awk 'END {print NR}' "$TEMP_DIR/distribution_usine.csv")
        echo "  -> $NB_CAPTAGES captages trouves"
        echo "  -> $NB_DISTRIB troncons de distribution trouves"
        
        # Combiner les fichiers
        cat "$TEMP_DIR/captages_usine.csv" "$TEMP_DIR/distribution_usine.csv" > "$FICHIER_FILTRE"
        
        # Creer l'en-tete du fichier de sortie si necessaire
        if [ ! -f "$FICHIER_SORTIE" ]; then
            echo "identifier;Leak volume (M.m3.year-1)" > "$FICHIER_SORTIE"
        fi
        
        # Appeler le programme C
        echo "Execution du programme C..."
        "$CODE_C_DIR/wildwater" leaks "$ID_USINE" "$FICHIER_FILTRE" "$FICHIER_SORTIE"
        RETOUR=$?
        
        if [ $RETOUR -ne 0 ]; then
            erreur "Le programme C a retourne une erreur (code: $RETOUR)"
        fi
        
        # Nettoyer les fichiers temporaires
        rm -f "$TEMP_DIR/captages_usine.csv" "$TEMP_DIR/distribution_usine.csv" "$FICHIER_FILTRE"
        
        echo ""
        echo "=== Traitement termine avec succes ==="
        echo "Resultat ajoute dans: $FICHIER_SORTIE"
        
        # Afficher le dernier resultat avec awk
        echo ""
        echo "Dernier resultat:"
        awk 'END {print}' "$FICHIER_SORTIE"
        ;;
    
    # Commande inconnue
    *)
        erreur "Commande inconnue: $COMMANDE (valides: histo, leaks)"
        ;;
esac


else
    erreur "Commande inconnue : '$COMMANDE'. Commandes valides : histo, leaks"
fi

afficher_duree

exit 0
