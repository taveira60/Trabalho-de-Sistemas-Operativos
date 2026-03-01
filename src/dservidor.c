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
   

    

    PIDList pids;

    while (aberto == 0)
    {
        Mensagem m;
        inicis_pidList(&pids);

        int ler = open("client_to_server", O_RDONLY);
        if (ler == -1)
        {
            perror("Erro a abrir o ficheiro");
            return EXIT_FAIL;
        }
        read(ler, &m, sizeof(Mensagem));
            if (strcmp(m.flag, "-a") == 0)
            {
                printf("Indexação:\n-Título: %s\n-Autores: %s\n-Ano: %s\n-Path: %s\n", m.titulo, m.autores, m.ano, m.path);

                uint64_t chave = cria_chave(m.titulo, m.autores, m.ano, m.path);
                if ((int)chave == -1) 
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
                    char *msg =  "Erro a indexar a informção\n";
                    enviar_resposta(msg,m.pid_id);
                }
                else
                {
                    char res[BUF_SIZE]; 
                    memset(res, 0, BUF_SIZE);

                    snprintf(res, BUF_SIZE, "Documento indexado com a chave %ld\n", chave);
                    enviar_resposta(res,m.pid_id);
                }
            }

            //Desligar Servidor
            else if (strcmp(m.flag,"-f") == 0)
            {
                printf("Servidor a desligar\n");
                close(ler);
                char *msg = "Servidor a desligar\n";
                enviar_resposta(msg,m.pid_id);
                tratarPid(&pids);
                free_pidList(&pids);
                return EXIT_SUCCESS;
            }

            else if (strcmp(m.flag, "-d") == 0) 
            {
                printf("Chave de procura: %ld\n", m.chave);   


                int indice;

                if((indice = apagaMeta(meta, m.chave)) == -1)
                {
                    printf("A chave não se encontrava indexada em cache\n");
                    
                    
                }
                if(apaga_do_ficheiro(meta,m.chave,indice,itens_indexados) == -1)
                {
                    char *msg = "A Chave não se encontra indexada\n";
                    printf("Mensagem:%s",msg);
                    enviar_resposta(msg,m.pid_id);
                }
                else
                {
                    char result[64];
                    sprintf(result, "O Ficheiro com a chave %ld foi desindexado\n", m.chave);
                    printf("Mensagem:%s",result);
                    enviar_resposta(result,m.pid_id);
                }
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
                        enviar_resposta(msg,m.pid_id);
                    } 
                    else 
                    {
                        enviar_resposta(result,m.pid_id);
                        free(result);
                    }
                    exit(EXIT_SUCCESS);
                }
                else
                {
                    adicionar_pid(&pids,pid);
                } 
            }
            else if(strcmp(m.flag,"-l") == 0)
            {
                pid_t pid;
                if((pid = fork()) == 0)
                {
                    
                    int nr = procuraPalavra(meta,m.chave,m.palavra,argv[1]);
                    if(nr >= 0)
                    {
                        char result[150];
                        sprintf(result,"A palavra %s está no ficheiro %d vezes\n",m.palavra,nr);
                        printf("%s",result);
                        enviar_resposta(result,m.pid_id);
                    }
                    else
                    {
                        printf("O ficheiro não estava indexado\n");
                        char *msg = "A chave não se encontra indexada\n";
                        enviar_resposta(msg,m.pid_id);
                       
                    }
                }
                else
                {
                    adicionar_pid(&pids,pid);
                    
                }
                
            }
            else if(strcmp(m.flag,"-s") == 0)
            {
                
                pid_t pid;
                if((pid =fork())== 0)
                {
                
                char *chaves = procura_todos(m.palavra,m.nr_processos,argv[1]);
                enviar_resposta(chaves,m.pid_id);
                }
                else
                {
                    adicionar_pid(&pids,pid);
                    
                }
            }
        close(ler);
    }  
    tratarPid(&pids);
    free_pidList(&pids);
    return EXIT_SUCCESS;
}