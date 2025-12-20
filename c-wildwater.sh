#!/bin/bash




#debut du chrono 
DEBUT=$(date +%s%3N)

#creation des differents dossier ( pour que les fichiers de sortie soient bien trie)
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CODE_C_DIR="$SCRIPT_DIR/codeC"
GRAPHS_DIR="$SCRIPT_DIR/graphs"
TESTS_DIR="$SCRIPT_DIR/tests"
TEMP_DIR="$SCRIPT_DIR/tmp"



#Message d accueil ( quand on lance le code)
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

#Messaege erreur 
erreur() {
    echo "Erreur : $1" >&2
    afficher_duree
    exit 1
}

#Fonciton que l on va appele quand ya une eerreur ou la fin du script pour donne le temps total d execution 
afficher_duree() {
    FIN=$(date +%s%3N)
    DUREE=$((FIN - DEBUT))
    echo ""
    echo "Duree totale d'execution : ${DUREE} ms"
}


#Verifier si l'utilisateur a bien donner un mode au minimum
if [ "$#" -lt 2 ]; then
    afficher_usage
    erreur "Nombre d'arguments insuffisant"
fi

FICHIER_DONNEES="$1"
COMMANDE="$2"
OPTION="$3"

# Verifier que l utilisateur a bien donenr un mode et une option
if [ "$#" -gt 3 ]; then
    erreur "Trop d'arguments fournis"
fi

#Verifier que le fichier donne  à traiter est existe bien 
if [ ! -f "$FICHIER_DONNEES" ]; then
    erreur "Le fichier '$FICHIER_DONNEES' est introuvable"
fi

#On cree les repertoires
mkdir -p "$GRAPHS_DIR" "$TESTS_DIR" "$TEMP_DIR"





echo " Verification de la compilation "

#on va dans le dossier code C
cd "$CODE_C_DIR" || erreur "Impossible d'acceder au repertoire codeC"

#Verifie si il existe pas deja un executable si oui lexecuter sinon non alors verfier si cest compilable ou non
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

#on va dans le dossier initale ( script)

cd "$SCRIPT_DIR" || erreur "Impossible de revenir au repertoire principal"



#en fonction de la la commande on gere le cas 

#CAS  HISTOGRAMME
if [ "$COMMANDE" = "histo" ]; then
    
    #Verification que l'option est bien fournie
    if [ "$#" -ne 3 ]; then
        erreur "La commande 'histo' necessite une option (max, src, real )"
    fi
    
    #Verification que l'option est valide
    if [[ "$OPTION" != "max" && "$OPTION" != "src" && "$OPTION" != "real" && "$OPTION" != "all" ]]; then
        erreur "Option invalide : '$OPTION'. Options valides : max, src, real "
    fi
    
    echo ""
    echo " Generation d'histogramme : mode $OPTION "
    
    #Definition des noms de fichiers
    DONNEES_FILTREES="$TEMP_DIR/donnees_filtrees.csv"
    FICHIER_SORTIE="$TESTS_DIR/vol_$OPTION.dat"
    
  
    #Filtrage des données (grep+awk )
  
    
    echo "Filtrage des donnees avec grep et awk..."
    
    #Extraction des lignes d'usines (description de capacite maximale)
    
    echo "  -> Extraction des capacites maximales des usines..."
    grep -E "^-;(Plant #|Module #|Unit #|Facility complex #)" "$FICHIER_DONNEES" | \
        grep -E ";-;[0-9]+;-$" > "$TEMP_DIR/usines.csv"
    
    #Extraction des lignes qui correspondetn à source vers usine
    
    echo "  -> Extraction des volumes captes par les sources..."
    grep -E "^-;(Source #|Well #|Well field #|Spring #|Fountain #|Resurgence #)" "$FICHIER_DONNEES" | \
        grep -E ";(Plant #|Module #|Unit #|Facility complex #)" > "$TEMP_DIR/captages.csv"
    
    #Compter le nombre de lignes extraites avec le grp et awk
    NB_USINES=$(wc -l < "$TEMP_DIR/usines.csv")
    NB_CAPTAGES=$(wc -l < "$TEMP_DIR/captages.csv")
    echo "  -> $NB_USINES usines trouvees"
    echo "  -> $NB_CAPTAGES relations de captage trouvees"
    
    #Fusisioner les deux fihcier pour envoyer au C
    cat "$TEMP_DIR/usines.csv" "$TEMP_DIR/captages.csv" > "$DONNEES_FILTREES"
    
    #Verifier que le fichier qu on vient de fusionner est bien rempli sinon le supp
    if [ ! -s "$DONNEES_FILTREES" ]; then
        rm -f "$DONNEES_FILTREES" "$TEMP_DIR"/*.csv
        erreur "Aucune donnee n'a pu etre extraite du fichier"
    fi
    

    #Appel au C
    echo "Appel du programme C pour le traitement..."
    "$CODE_C_DIR/wildwater" histo "$OPTION" "$DONNEES_FILTREES" "$FICHIER_SORTIE"
	
    
    #On verifie que le code c retourne bien qq chose 
    if [ $? -ne 0 ]; then
        rm -f "$DONNEES_FILTREES" "$TEMP_DIR"/*.csv
        erreur "Le programme C a retourne une erreur"
    fi
    
    echo "Traitement des donnees termine avec succes"
    
   
    #Preparation des données pour faire le gnuplot
 
    echo "Preparation des donnees pour les graphiques..."
    
    FICHIER_PETITES="$TEMP_DIR/petites_$OPTION.dat"
    FICHIER_GRANDES="$TEMP_DIR/grandes_$OPTION.dat"


	
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
    
   
    # Definition des parametres pour gnuplot selon le mode
   
    
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
    
   
    #Generation des graphiques avec gnuplot
   
    
    echo "Generation des graphiques avec gnuplot..."
    
    if [ "$OPTION" = "all" ]; then
        # raphiques empiles pour le mode all
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
        #Graphiques simples pour les modes comme max, src, real
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
    
    
	#On verifie que les graphiques ont bien ete genere 

    if [ $? -eq 0 ]; then
        echo "Graphiques generes avec succes :"
        echo "  - $GRAPHS_DIR/vol_${OPTION}_small.png (50 plus petites usines)"
        echo "  - $GRAPHS_DIR/vol_${OPTION}_big.png (10 plus grandes usines)"
    else
        erreur "Echec lors de la generation des graphiques avec gnuplot"
    fi
    
    #On suppirme les fichier temp
    rm -f "$DONNEES_FILTREES" "$FICHIER_PETITES" "$FICHIER_GRANDES" "$TEMP_DIR"/*.csv
    
    echo ""
    echo "=== Traitement termine avec succes ==="
    echo "Fichier de donnees : $FICHIER_SORTIE"


#TRAITEMENT CALCUL DES FUITES


elif [ "$COMMANDE" = "leaks" ]; then
    
    #Verification que dans l appelle du terminal il y a bien l identifiant usine
    if [ "$#" -ne 3 ]; then
        erreur "La commande 'leaks' necessite un identifiant d'usine"
    fi

	#On recupere l'identifiant de lusine 
    IDENTIFIANT_USINE="$3"
    
    echo ""
    echo "=== Calcul des fuites pour l'usine : $IDENTIFIANT_USINE ==="
    
    #Definition des noms de fichiers
    FICHIER_SORTIE="$TESTS_DIR/leaks.dat"
    DONNEES_FILTREES="$TEMP_DIR/donnees_usine.csv"
    
    #Creer le fichier s'il n'existe pas encore et on met bien  "identifier;Leak volume (M.m3.year-1)" au debut du fichier 
    if [ ! -f "$FICHIER_SORTIE" ]; then
        echo "identifier;Leak volume (M.m3.year-1)" > "$FICHIER_SORTIE"
        echo "Creation du fichier de sortie avec en-tete"
    fi
    #Filtrage des donnees pour cette usine specifique
    
    echo "Filtrage des donnees pour l'usine..."
    
    #Extraire les captages vers cette usine (source vers usine)
    echo "  -> Extraction des captages..."
    grep -F "$IDENTIFIANT_USINE" "$FICHIER_DONNEES" | \
        grep -E "^-;(Source #|Well #|Well field #|Spring #|Fountain #|Resurgence #)" > "$TEMP_DIR/captages_usine.csv"
    
    #Extraire tous les troncons de distribution de cette usine
    #Ce sont les lignes ou la colonne 1 contient l'identifiant de l'usine
    echo "  -> Extraction des troncons de distribution..."
    grep -F "$IDENTIFIANT_USINE" "$FICHIER_DONNEES" | \
        grep -v "^-;" > "$TEMP_DIR/distribution_usine.csv"
    
    # Extraire aussi les troncons usine vers stockage
    # Format: -;Usine;Storage;-;pourcentage
    echo "  -> Extraction des stockages..."
    grep -F "$IDENTIFIANT_USINE" "$FICHIER_DONNEES" | \
        grep -E "^-;.*Storage" >> "$TEMP_DIR/distribution_usine.csv"
    
    #Compter les lignes avec awk ditrib et captages
    NB_CAPTAGES=$(awk 'END {print NR}' "$TEMP_DIR/captages_usine.csv" 2>/dev/null || echo "0")
    NB_DISTRIB=$(awk 'END {print NR}' "$TEMP_DIR/distribution_usine.csv" 2>/dev/null || echo "0")
    echo "  -> $NB_CAPTAGES captages trouves"
    echo "  -> $NB_DISTRIB troncons de distribution trouves"
    
    #On fusionne les deux fichier pour les envoyer au c 
    cat "$TEMP_DIR/captages_usine.csv" "$TEMP_DIR/distribution_usine.csv" > "$DONNEES_FILTREES" 2>/dev/null
    
    #
    # Appel du programme C
    #
    
    echo "Appel du programme C pour le calcul des fuites..."
    "$CODE_C_DIR/wildwater" leaks "$IDENTIFIANT_USINE" "$DONNEES_FILTREES" "$FICHIER_SORTIE"
    
    #On verifie
    if [ $? -ne 0 ]; then
        rm -f "$DONNEES_FILTREES" "$TEMP_DIR"/*.csv
        erreur "Le programme C a retourne une erreur"
    fi
    
    #On supp tout les fichiers temp
    rm -f "$DONNEES_FILTREES" "$TEMP_DIR/captages_usine.csv" "$TEMP_DIR/distribution_usine.csv"
    
    echo ""
    echo "=== Calcul des fuites termine avec succes ==="
    echo "Resultat ajoute dans le fichier : $FICHIER_SORTIE"
    
    # Afficher le dernier resultat calcule
    echo ""
    echo "Dernier resultat calcule :"
    tail -1 "$FICHIER_SORTIE"


#Si Commande inconnue


else
    erreur "Commande inconnue : '$COMMANDE'. Commandes valides : histo, leaks"
fi


# Affichage de la duree totale d'execution


afficher_duree

exit 0
