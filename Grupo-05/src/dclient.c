#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "meta.h"

#define BUF_SIZE 4012

int main(int argc, char* argv[]) 
{
    Mensagem msg;
    memset(&msg,0,sizeof(Mensagem));

    if(strcmp(argv[1], "-a") == 0)
    {
        if (argc != 6) 
        {
            printf("Erro: A flag -a requer 4 argumentos.\n");
            printf("Uso: %s -a titulo autor ano path\n", argv[0]);
            return EXIT_FAILURE;
        }
    
        strcpy(msg.flag, "-a");
        strcpy(msg.titulo, argv[2]);
        strcpy(msg.autores, argv[3]);  
        strcpy(msg.ano, argv[4]);
        strcpy(msg.path, argv[5]);
    }
    else if(strcmp(argv[1],"-f") == 0)
    {
        if(argc != 2)
        {
            printf("Erro: A flag -f não requer argumentos.\n");
            printf("Uso: %s -f\n", argv[0]);
            return EXIT_FAILURE;
        }
        strcpy(msg.flag,"-f");
        printf("%s\n",msg.flag);
    }
    else if(strcmp(argv[1],"-d") == 0)
    {
        if(argc != 3)
        {
            printf("Erro: A flag -d requer 1 argumento.\n");
            printf("Uso: %s -d chave\n", argv[0]);
            return EXIT_FAILURE;
        }

        strcpy(msg.flag,"-d");
        msg.chave = strtoull(argv[2], NULL, 10);
    }
    else if(strcmp(argv[1],"-c") == 0)
    {
        if(argc != 3)
        {
            printf("Erro: A flag -c requer 1 argumento.\n");
            printf("Uso: %s -d chave\n", argv[0]);
            return EXIT_FAILURE;
        }

        strcpy(msg.flag,"-c");
        msg.chave = strtoull(argv[2], NULL, 10);
    }
    else if(strcmp(argv[1],"-l") == 0)
    {
        if(argc != 4)
        {
            printf("Erro: A flag -l requer 2 argumento.\n");
            printf("Uso: %s -l chave palavra\n", argv[0]);
            return EXIT_FAILURE;
        }

        strcpy(msg.flag,"-l"); 
        msg.chave = strtoull(argv[2], NULL, 10); 
        strcpy(msg.palavra,argv[3]); 
    }
    else if (strcmp(argv[1],"-s") == 0)
    {
        if(argc != 4)
        {
            printf("Erro: A flag -s requer 2 argumento.\n");
            printf("Uso: %s -s palavra nr_de_processos\n", argv[0]);
            return EXIT_FAILURE;
        }

        strcpy(msg.flag,"-s");

        strcpy(msg.palavra,argv[2]);
        
        msg.nr_processos = atoi(argv[3]);
    }
    else
    {
        printf("Else: %s\n",argv[1]);
        return EXIT_FAILURE;
    }
    
    int escrever = open("client_to_server", O_WRONLY);
    if(escrever == -1) {
        perror("Erro a abrir o ficheiro para escrita\n");
        return EXIT_FAILURE;
    }
    
    ssize_t bytes_escritos = write(escrever, &msg, sizeof(Mensagem));
    if(bytes_escritos == -1) {
        perror("Erro a Escrever para o Servidor");
        close(escrever);
        return EXIT_FAILURE;
    }
    
    close(escrever); 
    printf("Foram Escritos %ld bytes para o servidor\n", bytes_escritos);
    
    int ler = open("server_to_client", O_RDONLY);
    if(ler == -1) 
    {
        perror("Erro a abrir o ficheiro para leitura\n");
        return EXIT_FAILURE;
    }

    uint64_t tamanho_msg = 0;
    if(read(ler,&tamanho_msg,sizeof(uint64_t)) < 0)
    {
        perror("Erro na leitura");
        close(ler);
        return EXIT_FAILURE;
    }

    char buf[BUF_SIZE];
    size_t total_bytes;
    while(total_bytes < tamanho_msg)
    {
        ssize_t bytes_lidos = read(ler,buf,BUF_SIZE);
        if(bytes_lidos < 0)
        {
            perror("Erro na leitura");
        }
        buf[bytes_lidos] = '\0';
        write(STDOUT_FILENO, buf, bytes_lidos);
        total_bytes += bytes_lidos;
    }

    close(ler);
    return EXIT_SUCCESS;
}