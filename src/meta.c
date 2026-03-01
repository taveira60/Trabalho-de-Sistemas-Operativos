#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "meta.h"

int encontra_palavra(char *file, char *palavra) {
    int pipe_fd[2];
    int contador[2];
    int ret = -1;
    pid_t pids[2];

    if (pipe(pipe_fd) == -1 || pipe(contador) == -1) {
        perror("Erro na criação do pipe");
        return EXIT_FAILURE;
    }

    pid_t pidu;
    if ((pidu = fork()) == 0) {
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);
        close(contador[0]);
        close(contador[1]);

        execl("/bin/grep", "grep", palavra, file, NULL);
        perror("Erro no execl");
        exit(EXIT_FAILURE);
    } 
    else 
    {
        pids[0] = pidu;
    }

    
    pid_t pid;
    if ((pid = fork()) == 0) {
        close(pipe_fd[1]);
        close(contador[0]);
        dup2(pipe_fd[0], STDIN_FILENO);
        close(pipe_fd[0]);
        dup2(contador[1], STDOUT_FILENO);
        close(contador[1]);

        execlp("wc", "wc", "-l", NULL);
        perror("Erro no execlp");
        exit(EXIT_FAILURE);
    } 
    else
    {
        pids[1] = pid;
    } 
    

    close(pipe_fd[0]);
    close(pipe_fd[1]);
    close(contador[1]);

    char buf[64];
    ssize_t bytes_lidos = read(contador[0], buf, sizeof(buf) - 1);
    close(contador[0]);

    if (bytes_lidos > 0) {
        buf[bytes_lidos] = '\0';
        ret = atoi(buf);
    }

    for(int i = 0; i < 2; i++)
    {
        int status;
        pid_t pid = waitpid(pids[i],&status,0);
        if (WIFEXITED(status)) {
            printf("PID %d terminou normalmente com status %d\n", pid, WEXITSTATUS(status));
        }
    }
    return ret;
}


uint64_t mix(uint64_t h, uint64_t k)
{
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccdULL;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53ULL;
    k ^= k >> 33;
    return h ^ k;
}

uint64_t cria_chave(char *s1, char *s2, char *s3, char *s4)
{
    uint64_t chave = 14695981039346656037ULL;

    for (int i = 0; s1[i] != '\0'; i++)
    {
        uint64_t v = ((uint8_t)s1[i] ^ (i + 2)) * 0x9e3779b97f4a7c15ULL;
        chave = mix(chave, v << 7);
    }

    for (int i = 0; s2[i] != '\0'; i++)
    {
        uint64_t v = (uint8_t)s2[i] * (i + 1) * 0xdeadbeefdeadbdefULL;
        chave = mix(chave, v);
    }

    for (int i = 0; s3[i] != '\0'; i++)
    {
        uint64_t v = ((uint8_t)s3[i] + (i + 3)) ^ (0x811c9dc5 << (i % 32));
        chave = mix(chave, v << 3);
    }

    for (int i = 0; s4[i] != '\0'; i++)
    {
        uint64_t v = ((uint8_t)s4[i] + (i + 2)) * (i + 1) * 0xad93ae235eedbeefULL;
        chave = mix(chave, v << 21);
    }

    chave = mix(chave, chave);
    if (chave == 0)
        chave = mix(chave, chave);
    return chave;
}

uint64_t indexaMeta(H_Meta meta, char *titulo, char *autores, char *ano, char *path, uint64_t chave)
{

    int ind = chave % SIZE;
    if (meta[ind].chave == 0)
    {
        strcpy(meta[ind].titulo, titulo);
        strcpy(meta[ind].autores, autores);
        strcpy(meta[ind].ano, ano);
        strcpy(meta[ind].path, path);
        meta[ind].prox = NULL;
        meta[ind].chave = chave;
        meta[ind].contador = 0;
    }
    else
    {
        Index *atual = &meta[ind];
        Index *anterior = atual;

        while (atual != NULL)
        {

            if (atual->chave == chave)
            {
                printf("Já estava indexado\n");
                return 0;
            }
            anterior = atual;
            atual = atual->prox;
        }

        Index *novo = (Index *)malloc(sizeof(Index));
        strcpy(novo->titulo, titulo);
        strcpy(novo->autores, autores);
        strcpy(novo->ano, ano);
        strcpy(novo->path, path);
        novo->chave = chave;
        novo->contador = 0;
        novo->prox = NULL;
        anterior->prox = novo;
    }
    return chave;
}

uint64_t indexa_file(char *titulo, char *autores, char *ano, char *path, uint64_t chave)
{
    int fd_index = open(INDEXFILE, O_CREAT | O_RDWR | O_APPEND, 0644);
    if(fd_index == -1)
    {
        perror("Erro a abrir o index.txt");
        return 0;
    }

    Index ind;
    memset(&ind, 0, sizeof(Index));
    lseek(fd_index,0,SEEK_SET);

    while (read(fd_index, &ind, sizeof(Index)))
    {
        if (ind.chave == chave)
        {
            printf("Já estava indexada com essa chave\n");
            close(fd_index);
            return 0;
        }
    }
    

    lseek(fd_index, 0, SEEK_END);

    Index index;
    memset(&index, 0, sizeof(Index));

    strcpy(index.titulo, titulo);
    strcpy(index.autores, autores);
    strcpy(index.ano, ano);
    strcpy(index.path, path);
    index.chave = chave;
    index.contador = 0;

    ssize_t bytes_escritos;
    bytes_escritos = write(fd_index, &index, sizeof(Index));
    if (bytes_escritos != sizeof(Index))
    {
        perror("Erro de escrita para o file");
        close(fd_index);
        
        return 0;
    }
    printf("Bytes Escritos: %ld\n", bytes_escritos);
    close(fd_index);

    return chave;
}

int metaToFile(H_Meta meta)
{

    int fd_index = open(INDEXFILE, O_CREAT | O_WRONLY, 0644);
    if (fd_index < 0)
    {
        perror("Erro na abertura ou criação do Ficheiro");
        return -1;
    }

    for (int i = 0; i < SIZE; i++)
    {
        
        if (meta[i].chave != 0)
        {
            Index *temp = &meta[i];
            while (temp != NULL)
            {
                printf("Estou aqui\n");
                Index ind;
                memset(&ind, 0, sizeof(Index));

                strcpy(ind.titulo, temp->titulo);
                strcpy(ind.autores, temp->autores);
                strcpy(ind.ano, temp->ano);
                strcpy(ind.path, temp->path);
                ind.chave = temp->chave;
                ind.contador = temp->contador;


                printf("->Título: %s\n", ind.titulo);
                printf("->Autores: %s\n", ind.autores);
                printf("->Ano: %s\n", ind.ano);
                printf("->Path: %s\n", ind.path);
                printf("->Key: %ld\n", ind.chave);
                printf("->Contador: %d\n", ind.contador);

                ssize_t bytes_escritos;
                bytes_escritos = write(fd_index, &ind, sizeof(Index));
                if (bytes_escritos != sizeof(Index))
                {
                    perror("Erro de escrita para o file");
                    close(fd_index);
                    return -1;
                }

                temp = temp->prox;
                printf("Bytes Escritos: %ld\n", bytes_escritos);
            }
        }
    }
    close(fd_index);
    return 0;
}


int fileToMeta(H_Meta meta, int cache_size)
{
    Index m;
    memset(&m, 0, sizeof(Index));

    int fd_index = open(INDEXFILE, O_RDONLY);
    if (fd_index < 0)
    {
        perror("Erro a abrir o ficheiro");
        return -1;
    }

    int itens_cache = 0;
    ssize_t bytes_lidos;
    while ((bytes_lidos = read(fd_index, &m, sizeof(Index))) >= 0 && itens_cache < cache_size)
    {
        if (bytes_lidos < 0)
        {
            perror("Erro na a ler do ficheiro");
            close(fd_index);
            return -1;
        }

        int i = m.chave % SIZE;

        if (meta[i].chave == 0)
        {
            memcpy(&meta[i], &m, sizeof(Index));
            meta[i].prox = NULL;
            itens_cache++;
            continue;
        }

        Index *atual = &meta[i];
        Index *anterior = atual;
        while (atual != NULL)
        {
            if (atual->chave == m.chave)
            {
                printf("Ficheiro com esta Chave já existe\n");
                close(fd_index);
                return -1;
            }
            anterior = atual;
            atual = atual->prox;
        }

        Index *novo = (Index *)malloc(sizeof(Index));

        strcpy(novo->titulo, m.titulo);
        strcpy(novo->autores, m.autores);
        strcpy(novo->ano, m.ano);
        strcpy(novo->path, m.path);
        novo->chave = m.chave;
        novo->contador = m.contador;
        novo->prox = NULL;
        anterior->prox = novo;
        itens_cache++;

        printf("-----------\n");
        printf("->Título: %s\n", m.titulo);
        printf("->Autores: %s\n", m.autores);
        printf("->Ano: %s\n", m.ano);
        printf("->Path: %s\n", m.path);
        printf("->Key: %ld\n", m.chave);

        printf("Bytes lidos: %ld\n", bytes_lidos);
    }

    close(fd_index);
    return itens_cache;
}

char *procuraChave(H_Meta meta, uint64_t chave)
{
    int i = chave % SIZE;
    char *buf = (char *)malloc(SIZE * sizeof(char));
    buf[0] = '\0';

    if (meta[i].chave == 0)
    {
        printf("A Chave não se encontra indexada em meta-data\n");
        
    }
    else
    {
        Index *temp = &meta[i];
        while (temp != NULL)
        {
            printf("%ld | %ld\n", temp->chave, chave);
            if (temp->chave == chave)
            {
                temp->contador++;

                char titulo[210];
                snprintf(titulo, sizeof(titulo), "Título: %s\n", temp->titulo);

                char autores[212];
                snprintf(autores, sizeof(autores), "Autores: %s\n", temp->autores);

                char ano[32];
                snprintf(ano, sizeof(ano), "Ano: %s\n", temp->ano);

                char path[128];
                snprintf(path, sizeof(path), "Path: %s\n", temp->path);

                strcat(buf, titulo);
                strcat(buf, autores);
                strcat(buf, ano);
                strcat(buf, path);

                return buf;
            }
            temp = temp->prox;
        }
        printf("Chave não se encontra em meta-data\n");
    }
    
    int fd_index = open(INDEXFILE, O_CREAT | O_RDONLY);

    Index ind;
    int j = 0;
    while (read(fd_index, &ind, sizeof(Index)))
    {
        off_t pos = j * sizeof(Index);
        if (ind.chave == chave)
        {
            printf("Key encontrada\n");

            char titulo[210];
            snprintf(titulo, sizeof(titulo), "Título: %s\n", ind.titulo);

            char autores[212];
            snprintf(autores, sizeof(autores), "Autores: %s\n", ind.autores);

            char ano[32];
            snprintf(ano, sizeof(ano), "Ano: %s\n", ind.ano);

            char path[128];
            snprintf(path, sizeof(path), "Path: %s\n", ind.path);

            strcat(buf, titulo);
            strcat(buf, autores);
            strcat(buf, ano);
            strcat(buf, path);

            ind.contador++;
            close(fd_index);

            int fd_temp = open(INDEXFILE, O_WRONLY);
            if(fd_temp == -1)
            {
                perror("Erro a abrir o ficheiro");
                return NULL;
            }
            lseek(fd_temp, pos, SEEK_SET);

            if(write(fd_temp, &ind, sizeof(Index)) < 0)
            {
                perror("Erro a escrever no ficheiro");
                close(fd_temp);
                return NULL;
            }
            close(fd_temp);
            
            return buf;
        }
        j++;
    }
    printf("Não se encontra em ficheiro\n");
    close(fd_index);
    free(buf);
    return NULL;
}

int apagaMeta(H_Meta meta, uint64_t chave)
{
    int i = chave % SIZE;
    if (meta[i].chave == 0)
    {
        printf("A Chave não se encontra indexada\n");
        return -1;
    }
    else
    {
        if (meta[i].chave == chave)
        {
            if (meta[i].prox == NULL)
            {
                memset(&meta[i], 0, sizeof(Index));
            }
            else
            {
                Index *temp = meta[i].prox;
                meta[i].chave = temp->chave;
                strcpy(meta[i].titulo, temp->titulo);
                strcpy(meta[i].autores, temp->autores);
                strcpy(meta[i].ano, temp->ano);
                strcpy(meta[i].path, temp->path);
                meta[i].prox = temp->prox;
                free(temp);
            }
            printf("KEY\n");
            return i;
        }
        Index *atual = meta[i].prox;
        Index *anterior = &meta[i];
        while (atual != NULL)
        {
            if (atual->chave == chave)
            {
                printf("A key foi encontrada\n");
                anterior->prox = atual->prox;
                free(atual);
                return i;
            }
            anterior = atual;
            atual = atual->prox;
        }
    }
    return -1;
}

int apaga_do_ficheiro(H_Meta meta, uint64_t chave, int indice, int itens_indexados)
{
    int res = -1;
    int fd_or = open(INDEXFILE, O_RDONLY);
    if (fd_or == -1)
    {
        perror("Erro abrir o ficheiro");
        fd_or = open(INDEXFILE, O_RDONLY);
    }

    int fd_temp = open("temp.txt", O_CREAT | O_WRONLY | O_TRUNC, 0660);
    if (fd_temp == -1)
    {
        perror("Erro a abrir o ficheiro");
        return EXIT_FAILURE;
    }

    ssize_t bytes_lidos;

    Index m;
    memset(&m, 0, sizeof(Index));

    if(indice == -1)
    {
        while ((bytes_lidos = read(fd_or, &m, sizeof(Index))) > 0)
        {
            if (m.chave != chave)
            {
                write(fd_temp, &m, sizeof(Index));
            }
        }
    }
    else
    {
        Index temp;
        memset(&temp, 0, sizeof(Index));
        while ((bytes_lidos = read(fd_or, &m, sizeof(Index))) > 0)
        {
            if (m.chave != chave)
            {
                if((int)(m.chave % SIZE) == indice)
                {
                
                    if((uint64_t)temp.contador < m.chave)
                    {
                        memcpy(&temp,&m,sizeof(Index));
                    }
                }
                write(fd_temp, &m, sizeof(Index));
            }
        }
        res = indexaMeta(meta,temp.titulo,temp.autores,temp.ano,temp.autores,temp.chave);
        if(res == 0)
        {
            itens_indexados--;
        }
    }

    

    close(fd_or);
    close(fd_temp);
    if (remove(INDEXFILE) != 0)
    {
        perror("Erro ao apagar o ficheiro");
        return -1;
    }
    if (rename("temp.txt", INDEXFILE) != 0)
    {
        perror("Erro a renomear o ficheiro");
        return -1;
    }
    return res;
}



int procuraPalavra(H_Meta meta, uint64_t chave, char *palavra, char *cd)
{
    int ret = -1;
    int i = chave % SIZE;

    if (meta[i].chave == 0)
    {
        printf("A Chave não se encontra indexada em cache\n");
    }
    else
    {
        Index *temp = &meta[i];
        while (temp != NULL)
        {
            if (temp->chave == chave)
            {
                int size_cd = strlen(cd) + strlen(temp->path)+1;
                char caminho[size_cd];
                caminho[0] = '\0';
                strcat(caminho,cd); strcat(caminho,temp->path);
                ret = encontra_palavra(caminho, palavra);
                break;
            }
            temp = temp->prox;
        }
    }

    int fd_index = open(INDEXFILE, O_RDONLY);
    if (fd_index < 0) {
        printf("Erro ao abrir o ficheiro de índice\n");
        return -1;
    }

    Index m;
    memset(&m, 0, sizeof(Index));

    if(ret == -1)
    {
        while ((read(fd_index, &m, sizeof(Index))) > 0)
        {
            if (m.chave == chave)
            {
                close(fd_index);
                int size_cd = strlen(cd) + strlen(m.path)+1;
                char caminho[size_cd];
                caminho[0] = '\0';
                strcat(caminho,cd); strcat(caminho,m.path);
                ret = encontra_palavra(caminho, palavra);
            }
        }
    }
    close(fd_index);
    printf("O ficheiro não estava indexado no ficheiro\n");
    return ret;
}



char *procura_todos(char *palavra, int nr_processos, char *cd) 
{
    
    int fd_index = open(INDEXFILE, O_RDONLY);
    if (fd_index == -1) 
    {
        perror("Erro ao abrir o ficheiro de indexação");
        return NULL;
    }

    off_t offset = lseek(fd_index, 0, SEEK_END);
    
    int nr_files = offset / sizeof(Index); 
    printf("Número de Ficheiros indexados: %d\n", nr_files);

    
    if (nr_processos <= 0) {
        printf("Erro: nr_processos deve ser > 0 (recebido %d)\n",
                nr_processos);
        return NULL;
    }
    
    int max_leitura = (nr_files / nr_processos) + 1;
    printf("Número máximo de índices a ler por processo: %d\n", max_leitura);

    lseek(fd_index, 0, SEEK_SET);
    close(fd_index);

    

    char *buf = malloc(nr_files * 256);
    buf[0] = '\0';
    pid_t pids[nr_processos];
   
    for (int i = 0; i < nr_processos; i++) 
    {

        int pipefd[2];
        if (pipe(pipefd))
        {
            perror("pipe");
            free(buf);
            return NULL;
        }


        pid_t pid = fork();
        if (pid == 0) 
        { 
            close(pipefd[0]);

            int fd_filho = open(INDEXFILE,O_RDONLY);
            if(fd_filho == -1)
            {
                perror("Erro ao abrir o ficheiro de indexação filho");
                return NULL;
            }

            Index ind;
            off_t start_pos = i * max_leitura * sizeof(Index);
            
            if(lseek(fd_filho, start_pos, SEEK_SET)== -1)
            {
                perror("Erro");
            }
            
            ssize_t bytes_lidos;

            for (int j = 0; j < max_leitura && ( bytes_lidos= read(fd_filho, &ind, sizeof(Index))) > 0; j++) 
            { 
                if (bytes_lidos != sizeof(Index)) 
                {
                    perror("Erro na leitura");
                    break;
                } 

            
                int size_cd = strlen(cd) + strlen(ind.path) + 1;
                char caminho[size_cd];
                snprintf(caminho, size_cd, "%s%s", cd, ind.path);

                if (encontra_palavra(caminho, palavra) > 0) 
                {
                    char temp[256];
                    int len = snprintf(temp, sizeof(temp), "->%ld\n", ind.chave);
                    printf("%s\n",temp);
                    write(pipefd[1], temp, len);
                }
            }
            close(pipefd[1]);
            exit(0);
        }
        else
        {
            pids[i] = pid;
        }

            close(pipefd[1]);

            char buffer[256];
            ssize_t bytes_lidos = 0;
            while ((bytes_lidos = read(pipefd[0], buffer, sizeof(buffer)-1)) > 0) {
                buffer[bytes_lidos] = '\0';
                strcat(buf, buffer);
            }
            close(pipefd[0]);
    }

    for(int i = 0; i < nr_processos;i++)
    {
        int status;
        pid_t pid = waitpid(pids[i],&status,0);
        if (WIFEXITED(status)) {
            printf("PID %d terminou normalmente com status %d\n", pid, WEXITSTATUS(status));
        }
    }
    close(fd_index);
    return buf;
}

void enviar_resposta(char *msg,pid_t pid)
{
    char server_to_client[64];
    sprintf(server_to_client,"server_to_client_%d",pid);
    int fd = open(server_to_client,O_WRONLY | O_NONBLOCK);
    if(fd == -1)
    {
        perror("Erro a abrir o fifo:");
        return;

    }
    uint64_t len = strlen(msg);
    write(fd,&len,sizeof(uint64_t));
    write(fd,msg,strlen(msg));
    close(fd);
}

void inicis_pidList(PIDList* list) {
    list->count = 0;
    list->tamanho = 4;  
    list->pids = malloc(list->tamanho * sizeof(pid_t));
    if (list->pids == NULL) {
        perror("malloc");
        exit(1);
    }
}

void adicionar_pid(PIDList* list, pid_t pid) {
    if (list->count == list->tamanho) {
        list->tamanho *= 2;
        list->pids = realloc(list->pids, list->tamanho * sizeof(pid_t));
        if (!list->pids) {
            perror("realloc");
            exit(1);
        }
    }
    list->pids[list->count++] = pid;

}

void free_pidList(PIDList* list) {
    free(list->pids);
    list->pids = NULL;
    list->count = 0;
    list->tamanho = 0;
}

void tratarPid(PIDList* list) {
    int status;
    for (size_t i = 0; i < list->count; i++) {
        pid_t pid = waitpid(list->pids[i], &status, 0);
        if (pid == -1) {
            perror("waitpid");
            continue;
        }
        if (WIFEXITED(status)) {
            printf("PID %d terminou normalmente com status %d\n", pid, WEXITSTATUS(status));
        }
    }
}            