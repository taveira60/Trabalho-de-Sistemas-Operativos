#!/bin/bash


NUM_QUERIES=10         # Número de chaves a consultar
CHAVE_MIN=1000000000   # Valor mínimo da chave
CHAVE_MAX=9999999999   # Valor máximo da chave

for ((i=0; i<NUM_QUERIES; i++)); do
    chave=$(shuf -i ${CHAVE_MIN}-${CHAVE_MAX} -n 1)
    echo "Chave: $chave"
    ./dclient -c "$chave" &
done

wait
echo "..."