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
        if msg == "END_OF_MESSAGES\n":
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

    def return_to_choice():
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

        def receive_thread():
            msg = receive_message(client_socket)
            if msg.startswith("No such user:"):
                receiver_id_label.pack_forget()
                receiver_id_entry.pack_forget()
                receiver_id_button.pack_forget()
                message_label = tk.Label(root, text=msg)
                message_label.pack()
                send_message_button.pack()
                return_button= tk.Button(root, text="Return", command=return_to_choice)
                return_button.pack()
            elif msg.startswith("You can send a message"):
                receiver_id_label.pack_forget()
                receiver_id_entry.pack_forget()
                receiver_id_button.pack_forget()
                can_send_label = tk.Label(root, text="Enter a message")
                can_send_label.pack()
                can_send_entry = tk.Entry(root)
                can_send_entry.pack()

                def send_message_to_client():
                    message_entry = can_send_entry.get()
                    send_message(client_socket, message_entry)
                    message_sent_label= tk.Label(root, text="Message sent")
                    message_sent_label.pack()
                    # Ukrywanie elementów po wysłaniu wiadomości
                    can_send_label.pack_forget()
                    can_send_entry.pack_forget()
                    send_button.pack_forget()
                    return_to_choice()


                send_button = tk.Button(root, text="Enter", command=send_message_to_client)
                send_button.pack()

        receive_thread = threading.Thread(target=receive_thread)
        receive_thread.start()

        receiver_id_button = tk.Button(root, text="Enter", command=choose_receiver_id)
        receiver_id_button.pack()

    def check_messages():
        choice_label.pack_forget()
        send_message_button.pack_forget()
        check_messages_button.pack_forget()
        choice = "2"
        client_socket.sendall(choice.encode('utf-8'))  # Wysłanie wyboru do serwera

        def receive_messages_thread(root):
            messages = receive_all_messages(client_socket)
            if messages:
                messages_output = tk.Text(root)
                messages_output.pack()
                for msg in messages:
                    messages_output.insert(tk.END, msg + "\n")

            else:
                no_messages_text = tk.Label(root, text="No messages")
                no_messages_text.pack()
        receive_thread = threading.Thread(target=receive_messages_thread, args=(root,))
        receive_thread.start()

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
    check_messages_button = tk.Button(root, text="Check if you have any messages", command = check_messages)

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
