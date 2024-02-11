#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

#define MAX_USERS 15
#define MAX_USERNAME_LENGTH 20
#define MAX_MESSAGES 10
#define MAX_MESSAGE_LENGTH 256
#define MAX_ERROR_MSG_LENGTH 300

// Struktura przechowująca informacje o wiadomości
struct message
{
    char sender[MAX_USERNAME_LENGTH];
    char content[MAX_MESSAGE_LENGTH];
};

// Struktura przechowująca informacje o kliencie
struct cln
{
    int cfd;
    struct sockaddr_in caddr;
    char nickname[MAX_USERNAME_LENGTH];
    int id;
    struct message messages[MAX_MESSAGES];
    int messages_count;
};

// Struktura przechowująca informacje o użytkownikach
struct users
{
    int counter;
    struct cln clients[MAX_USERS];
};
struct users users;

// Mutexy do synchronizacji dostępu do struktury users
pthread_mutex_t users_mutex = PTHREAD_MUTEX_INITIALIZER;

// Funkcja dodająca użytkownika do globalnej listy użytkowników
void add_user(struct users *user_list, const char *usernames)
{
    pthread_mutex_lock(&users_mutex);
    if (user_list->counter < MAX_USERS)
    {
        strcpy(user_list->clients[user_list->counter].nickname, usernames);
        user_list->clients[user_list->counter].id = user_list->counter;
        user_list->counter++;
    }
    else
    {
        printf("User list is full. Cannot add more users.\n");
    }
    pthread_mutex_unlock(&users_mutex);
}

// Funkcja przechowująca odebraną wiadomość u odbiorcy
void store_message(struct cln *receiver, const char *sender, const char *message)
{
    if (receiver->messages_count < MAX_MESSAGES)
    {
        strcpy(receiver->messages[receiver->messages_count].sender, sender);
        strcpy(receiver->messages[receiver->messages_count].content, message);
        printf("wiadomosc: %s\n", message);
        printf("sender: %s\n", sender);
        printf("receiver: %s\n", receiver->nickname);
        receiver->messages_count++;
    }
    else
    {
        printf("Message buffer is full for user %s. Cannot store more messages.\n", receiver->nickname);
    }
}

// Funkcja wysyłająca wszystkie wiadomości do klienta
void send_all_messages(struct cln receiver)
{
    char send_msg[300];
    // printf("Twoja ilosc odebranych wiadomosci: %d\n", receiver.messages_count);

    for (int i = 0; i < receiver.messages_count; i++)
    {
        snprintf(send_msg, sizeof(send_msg), "%s: %s\n", receiver.messages[i].sender, receiver.messages[i].content);
        write(receiver.cfd, send_msg, strlen(send_msg));
    }

    // Po wysłaniu wszystkich wiadomości wysyłamy pustą wiadomość jako sygnał kończący
    //write(receiver.cfd, "END_OF_MESSAGES\n", strlen("END_OF_MESSAGES\n") + 1);
    receiver.messages_count = 0; // Resetujemy liczbę wiadomości po wysłaniu
}

void show_all_usernames(struct cln *client)
{
    char message[512];
    pthread_mutex_lock(&users_mutex);
    sprintf(message, "All usernames and IDs:\n");
    for (size_t i = 0; i < users.counter; ++i)
    {
        sprintf(message + strlen(message), "- ID: %d, Username: %s\n", users.clients[i].id, users.clients[i].nickname);
    }
    pthread_mutex_unlock(&users_mutex);
    write(client->cfd, message, strlen(message));
}

// Wątek obsługujący komunikację z klientem
void *cthread(void *arg)
{
    struct cln *client_info = (struct cln *)arg;
    int cfd = client_info->cfd;
    char receiver_id[256];

    ssize_t username_rc = read(cfd, client_info->nickname, sizeof(client_info->nickname) - 1);
    client_info->nickname[username_rc] = '\0';
    printf("Client connected: %s\n", client_info->nickname);
    // Sprawdź, czy użytkownik już istnieje na liście
    int existing_user_index = -1;
    for (int i = 0; i < users.counter; ++i)
    {
        if (strcmp(users.clients[i].nickname, client_info->nickname) == 0)
        {
            existing_user_index = i;
            break;
        }
    }

    // Jeśli użytkownik istnieje, zaktualizuj jego deskryptor
    if (existing_user_index != -1)
    {
        users.clients[existing_user_index].cfd = cfd;
        // Odeślij klientowi jego istniejące id
        char id_buffer[10];
        sprintf(id_buffer, "%d\n", users.clients[existing_user_index].id);
        write(cfd, id_buffer, strlen(id_buffer));
        show_all_usernames(client_info);
    }
    else
    {
        // Użytkownik nie istnieje, więc dodaj go do listy
        client_info->id = users.counter; // Nowe id
        add_user(&users, client_info->nickname);
        // Odeślij klientowi jego nowe id
        char id_buffer[10];
        sprintf(id_buffer, "%d\n", client_info->id);
        write(cfd, id_buffer, strlen(id_buffer));
        show_all_usernames(client_info);
    }
    int id = client_info->id;
    while (1)
    {

        char choice[256];
        ssize_t msg_length = read(cfd, choice, sizeof(choice) - 1); // Odbior wyboru
        if (msg_length <= 0)
        {
            // Błąd lub zamknięcie połączenia, wyjście z pętli
            perror("Error reading data");
            break;
        }
        choice[msg_length] = '\0';
        if (strcmp(choice, "1") == 0)
        {

            ssize_t rc = read(cfd, receiver_id, sizeof(receiver_id));
            if (rc <= 0)
            {
                // Błąd lub zamknięcie połączenia, wyjście z pętli
                perror("Error reading data");
                break;
            }

            receiver_id[rc] = '\0';
            int received_id = atoi(receiver_id); // Konwertowanie otrzymanego ciągu na int

            // Sprawdzanie, czy klient o danym id istnieje
            int client_exists = 0;
            for (int i = 0; i < users.counter; ++i)
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

            if (client_exists == 0)
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
                // Znajdź nadawcę wiadomości
                char sender[MAX_USERNAME_LENGTH];
                strcpy(sender, client_info->nickname);
                printf(" %s\n", client_info->nickname);

                // Przechowywanie wiadomości u odpowiedniego klienta
                for (size_t i = 0; i < users.counter; ++i)
                {

                    if (users.clients[i].id == received_id)
                    {
                        store_message(&users.clients[i], sender, message);
                        int test = users.clients[i].messages_count;
                        printf("Ilosc wiadomosci: %d\n", test);

                        // char formatted_message[MAX_MESSAGE_LENGTH];
                        // sprintf(formatted_message, "%s: %s", sender, message);
                        // write(users.clients[i].cfd, formatted_message, strlen(formatted_message));
                        break;
                    }
                }
            }
        }
        else if (strcmp(choice, "2") == 0)
        {
            send_all_messages(users.clients[id]);
        }
        else
        {
            // W przypadku innego wyboru, poinformuj klienta o błędzie
            char error_msg[MAX_ERROR_MSG_LENGTH];
            snprintf(error_msg, sizeof(error_msg), "Invalid choice: %s\n", choice);
            write(cfd, error_msg, strlen(error_msg));
        }
    }
    // Zamknięcie socketu klienta
    close(cfd);
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

    // Zamkniecie socketu serwera
    close(sfd);

    return 0;
}