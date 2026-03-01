#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    int res = mkfifo("client_to_server",0666);
   
    if(res == -1)
    {
        perror("Erro de criação do fifo: client_to_server\n");
        return -1;
    }

    /*res = mkfifo("server_to_client",0666);
    
    if(res == -1)
    {
        perror("Erro de criação do fifo: server_to_client\n");
        return -1;
    }*/

    printf("Fifo criado com sucesso\n");

    return  0;
}