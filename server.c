#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_CLIENTS 100

int client_sockets[MAX_CLIENTS];
int num_clients = 0;

pthread_mutex_t lock;

void handle_client(int client_socket) {
   char client_message[2000];
   recv(client_socket, client_message, 2000, 0);

   // Wysyłanie wiadomości do innych klientów
   for (int i = 0; i < num_clients; i++) {
       if (client_sockets[i] != client_socket) {
           send(client_sockets[i], client_message, strlen(client_message), 0);
       }
   }

   close(client_socket);
}

int main() {
   int serverSocket, clientSocket;
   struct sockaddr_in serverAddr;
   struct sockaddr_storage serverStorage;
   socklen_t addr_size;

   // Inicjalizacja mutex
   pthread_mutex_init(&lock, NULL);

   // Utwórz gniazdo
   serverSocket = socket(PF_INET, SOCK_STREAM, 0);

   // Skonfiguruj ustawienia struktury adresów serwera
   serverAddr.sin_family = AF_INET;
   serverAddr.sin_port = htons(7799);
   serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
   memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

   // Powiąż strukturę adresów z gniazdem
   bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

   // Nasłuchuj na gnieździe, z maksymalną kolejką żądań połączenia wynoszącą 40
   if(listen(serverSocket,50)==0)
       printf("Nasłuchiwanie\n");
   else
       printf("Błąd\n");

   while(1) {
       // Funkcja accept tworzy nowe gniazdo dla przychodzącego połączenia
       addr_size = sizeof serverStorage;
       clientSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);

       // Dodaj nowe połączenie do listy klientów
       pthread_mutex_lock(&lock);
       if (num_clients < MAX_CLIENTS) {
           client_sockets[num_clients] = clientSocket;
           num_clients++;
       }
       pthread_mutex_unlock(&lock);

       // Dla każdego żądania klienta twórz proces potomny i przypisz mu żądanie klienta do obsługi
       // Tak, że główna nit może obsługiwać następne żądanie
       pthread_t thread;
       pthread_create(&thread, NULL, (void *) handle_client, (void *) clientSocket);
       pthread_detach(thread);
   }
   return 0;
}
