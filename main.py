import socket
import threading
import time
import select

def connect_to_server(hostname, port):
   client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
   client_socket.connect((hostname, port))
   return client_socket

def send_username(client_socket, username):
   client_socket.sendall(username.encode('utf-8'))

def send_id(client_socket, receiver_id):
   client_socket.sendall(receiver_id.encode('utf-8'))

def receive_id(client_socket):
  id_buffer = bytearray()
  while True:
     chunk = client_socket.recv(1)
     if chunk == b'\n':
        break
     id_buffer += chunk
  return id_buffer.decode('utf-8')

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

def send_messages(client_socket):
    while True:
        receiver_id = input("Enter the id of the user you want to send a message to: ")
        send_id(client_socket, receiver_id)
        time.sleep(0.1) # Opóźnienie 0.1 sekundy
        message = input("Enter your message: ")
        send_message(client_socket, message)


def check_messages(client_socket):
    while True:
        # Sprawdź, czy na gnieździe klienta są jakieś wiadomości do odbioru
        ready_to_read, _, _ = select.select([client_socket], [], [], 0)
        if ready_to_read:
            # Jeżeli na gnieździe klienta są jakieś wiadomości do odbioru, odczytaj je
            msg = client_socket.recv(1024).decode()
            print('Received message: ', msg)
        else:
            # Jeżeli na gnieździe klienta nie ma żadnych wiadomości do odbioru, przejdź do następnej iteracji pętli
            continue

def main():
   hostname = "192.168.80.130"
   port = 1234
   username = input("Enter your username: ")

   client_socket = connect_to_server(hostname, port)
   send_username(client_socket, username)
   print("Your id is: ", receive_id(client_socket))
   #to przed whilem serwera

   #client_thread = ClientThread(client_socket)
   #client_thread.start()
   # Utwórz i uruchom wątki
   send_thread = threading.Thread(target=send_messages, args=(client_socket,))
   check_thread = threading.Thread(target=check_messages, args=(client_socket,))
   send_thread.start()
   check_thread.start()



if __name__ == "__main__":
   main()
