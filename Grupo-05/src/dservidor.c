#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "meta.h"

#define EXIT_FAIL -1
#define BUF_SIZE 512
#define FILENAME "./index.txt"

int main(int argc, char *argv[])
{   
    int itens_indexados = 0;
    int max_cache_size = atoi(argv[2]);
    printf("Max Cache_Size: %d\n",max_cache_size);

    if(argc < 3)
    {
        printf("Utilização:\n");
        printf("./dservidor 'pasta_dos_documentos' 'tamanho_da_cache'\n");
        return -1;
    }

    printf("Servidor On\n");
    int aberto = 0;

    H_Meta meta;
    memset(&meta, 0, sizeof(H_Meta));

    itens_indexados = fileToMeta(meta,max_cache_size);

    int escrever = open("server_to_client",O_WRONLY);
    if (escrever == -1)
    {
        perror("Erro a abrir o ficheiro\n");
        return EXIT_FAIL;
    }

    char buf[BUF_SIZE];
    ssize_t bytes_lidos;

    while (1)
    {
        Mensagem m;
        int ler = open("client_to_server", O_RDONLY);
        if (ler == -1)
        {
            perror("Erro a abrir o ficheiro");
            return EXIT_FAIL;
        }

        while((bytes_lidos = read(ler, &m, sizeof(Mensagem))))
        {
            if (strcmp(m.flag, "-a") == 0)
            {
                printf("Indexação:\n-Título: %s\n-Autores: %s\n-Ano: %s\n-Path: %s\n", m.titulo, m.autores, m.ano, m.path);

                uint64_t chave = cria_chave(m.titulo, m.autores, m.ano, m.path);
                if (chave == -1) 
                {
                    perror("Erro ao criar chave");
                    continue;
                }
                printf("Chave: %ld\n",chave);

                if(itens_indexados < max_cache_size)
                {
                    chave = indexa_file(m.titulo, m.autores, m.ano, m.path, chave);
                    chave = indexaMeta(meta, m.titulo, m.autores, m.ano, m.path, chave);
                    if (chave == 0) 
                    {
                        perror("Erro ao indexar");
                        continue;
                    }
                    else
                        itens_indexados++;
                }
                else
                {
                    chave = indexa_file(m.titulo, m.autores, m.ano, m.path, chave);
                    if (chave == 0) 
                    {
                        perror("Erro ao indexar no file");
                        continue;
                    }
                    else
                    itens_indexados++;
                }    

                if(chave == 0)
                {
                    const char *msg =  "Erro a indexar a informção\n";
                    enviar_resposta(escrever, msg);
                }
                else
                {
                    char res[BUF_SIZE]; 
                    memset(res, 0, BUF_SIZE);

                    snprintf(res, BUF_SIZE, "Documento indexado com a chave %ld\n", chave);
                    enviar_resposta(escrever,res);
                }
            }

            //Desligar Servidor
            else if (strcmp(m.flag,"-f") == 0)
            {
            
                const char *msg = "Servidor a desligar\n";
                write(escrever,msg,strlen(msg));
                close(escrever);
                return 0;
            }

            else if (strcmp(m.flag, "-d") == 0) 
            {
                printf("Chave de procura: %ld\n", m.chave);   


                int indice;
                size_t bytes_escritos;

                if((indice = apagaMeta(meta, m.chave)) == -1)
                {
                    printf("A chave não se encontrava indexada em cache\n");
                    
                    
                }
                if(apaga_do_ficheiro(meta,m.chave,indice,itens_indexados) == -1)
                {
                    const char *msg = "A Chave não se encontra indexada\n";
                    printf("Mensagem:%s",msg);
                    enviar_resposta(escrever,msg);
                    if(bytes_escritos <= 0)
                    {
                        perror("Erro na escrita\n");
                        
                    }
                }
                else
                {
                    char result[64];
                    sprintf(result, "O Ficheiro com a chave %ld foi desindexado\n", m.chave);
                    printf("Mensagem:%s",result);
                    enviar_resposta(escrever,result);
                }
                
                printf("Escritos bytes:%ld\n", bytes_escritos);
            }
            

            // ARG[0] -C
            else if (strcmp(m.flag, "-c") == 0)  
            {
                printf("Chave de procura: %ld\n", m.chave);
                pid_t pid = fork();
                if (pid == 0) 
                {
                    char *result = procuraChave(meta, m.chave);
                    if (result == NULL || *result == '\0') {
                        char *msg = "A chave não se encontra indexada\n";
                        enviar_resposta(escrever,msg);
                    } 
                    else 
                    {
                        enviar_resposta(escrever,result);
                    }
                    free(result);
                    exit(EXIT_SUCCESS);
                } 
            }
            else if(strcmp(m.flag,"-l") == 0)
            {
                size_t bytes_escritos;
                pid_t pid;

                if((pid = fork()) == 0)
                {
                    int nr = procuraPalavra(meta,m.chave,m.palavra,argv[1]);
                    //char result[64];
                    if(nr >= 0)
                    {
                        char result[64];
                        sprintf(result,"A palavra %s está no ficheiro %d vezes\n",m.palavra,nr);
                        printf("%s",result);
                        bytes_escritos = write(escrever, result, strlen(result)); 
                        close(escrever);
                        if(bytes_escritos <= 0)
                        {
                            perror("Erro na escrita\n");
                        }
                    }
                    else
                    {
                        printf("O ficheiro não estava indexado\n");
                        const char *msg = "A chave não se encontra indexada\n";
                        if(write(escrever,msg,strlen(msg)) < 0)
                        {
                            perror("Erro a escrever no pipe");
                        }
                       
                    }
                }
                
            }
            else if(strcmp(m.flag,"-s") == 0)
            {
                char *chaves = procura_todos(m.palavra,m.nr_processos,argv[1]);
                enviar_resposta(escrever,chaves);
                
            }
        } 
        close(ler);  
    }   
    printf("Servidor off\n");
    return EXIT_SUCCESS;
}