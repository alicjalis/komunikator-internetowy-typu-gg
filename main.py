import socket
import threading
import time

def connect_to_server(hostname, port):
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

def send_message(client_socket, receiver_id, message):
   command = f"send_message {receiver_id} {message}\n"
   client_socket.sendall(command.encode('utf-8'))

def receive_message(client_socket):
   msg_buffer = bytearray()
   while True:
       chunk = client_socket.recv(1)
       if chunk == b'\n':
           break
       msg_buffer += chunk
   return msg_buffer.decode('utf-8')


class ClientThread(threading.Thread):
   def __init__(self, client_socket):
       threading.Thread.__init__(self)
       self.client_socket = client_socket

   def run(self):
       while True:
           try:
               msg = receive_message(self.client_socket)
               print("Received message: ", msg)
           except Exception as e:
               print("Error receiving message: ", e)
               break

def main():
   hostname = "192.168.80.130"
   port = 1234
   username = input("Enter your username: ")

   client_socket = connect_to_server(hostname, port)
   send_username(client_socket, username)
   print("Your id is: ", receive_id(client_socket))

   client_thread = ClientThread(client_socket)
   client_thread.start()

   while True:
       receiver_id = input("Enter the id of the user you want to send a message to: ")
       send_message(client_socket, receiver_id, "")
       time.sleep(0.1)  # Opóźnienie 0.1 sekundy
       msg = receive_message(client_socket)
       if msg.startswith("No such user:"):
           print(msg)
           continue
       else:
           message = input("Enter your message: ")
           send_message(client_socket, receiver_id, message)
           break


if __name__ == "__main__":
   main()
