#ifndef META_H
#define META_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024


#define INDEXFILE "./index.txt"

typedef struct index
{
    uint64_t chave;
    int contador;

    char titulo[200];
    char autores[200];
    char ano[5];
    char path[64];
    
    struct index *prox; 
}Index, H_Meta[SIZE];

typedef struct mensagem
{
    char flag[3];

    char titulo[200];
    char autores[200];
    char ano[5];
    char path[64];

    uint64_t chave;

    char palavra[100];

    int nr_processos;

}Mensagem;



int encontra_palavra(char *file, char *palavra);


uint64_t cria_chave(char *s1,char *s2,char *s3,char *s4);

uint64_t  indexaMeta(H_Meta meta, char *titulo, char *autores, char *ano, char *path, uint64_t chave);

uint64_t indexa_file(char *titulo, char *autores, char *ano, char *path,uint64_t chave);

int metaToFile(H_Meta meta);

int fileToMeta(H_Meta meta,int cache_size);

char *procuraChave(H_Meta meta,uint64_t chave);

int apagaMeta(H_Meta meta,uint64_t chave);

int apaga_do_ficheiro(H_Meta meta, uint64_t chave,int indice, int indices_indexados);

int procuraPalavra(H_Meta meta,uint64_t chave,char *palavra, char *cd);

char *procura_todos(char *palavra, int nr_processos,char *cd);

void enviar_resposta(int fd,char *msg);



#endif