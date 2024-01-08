#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>
#include <sys/unistd.h>

#define MAXLINE 1024

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    SOCKET sockfd;
    struct sockaddr_in servaddr;
    char buff[MAXLINE];

    // Inicjalizacja Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    // Tworzenie gniazda
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "socket error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Konfiguracja adresu serwera
    ZeroMemory(&servaddr, sizeof(servaddr)); // zastąp memset ZeroMemory
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    // Połączenie z serwerem
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        std::cerr << "connect error: " << WSAGetLastError() << std::endl;
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    // Wysyłanie wiadomości do serwera
    std::string message = "Hello, Server :P!";

    if (send(sockfd, message.c_str(), message.size(), 0) == SOCKET_ERROR) {
        sleep(1000);
        std::cerr << "send error: " << WSAGetLastError() << std::endl;
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    // Odbieranie wiadomości od serwera
    ZeroMemory(buff, sizeof(buff)); // zastąp memset ZeroMemory
    int recvResult = recv(sockfd, buff, sizeof(buff)-1, 0);
    if (recvResult == SOCKET_ERROR || recvResult == 0) {
        std::cerr << "recv error: " << WSAGetLastError() << std::endl;
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    // Wyświetlanie odebranej wiadomości
    std::cout << "Received: " << buff << std::endl;

    // Zamykanie połączenia
    closesocket(sockfd);
    WSACleanup();

    return 0;
}
