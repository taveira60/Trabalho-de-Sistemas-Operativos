# Sistema de Indexação e Procura de Metadados 

Este projeto consiste num sistema cliente-servidor desenvolvido em C para gerir e pesquisar metadados de e-books. O sistema utiliza Named Pipes (FIFOs) para comunicação entre processos e implementa multiprocessamento para pesquisas paralelas.

## 🚀 Funcionalidades

- Indexação Binária: Converte catálogos (.tsv) num índice binário otimizado (index.txt).
- Servidor com Cache: Mantém metadados em memória para aceleração de consultas.
- Pesquisa Paralela: Distribui a procura de palavras por múltiplos processos filhos (fork/exec).
- Comunicação IPC: Protocolo bidirecional via FIFOs (Named Pipes).

## 📁 Estrutura do Projeto

- dservidor.c: Núcleo do sistema, gestão de cache e coordenação de pesquisa.
- dclient.c: Interface para o utilizador enviar comandos.
- meta.c / meta.h: Lógica de indexação e estruturas de dados.
- mkfifo.c: Utilitário para criar os pipes de comunicação.
- Makefile: Automação da compilação.

## 🛠️ Como Utilizar

1. Compilar o projeto:
   Execute o comando: make

2. Criar os canais (FIFOs):
   Execute o comando: ./mkfifo

3. Iniciar o Servidor:
   Execute o comando: ./dservidor 500
   (O número 500 define o tamanho da cache)

4. Exemplos de Comandos do Cliente:
   (Abra outro terminal)

   - Indexar novo livro:
     ./dclient -i "Titulo" "Autor" "2024" "caminho/livro.txt"

   - Pesquisa por chave:
     ./dclient -l "ID_DA_CHAVE" "palavra"

   - Pesquisa Global (Distribuída):
     ./dclient -s "palavra" 4

## 🔄 Fluxo de Funcionamento



1. O Cliente envia um pedido para o pipe "client_to_server".
2. O Servidor processa o pedido e, se necessário, delega a pesquisa a processos filhos.
3. O resultado é enviado de volta através de um pipe exclusivo: "server_to_client_PID".

## 🧹 Limpeza

Para apagar binários e ficheiros temporários:
Execute o comando: make clean
