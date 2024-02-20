import socket
import threading
import time

# Funkcja łącząca się z serwerem
def connect_to_server(hostname, port):
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((hostname, port))
    return client_socket

# Funkcja wysyłająca nazwę użytkownika do serwera
def send_username(client_socket, username):
    client_socket.sendall(username.encode('utf-8'))

# Funkcja wysyłająca identyfikator odbiorcy do serwera
def send_id(client_socket, receiver_id):
    client_socket.sendall(receiver_id.encode('utf-8'))

# Funkcja odbierająca identyfikator od serwera
def receive_id(client_socket):
    id_buffer = bytearray()
    while True:
        chunk = client_socket.recv(1)
        if chunk == b'\n':
            break
        id_buffer += chunk
    return id_buffer.decode('utf-8')

# Funkcja wysyłająca wiadomość do serwera
def send_message(client_socket, message):
    client_socket.sendall(message.encode('utf-8'))

# Funkcja odbierająca wiadomość od serwera
def receive_message(client_socket):
    msg_buffer = bytearray()
    while True:
        chunk = client_socket.recv(1)
        if chunk == b'\n':
            break
        msg_buffer += chunk
    return msg_buffer.decode('utf-8')

def receive_messages(client_socket):
    msg_buffer = bytearray()
    while True:
        chunk = client_socket.recv(1)
        if not chunk:  # Jeśli chunk jest pusty, oznacza to koniec danych
            break
        msg_buffer += chunk
    return msg_buffer.decode('utf-8')

# Funkcja odbierająca wszystkie wiadomości od serwera
def receive_all_messages(client_socket):
    messages = []
    while True:
        msg = receive_message(client_socket)
        print(msg)
        if not msg:
            break
        messages.append(msg)
        print(messages)
    return messages

def message_receiver(client_socket):
    while getattr(threading.current_thread(), "running", True):
        message = receive_message(client_socket)
        if message:
            print("Received message:", message)
        else:
            print("No messages received")
        break  # Zakończ pętlę po odebraniu jednej wiadomości


# Główna funkcja programu klienta
def main():
    hostname = "192.168.80.130"
    port = 1234
    username = input("Enter your username: ")

    client_socket = connect_to_server(hostname, port)
    send_username(client_socket, username)
    print("Your id is: ", receive_id(client_socket))
    usernames_and_ids_msg = client_socket.recv(4096).decode('utf-8')
    print(usernames_and_ids_msg)

    # client_thread = ClientThread(client_socket)
    # client_thread.start()
    print("Choose what you want to do: \n1 - send a message\n2 - check if you have any messages\n")

    while True:
        # TODO read sprawdza czy ma wiad, dostaje albo no messages albo jakies messages
        choice = input("Enter your choice: ")
        client_socket.sendall(choice.encode('utf-8'))  # Wysłanie wyboru do serwera
        if choice == "1":
            receiver_id = input("Enter the id of the user you want to send a message to: ")
            send_id(client_socket, receiver_id)
            time.sleep(0.1)  # Opóźnienie 0.1 sekundy
            msg = receive_message(client_socket)
            if msg.startswith("No such user:"):
                print(msg)
                continue
            elif msg.startswith("You can send a message"):
                message = input("Enter your message: ")
                send_message(client_socket, message)
                continue
        elif choice == "2":
            messages = client_socket.recv(4096).decode('utf-8')
            print("Received messages:", messages)
            messages = messages.split("\n")  # Dzielenie otrzymanej wiadomości na osobne linie
            for msg in messages:
                if msg:  # Sprawdź, czy wiadomość nie jest pusta
                    sender, content = msg.split(":", 1)  # Podział na nadawcę i treść wiadomości
                    print(f"{sender}: {content}")  # Wyświetlenie nadawcy i treści wiadomości
        else:
            print("Invalid choice")


if __name__ == "__main__":
    main()