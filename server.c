#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>

#define MAX_USERS 15
#define MAX_USERNAME_LENGTH 20

// Struktura przechowująca informacje o kliencie
struct cln
{
    int cfd;
    struct sockaddr_in caddr;
    char nickname[MAX_USERNAME_LENGTH];
    int id;
};

// Struktura przechowująca informacje o użytkownikach
struct users
{
    size_t counter;
    struct cln clients[MAX_USERS];
};
struct users users;

// Mutexy do synchronizacji dostępu do struktur users i rooms
pthread_mutex_t users_mutex = PTHREAD_MUTEX_INITIALIZER;

// Funkcja dodająca użytkownika do globalnej listy użytkowników
void addUser(struct users *userList, const char *usernames)
{
    pthread_mutex_lock(&users_mutex);
    if (userList->counter < MAX_USERS)
    {
        strcpy(userList->clients[userList->counter].nickname, usernames);
        userList->clients[userList->counter].id = userList->counter;
        userList->counter++;
    }
    else
    {
        printf("User list is full. Cannot add more users.\n");
    }
    pthread_mutex_unlock(&users_mutex);
}

// Funkcja wyświetlająca wszystkie nazwy zalogowanych użytkowników
void showAllUsernames(struct cln *client)
{
    char message[512];
    pthread_mutex_lock(&users_mutex);
    sprintf(message, "All usernames:\n");
    for (size_t i = 0; i < users.counter; ++i)
    {
        sprintf(message + strlen(message), "- %s\n", users.clients[i].nickname);
    }
    pthread_mutex_unlock(&users_mutex);
    write(client->cfd, message, strlen(message));
}

// Wątek obsługujący komunikację z klientem
void *cthread(void *arg)
{
    struct cln *client_info = (struct cln *)arg;
    int cfd = client_info->cfd;
    char buf[256];

    ssize_t username_rc = read(cfd, client_info->nickname, sizeof(client_info->nickname) - 1);
    client_info->nickname[username_rc] = '\0';
    printf("Client connected: %s\n", client_info->nickname);
    addUser(&users, client_info->nickname);
    printf("niby dodalo\n");
    client_info->id = users.counter;
    char id_buffer[10];
    sprintf(id_buffer, "%d\n", client_info->id);
    write(cfd, id_buffer, strlen(id_buffer));

    while (1)
    {
        printf("iteracja while\n");
        ssize_t rc = read(cfd, buf, sizeof(buf));
        if (rc <= 0)
        {
            // Błąd lub zamknięcie połączenia, wyjście z pętli
            perror("Error reading data");
            break;
        }

        buf[rc] = '\0';
        int received_id = atoi(buf); // Konwertowanie otrzymanego ciągu na int

        // Sprawdzanie, czy klient o danym id istnieje
        int client_exists = 0;
        for (size_t i = 0; i < users.counter; ++i)
        {
            if (users.clients[i].id == received_id)
            {
                char send_msg[256];
                sprintf(send_msg, "You can send a message\n");
                write(cfd, send_msg, strlen(send_msg));
                printf("client exists na 1\n");
                client_exists = 1;
                break;
            }
        }

        if (!client_exists)
        {
            printf("No such user: %d\n", received_id);
            char error_msg[256];
            sprintf(error_msg, "No such user: %d\n", received_id);
            write(cfd, error_msg, strlen(error_msg));
        }

        if (client_exists == 1)
        {
            char message[256];
            ssize_t msg_length = read(cfd, message, sizeof(message) - 1);
            if (msg_length <= 0)
            {
                // Błąd lub zamknięcie połączenia, wyjście z pętli
                perror("Error reading data");
                break;
            }

            message[msg_length] = '\0';

            // Wysyłanie wiadomości do odpowiedniego klienta
            for (size_t i = 0; i < users.counter; ++i)
            {
                if (users.clients[i].id == received_id)
                {
                    write(users.clients[i].cfd, message, strlen(message));
                    break;
                }
            }
        }
    }
    // Zamknięcie socketu klienta
    // close(cfd);
    free(client_info);

    return NULL;
}

int main()
{
    pthread_t tid;
    socklen_t sl;
    int sfd;

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(1234);

    sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr));
    listen(sfd, 10);

    while (1)
    {
        int *cfd_ptr = malloc(sizeof(int));
        sl = sizeof(struct sockaddr_in);
        *cfd_ptr = accept(sfd, (struct sockaddr *)&saddr, &sl);

        pthread_create(&tid, NULL, cthread, cfd_ptr);
        pthread_detach(tid);
    }

    // Zamkniecie socketu servera
    close(sfd);

    return 0;
}
//gdzie on wypisuje wiadomosci ktore pisze