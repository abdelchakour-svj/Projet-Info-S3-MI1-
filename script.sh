#!/bin/bash


START_TIME=$(date +%s%3N)


if [ "$#" -lt 2 ]; then
    echo "Erreur : Nombre d'arguments insuffisant"
    echo "Usage : $0 <fichier.dat> <action> [options]"
    echo "Actions disponibles :"
    echo "  histo {max|src|real}     - Génère des histogrammes"
    echo "  leaks \"<identifiant>\"  - Calcule les fuites d'une usine"
    exit 1
fi

DATA_FILE="$1"
ACTION="$2"
OPTION="$3"


if [ ! -f "$DATA_FILE" ]; then
    echo "Erreur : fichier $DATA_FILE introuvable"
    exit 1
fi

if [ ! -f "./wildwater" ]; then
    echo "Compilation du programme C..."
    make
    if [ $? -ne 0 ]; then
        echo "Erreur : échec de la compilation"
        exit 1
    fi
fi




if [ "$ACTION" = "histo" ]; then
    if [ "$#" -ne 3 ]; then
    echo "Erreur : l'action 'histo' nécessite une option"
    echo "Options disponibles : {max, src, real}"
    exit 1
fi

if [[ "$OPTION" != "max" && "$OPTION" != "src" && "$OPTION" != "real" ]]; then
    echo "Erreur : option invalide ($OPTION)"
    exit 1
fi

#Filtrage des donnée 

TMP_FILE="filtered_$OPTION.tmp"

if [ "$OPTION" = "src" ]; then
    # volume total capté par les sources
    grep "^-;Spring #[^;]*;Facility complex #[^;]*;[^;]*;[^;]*$" "$DATA_FILE" | awk -F';' '{print $3 ";" $4}' > "$TMP_FILE"
    

elif [ "$OPTION" = "real" ]; then
    #MAUVAIS GREP A REVOIR
    grep "^-;Spring #[^;]*;Facility complex #[^;]*;[^;]*;[^;]*$" "$DATA_FILE | awk -F';' '{print $3 ";" $4 ";" $5}' > "$TMP_FILE"
    

elif [ "$OPTION" = "max" ]; then
    # ;-;identifiant;-;capacite max;-;
     grep "^-;Facility complexe #[^;]*;-;[^;]*;-" "$DATA_FILE" | awk -F';' '{print $2 ";" $4}' > "$TMP_FILE" 
    
fi

if [ ! -s "$TMP_FILE" ]; then
        echo "Erreur : aucune donnée filtrée pour l'option $OPTION"
        rm -f "$TMP_FILE"
        exit 1
    fi
OUTPUT_FILE="vol_$OPTION.dat"
./wildwater "$OPTION" "$TMP_FILE" "$OUTPUT_File"

if [ "$?" -ne 0 ]; then
    echo "Erreur lors de l'exécution du programme C"
    exit 1
fi

sort -t';' -k1 -r "$OUTPUT_FILE" > sorted_temp.dat
# 50 plus petites valeurs
sort -t';' -k2 -n sorted_temp.dat | head -n 50 > small_$OPTION.dat

# 10 plus grandes valeurs
sort -t';' -k2 -nr sorted_temp.dat | head -n 10 > big_$OPTION.dat

if [ "$OPTION" = "max" ]; then
        YLABEL="Volume (k.m³.year⁻¹)"
        TITLE_SUFFIX="Capacité maximale"
    elif [ "$OPTION" = "src" ]; then
        YLABEL="Volume (k.m³.year⁻¹)"
        TITLE_SUFFIX="Volume capté"
    else
        YLABEL="Volume (k.m³.year⁻¹)"
        TITLE_SUFFIX="Volume traité"
    fi


gnuplot <<EOF
set terminal png size 1400,900
set datafile separator ";"
set style data histograms
set style fill solid border -1
set boxwidth 0.9
set xtics rotate by -45 font ",8"
set grid y

set output "vol_${OPTION}_small.png"
set title "50 plus petites usines - $TITLE_SUFFIX"
set ylabel "$YLABEL"
set xlabel "Identifiant usine"
plot "small_$OPTION.dat" using 2:xtic(1) notitle with histograms lc rgb "blue"

set output "vol_${OPTION}_big.png"
set title "10 plus grandes usines - $TITLE_SUFFIX"
set ylabel "$YLABEL"
set xlabel "Identifiant usine"
plot "big_$OPTION.dat" using 2:xtic(1) notitle with histograms lc rgb "red"
EOF

# A FAIRE URGENT
if [ $? -eq 0 ]; then
        echo "Histogrammes générés avec succès : "
        echo "  - vol_${OPTION}_small.png (50 plus petites usines)"
        echo "  - vol_${OPTION}_big.png (10 plus grandes usines)"
    else
        echo "Erreur lors de la génération des graphiques"
        exit 1
    fi


elif [ "$ACTION" = "leaks" ]; then
    # Vérification du 3ème argument (identifiant de l'usine)
    if [ "$#" -ne 3 ]; then
        echo "Erreur : l'action 'leaks' nécessite un identifiant d'usine"
        echo "Usage : $0 <fichier.dat> leaks \"<identifiant usine>\""
        exit 1
    fi
    
FACILITY_ID="$3"
OUTPUT_FILE="leaks.dat"

 ./wildwater "leaks" "$DATA_FILE" "$FACILITY_ID" "$OUTPUT_FILE"
    
if [ $? -ne 0 ]; then
    echo "Erreur lors de l'exécution du programme C"
    exit 1
fi
    
echo "Calcul des fuites terminé. Résultat dans $OUTPUT_FILE"

else
    echo "Erreur : action invalide ($ACTION)"
    echo "Actions supportées : histo ou leaks"
    exit 1
fi
#RM FICHIER TEMP INUTILE 
rm -f "$TMP_FILE" sorted_temp.dat "small_$OPTION.dat" "big_$OPTION.dat"



END_TIME=$(date +%s%3N)
DURATION=$((END_TIME - START_TIME))
echo "Durée totale d'exécution : ${DURATION} ms"

exit 0
