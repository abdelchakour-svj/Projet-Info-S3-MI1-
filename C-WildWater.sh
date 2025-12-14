#!/bin/bash


START_TIME=$(date +%s%3N)


if [ "$#" -lt 2 ]; then
    echo "Erreur : Nb argument insufisant "
    echo "expliquer cm faire a revoir "
    exit 1
fi

DATA_FILE="$1"
ACTION="$2"
OPTION="$3"


if [ ! -f "$DATA_FILE" ]; then
    echo "Erreur : fichier $DATA_FILE introuvable"
    exit 1
fi

if [ "$ACTION" != "histo" ]; then
    echo "Erreur : seule l'action 'histo' est supportée ici"
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

#apelle du c

./wildwater "$OPTION" "$TMP_FILE" result.dat

if [ "$?" -ne 0 ]; then
    echo "Erreur lors de l'exécution du programme C"
    exit 1
fi

#gnuplot graph chatgpt a revoir


# 50 plus petites valeurs
sort -t';' -k2 -n result.dat | head -n 50 > small.dat

# 10 plus grandes valeurs
sort -t';' -k2 -nr result.dat | head -n 10 > big.dat

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




END_TIME=$(date +%s%3N)
DURATION=$((END_TIME - START_TIME))
echo "Durée totale d'exécution : ${DURATION} ms"

exit 0
