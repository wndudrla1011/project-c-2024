#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100

void error_handling(char *message);

typedef struct
{
    int *clnt_sock;
    char *message;
} recieve_Data;

void *send_msg(void *clnt_sock)
{
    int *cs = (int *)clnt_sock;
    while (1)
    {
        char message[BUF_SIZE];
        printf("\nserver -> : ");
        fgets(message, BUF_SIZE, stdin);
        write(*cs, message, sizeof(message));
    }
}

void *recv_msg(void *rcvdt)
{
    recieve_Data *data = (recieve_Data *)rcvdt;

    while (1)
    {
        int str_len = read(*(data->clnt_sock), data->message, sizeof(char) * BUF_SIZE);

        if (str_len != -1)
        {
            printf("\n <- client : %s\n", data->message);
        }
    }
}

int main(int argc, char *argv[])
{
    int serv_sock;
    int clnt_sock;
    char message[BUF_SIZE];
    struct sockaddr_in serv_addr, clnt_addr;
    pthread_t p_thread[2];
    socklen_t clnt_addr_size;

    printf("read port....\n");
    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    printf("set server socket\n");
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        error_handling("socket() error");
    }

    printf("set server addr...\n");
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    printf("binding...\n");
    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_handling("bind() error");
    }

    if (listen(serv_sock, 5) == -1)
    {
        error_handling("listen() error");
    }

    printf("waiting...\n");

    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1)
        error_handling("accept() error");
    printf("accept!\n");

    recieve_Data rcvdt;
    rcvdt.clnt_sock = &clnt_sock;
    rcvdt.message = message;

    int t;
    int status;
    for (t = 0; t < 2; t++)
    {
        if (t == 0)
            pthread_create(&p_thread[t], NULL, send_msg, (void *)&clnt_sock);
        else if (t == 1)
            pthread_create(&p_thread[t], NULL, recv_msg, (void *)&rcvdt);
    }

    pthread_join(p_thread[0], (void **)&status);
    pthread_join(p_thread[1], (void **)&status);

    close(clnt_sock);
    close(serv_sock);

    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}