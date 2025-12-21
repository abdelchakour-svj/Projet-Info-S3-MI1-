#!/bin/bash


# Je commence par noter l'heure exacte du lancement et je prépare mes chemins de dossiers. 
# C'est plus pratique de tout mettre dans des variables au début pour pas se perdre après.
DEBUT=$(date +%s%3N)
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CODE_C_DIR="$SCRIPT_DIR/codeC"
GRAPHS_DIR="$SCRIPT_DIR/graphs"
TESTS_DIR="$SCRIPT_DIR/tests"
TEMP_DIR="$SCRIPT_DIR/tmp"

# Ici, je crée des petites fonctions pour m'aider. Une pour expliquer comment utiliser le script si on se trompe, 
# une pour afficher les messages d'erreur et une pour donner le temps total à la fin.
afficher_usage() {
    echo "Usage : $0 <fichier.dat> <commande> [options]"
    echo ""
    echo "Commandes disponibles :"
    echo "  histo {max|src|real|all}  - Generation d'histo usines"
    echo "  leaks \"<identifiant>\"      - Calcul des fuites"
    echo ""
    echo "Exemples d'utilisation :"
    echo "  $0 wildwater.dat histo max"
    echo "  $0 wildwater.dat histo src"
    echo "  $0 wildwater.dat leaks \"Facility complex #RH400057F\""
}

erreur() {
    echo "Erreur : $1" >&2
    afficher_duree
    exit 1
}

afficher_duree() {
    FIN=$(date +%s%3N)
    DUREE=$((FIN - DEBUT))
    echo ""
    echo "Duree totale d'execution : ${DUREE} ms"
}

# Je vérifie que l'utilisateur a bien tapé les bons arguments. 
# S'il en manque ou s'il y en a trop, je l'arrête tout de suite. Je check aussi si le fichier de données existe.
if [ "$#" -lt 2 ]; then
    afficher_usage
    erreur "Nombre d'arguments insuffisant"
fi

FICHIER_DONNEES="$1"
COMMANDE="$2"
OPTION="$3"

if [ "$#" -gt 3 ]; then
    erreur "Trop d'arguments fournis"
fi

if [ ! -f "$FICHIER_DONNEES" ]; then
    erreur "Le fichier '$FICHIER_DONNEES' est introuvable"
fi

# Je crée les dossiers nécessaires s'ils n'existent pas encore pour éviter les erreurs d'écriture.
mkdir -p "$GRAPHS_DIR" "$TESTS_DIR" "$TEMP_DIR"

# C'est l'étape de la compilation. Je vais dans le dossier du code C. 
# Si le programme n'est pas déjà compilé, je lance 'make'. C'est plus simple que de tout compiler à la main à chaque fois.
echo " Verification de la compilation "
cd "$CODE_C_DIR" || erreur "Impossible d'acceder au repertoire codeC"

if [ ! -f "wildwater" ]; then
    echo "Compilation du programme C avec make"
    make
    if [ $? -ne 0 ]; then
        erreur "La compilation a echoue"
    fi
    echo "Compilation terminee avec succes"
else
    echo "L'executable wildwater existe deja"
fi

cd "$SCRIPT_DIR" || erreur "Impossible de revenir au repertoire principal"

# Si l'utilisateur a choisi la commande "histo", je commence par filtrer les données.
# Je récupère seulement les lignes qui m'intéressent (usines et captages) pour que le programme C n'ait pas à lire des trucs inutiles.
if [ "$COMMANDE" = "histo" ]; then
    
    if [ "$#" -ne 3 ]; then
        erreur "La commande 'histo' necessite une option (max, src, real )"
    fi
    
    if [[ "$OPTION" != "max" && "$OPTION" != "src" && "$OPTION" != "real" && "$OPTION" != "all" ]]; then
        erreur "Option invalide : '$OPTION'. Options valides : max, src, real "
    fi
    
    echo ""
    echo " Generation d'histogramme : mode $OPTION "
    
    DONNEES_FILTREES="$TEMP_DIR/donnees_filtrees.csv"
    FICHIER_SORTIE="$TESTS_DIR/vol_$OPTION.dat"
    
    echo "Filtrage des donnees avec grep et awk..."
    
    # Ici, je cherche les lignes des usines et des sources dans le fichier géant de départ.
    echo "  -> Extraction des capacites maximales des usines..."
    grep -E "^-;(Plant #|Module #|Unit #|Facility complex #)" "$FICHIER_DONNEES" | \
        grep -E ";-;[0-9]+;-$" > "$TEMP_DIR/usines.csv"
    
    echo "  -> Extraction des volumes captes par les sources..."
    grep -E "^-;(Source #|Well #|Well field #|Spring #|Fountain #|Resurgence #)" "$FICHIER_DONNEES" | \
        grep -E ";(Plant #|Module #|Unit #|Facility complex #)" > "$TEMP_DIR/captages.csv"
    
    NB_USINES=$(wc -l < "$TEMP_DIR/usines.csv")
    NB_CAPTAGES=$(wc -l < "$TEMP_DIR/captages.csv")
    echo "  -> $NB_USINES usines trouvees"
    echo "  -> $NB_CAPTAGES relations de captage trouvees"
    
    cat "$TEMP_DIR/usines.csv" "$TEMP_DIR/captages.csv" > "$DONNEES_FILTREES"
    
    if [ ! -s "$DONNEES_FILTREES" ]; then
        rm -f "$DONNEES_FILTREES" "$TEMP_DIR"/*.csv
        erreur "Aucune donnee n'a pu etre extraite du fichier"
    fi
    
    # Une fois les données filtrées, j'appelle le programme C qui va faire les vrais calculs.
    echo "Appel du programme C pour le traitement..."
    "$CODE_C_DIR/wildwater" histo "$OPTION" "$DONNEES_FILTREES" "$FICHIER_SORTIE"
    
    if [ $? -ne 0 ]; then
        rm -f "$DONNEES_FILTREES" "$TEMP_DIR"/*.csv
        erreur "Le programme C a retourne une erreur"
    fi
    
    echo "Traitement des donnees termine avec succes"
    
    # Là, je prépare les fichiers pour faire les graphiques. Je trie tout ça pour isoler
    # les 50 plus petites usines et les 10 plus grandes pour que ce soit lisible sur l'image.
    echo "Preparation des donnees pour les graphiques..."
    
    FICHIER_PETITES="$TEMP_DIR/petites_$OPTION.dat"
    FICHIER_GRANDES="$TEMP_DIR/grandes_$OPTION.dat"
    
    if [ "$OPTION" = "all" ]; then
        awk -F';' 'NR>1 {total=$2+$3+$4; print $0";"total}' "$FICHIER_SORTIE" | \
            sort -t';' -k5 -n | head -50 | \
            awk -F';' '{print $1";"$2";"$3";"$4}' > "$FICHIER_PETITES"
        
        awk -F';' 'NR>1 {total=$2+$3+$4; print $0";"total}' "$FICHIER_SORTIE" | \
            sort -t';' -k5 -nr | head -10 | \
            awk -F';' '{print $1";"$2";"$3";"$4}' > "$FICHIER_GRANDES"
    else
	awk -F';' 'NR>1 && ($2+0) > 0 { gsub(/\r/,"",$0); print }' "$FICHIER_SORTIE" \
	| LC_ALL=C sort -t';' -k2,2g \
	| head -50 > "$FICHIER_PETITES"

	awk -F';' 'NR>1 && ($2+0) > 0 { gsub(/\r/,"",$0); print }' "$FICHIER_SORTIE" \
	| LC_ALL=C sort -t';' -k2,2gr \
	| head -10 > "$FICHIER_GRANDES"
    fi
    
    # Je définis les légendes des graphiques selon l'option choisie par l'utilisateur.
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
    
    # C'est la partie Gnuplot. C'est ici que les images PNG sont créées à partir des données traitées.
    echo "Generation des graphiques avec gnuplot..."
    
    if [ "$OPTION" = "all" ]; then
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
    
    if [ $? -eq 0 ]; then
        echo "Graphiques generes avec succes :"
        echo "  - $GRAPHS_DIR/vol_${OPTION}_small.png (50 plus petites usines)"
        echo "  - $GRAPHS_DIR/vol_${OPTION}_big.png (10 plus grandes usines)"
    else
        erreur "Echec lors de la generation des graphiques avec gnuplot"
    fi
    
    # Un peu de ménage : je supprime les fichiers temporaires pour pas encombrer le disque.
    rm -f "$DONNEES_FILTREES" "$FICHIER_PETITES" "$FICHIER_GRANDES" "$TEMP_DIR"/*.csv
    
    echo ""
    echo "=== Traitement termine avec succes ==="
    echo "Fichier de donnees : $FICHIER_SORTIE"

# Si l'utilisateur a choisi la commande "leaks", je calcule les fuites pour une usine précise.
# Je récupère tout ce qui arrive dans l'usine et tout ce qui en ressort (consommation + stockage).
elif [ "$COMMANDE" = "leaks" ]; then
    
    if [ "$#" -ne 3 ]; then
        erreur "La commande 'leaks' necessite un identifiant d'usine"
    fi
    
    IDENTIFIANT_USINE="$3"
    
    echo ""
    echo "=== Calcul des fuites pour l'usine : $IDENTIFIANT_USINE ==="
    
    FICHIER_SORTIE="$TESTS_DIR/leaks.dat"
    DONNEES_FILTREES="$TEMP_DIR/donnees_usine.csv"
    
    if [ ! -f "$FICHIER_SORTIE" ]; then
        echo "identifier;Leak volume (M.m3.year-1)" > "$FICHIER_SORTIE"
        echo "Creation du fichier de sortie avec en-tete"
    fi
    
    echo "Filtrage des donnees pour l'usine..."
    
    # J'extrais les captages, la distribution et les stockages liés à cette usine précise.
    echo "  -> Extraction des captages..."
    grep -F "$IDENTIFIANT_USINE" "$FICHIER_DONNEES" | \
        grep -E "^-;(Source #|Well #|Well field #|Spring #|Fountain #|Resurgence #)" > "$TEMP_DIR/captages_usine.csv"
    
    echo "  -> Extraction des troncons de distribution..."
    grep -F "$IDENTIFIANT_USINE" "$FICHIER_DONNEES" | \
        grep -v "^-;" > "$TEMP_DIR/distribution_usine.csv"
    
    echo "  -> Extraction des stockages..."
    grep -F "$IDENTIFIANT_USINE" "$FICHIER_DONNEES" | \
        grep -E "^-;.*Storage" >> "$TEMP_DIR/distribution_usine.csv"
    
    NB_CAPTAGES=$(awk 'END {print NR}' "$TEMP_DIR/captages_usine.csv" 2>/dev/null || echo "0")
    NB_DISTRIB=$(awk 'END {print NR}' "$TEMP_DIR/distribution_usine.csv" 2>/dev/null || echo "0")
    echo "  -> $NB_CAPTAGES captages trouves"
    echo "  -> $NB_DISTRIB troncons de distribution trouves"
    
    cat "$TEMP_DIR/captages_usine.csv" "$TEMP_DIR/distribution_usine.csv" > "$DONNEES_FILTREES" 2>/dev/null
    
    # J'envoie ces données filtrées au programme C pour obtenir le volume des fuites.
    echo "Appel du programme C pour le calcul des fuites..."
    "$CODE_C_DIR/wildwater" leaks "$IDENTIFIANT_USINE" "$DONNEES_FILTREES" "$FICHIER_SORTIE"
    
    if [ $? -ne 0 ]; then
        rm -f "$DONNEES_FILTREES" "$TEMP_DIR"/*.csv
        erreur "Le programme C a retourne une erreur"
    fi
    
    rm -f "$DONNEES_FILTREES" "$TEMP_DIR/captages_usine.csv" "$TEMP_DIR/distribution_usine.csv"
    
    echo ""
    echo "=== Calcul des fuites termine avec succes ==="
    echo "Resultat ajoute dans le fichier : $FICHIER_SORTIE"
    
    echo ""
    echo "Dernier resultat calcule :"
    tail -1 "$FICHIER_SORTIE"

# Si la commande n'existe pas, je renvoie une erreur.
else
    erreur "Commande inconnue : '$COMMANDE'. Commandes valides : histo, leaks"
fi

# Enfin, j'affiche combien de temps le script a mis pour tout faire.
afficher_duree

exit 0
