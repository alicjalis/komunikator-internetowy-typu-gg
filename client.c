#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
 int clientSocket;
 struct sockaddr_in serverAddr;
 char serverMessage[2000], clientMessage[2000];

 // Utwórz gniazdo
 clientSocket = socket(PF_INET, SOCK_STREAM, 0);

 // Konfiguracja adresu serwera
 serverAddr.sin_family = AF_INET;
 serverAddr.sin_port = htons(atoi(argv[2]));
 serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
 memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

 // Połącz z serwerem
 connect(clientSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

 // Wysyłanie i odbieranie wiadomości
 sprintf(clientMessage, "ID:%s\t", argv[3]);
 strcat(clientMessage, argv[4]);
 send(clientSocket, clientMessage, strlen(clientMessage), 0);
 recv(clientSocket, serverMessage, 2000, 0);
 printf("Received from server: %s\n", serverMessage);

 // Zamknij gniazdo
 close(clientSocket);

 return 0;
}
