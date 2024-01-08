#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#define MAXLINE 1024

void *handle_client(void *arg);

int main(int argc, char *argv[]) {
 int listenfd, connfd;
 char buff[MAXLINE];
 struct sockaddr_in servaddr;

 // Tworzenie gniazda
 if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket error");
    exit(1);
 }

 // Konfiguracja adresu serwera
 bzero(&servaddr, sizeof(servaddr));
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
 servaddr.sin_port = htons(atoi(argv[1]));

 // Powiązanie adresu z gniazdem
 if ((bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) == -1) {
    perror("bind error");
    exit(1);
 }

 // Nasłuchiwanie na gnieździe
 if ((listen(listenfd, 5)) == -1) {
    perror("listen error");
    exit(1);
 }

 while (1) {
    // Akceptacja połączenia od klienta
    if ((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) == -1) {
        perror("accept error");
        continue;
    }

    // Utworzenie nowego wątku do obsługi połączenia klienta
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, handle_client, (void*)&connfd) != 0) {
        perror("pthread_create error");
        continue;
    }
 }

 return 0;
}

void *handle_client(void *arg) {
 int connfd = *(int*)arg;
 char buff[MAXLINE];

 // Odbieranie wiadomości od klienta
 bzero(buff, sizeof(buff));
 read(connfd, buff, sizeof(buff)-1);

 // Wysyłanie wiadomości z powrotem do klienta
 write(connfd, buff, strlen(buff));

 // Zamykanie połączenia
 close(connfd);

 return NULL;
}
