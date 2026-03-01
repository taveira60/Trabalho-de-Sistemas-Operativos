#!/bin/bash
WORDS=("freedom" "love" "war" "science" "hope" "death" "light" "future" "power" "knowledge")

# Se receber argumentos, usa-os como palavras
if [ "$#" -gt 0 ]; then
    WORDS=("$@")
fi

MAX_PROC=6  # Máximo de processos paralelos aleatórios

for word in "${WORDS[@]}"; do
    # Gera um número aleatório de processos entre 1 e MAX_PROC
    num_proc=$(( (RANDOM % MAX_PROC) + 1 ))

    echo "->: \"$word\" with $num_proc processo(s)..."
    
    ./dclient -s "$word" "$num_proc" &
done

# Espera todos os processos terminarem
wait
echo "<->"