#!/bin/bash
WORDS=("truth" "algorithm" "chaos" "matrix" "life" "network" "planet" "vision")
NUM_QUERIES=10
CHAVE_MIN=1000000000
CHAVE_MAX=9999999999

for ((i=0; i<NUM_QUERIES; i++)); do
    chave=$(shuf -i ${CHAVE_MIN}-${CHAVE_MAX} -n 1)
    palavra=${WORDS[$RANDOM % ${#WORDS[@]}]}

    echo "--\"$palavra\" na chave: $chave"
    ./dclient -l "$chave" "$palavra" &
done

wait
echo "--.--"