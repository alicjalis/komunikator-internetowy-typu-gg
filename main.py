import tkinter as tk
import socket
import threading
import time


def connect_to_server(hostname, port):
    global client_socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((hostname, port))
    return client_socket


def send_username(client_socket, username):
    client_socket.sendall(username.encode('utf-8'))


def receive_id(client_socket):
    id_buffer = bytearray()
    while True:
        chunk = client_socket.recv(1)
        if chunk == b'\n':
            break
        id_buffer += chunk
    return id_buffer.decode('utf-8')


def send_choice(client_socket, choice):
    client_socket.sendall(choice.encode('utf-8'))


def send_id(client_socket, receiver_id):
    client_socket.sendall(receiver_id.encode('utf-8'))

def send_message(client_socket, message):
    client_socket.sendall(message.encode('utf-8'))


def receive_message(client_socket):
    msg_buffer = bytearray()
    while True:
        chunk = client_socket.recv(1)
        if chunk == b'\n':
            break
        msg_buffer += chunk
    return msg_buffer.decode('utf-8')


def receive_all_messages(client_socket):
    messages = []
    while True:
        msg = receive_message(client_socket)
        if not msg:
            break
        messages.append(msg)
    return messages


def main():
    def connect():
        hostname = "192.168.80.130"
        port = 1234
        username = nickname_entry.get()
        client_socket = connect_to_server(hostname, port)
        send_username(client_socket, username)
        # Ukrycie elementów GUI
        nickname_label.pack_forget()
        nickname_entry.pack_forget()
        connect_button.pack_forget()

        # Ustawienie id użytkownika w odpowiednim miejscu GUI
        your_id_is_label = tk.Label(root, text="Your id is: ")
        your_id_is_label.pack()

        # Odbieranie id użytkownika
        user_id = receive_id(client_socket)

        # Ustawienie id użytkownika w odpowiednim miejscu GUI
        user_info_label = tk.Label(root, text=user_id)
        user_info_label.pack()


        # Odbieranie wiadomości z serwera dotyczącej użytkowników
        usernames_and_ids_msg = client_socket.recv(4096).decode('utf-8')
        usernames_and_ids_label.config(text=usernames_and_ids_msg)


        # Ustawienie opcji wysyłania wiadomości i sprawdzania wiadomości
        choice_label.pack()
        send_message_button.pack()
        check_messages_button.pack()



    def send_a_message():
        choice_label.pack_forget()
        send_message_button.pack_forget()
        check_messages_button.pack_forget()
        choice = "1"
        client_socket.sendall(choice.encode('utf-8'))  # Wysłanie wyboru do serwera
        receiver_id_label = tk.Label(root, text="Enter the id of the user you want to send a message to: ")
        receiver_id_label.pack()
        receiver_id_entry = tk.Entry(root)
        receiver_id_entry.pack()
        def choose_receiver_id():
            receiver_id = receiver_id_entry.get()
            send_id(client_socket, receiver_id)

        receiver_id_button = tk.Button(root, text="Enter", command=choose_receiver_id)
        receiver_id_button.pack()
        msg = receive_message(client_socket)
        if msg.startswith("No such user:"):
            receiver_id_label.pack_forget()
            receiver_id_entry.pack_forget()
            receiver_id_button.pack_forget()
            message_label = tk.Label(root, text=msg)
            message_label.pack()

    def receive_messages_thread(client_socket):
        while True:
            messages = receive_all_messages(client_socket)
            if messages:
                messages_text = "\n".join(messages)
            else:
                messages_text = "No messages"
            message_list.config(text=messages_text)

    def receive_id_thread(client_socket):
        usernames_and_ids_msg = client_socket.recv(4096).decode('utf-8')
        usernames_and_ids_label.config(text=usernames_and_ids_msg)

    root = tk.Tk()
    root.title("Chat Client")
    root.geometry("400x300")

    nickname_label = tk.Label(root, text="Enter your nickname:")
    nickname_label.pack()

    nickname_entry = tk.Entry(root)
    nickname_entry.pack()

    connect_button = tk.Button(root, text="Connect", command=connect)
    connect_button.pack()

    usernames_and_ids_label = tk.Label(root, text="")
    usernames_and_ids_label.pack()

    choice_label = tk.Label(root, text="")
    send_message_button = tk.Button(root, text="Send a message", command=send_a_message)
    check_messages_button = tk.Button(root, text="Check if you have any messages")

    input_label = tk.Label(root, text="Receiver ID:")
    input_entry = tk.Entry(root)

    # message_label = tk.Label(root, text="Message:")
    # message_entry = tk.Entry(root)
    #
    # message_button = tk.Button(root, text="Send")

    message_list = tk.Label(root, text="")

    root.mainloop()


if __name__ == "__main__":
    main()
