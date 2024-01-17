import socket

def connect_to_server(hostname, port):
   client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
   client_socket.connect((hostname, port))
   return client_socket

def send_username(client_socket, username):
   client_socket.sendall(username.encode('utf-8'))

def receive_id(client_socket):
   id_buffer = client_socket.recv(1024)
   return id_buffer.decode('utf-8')

def send_message(client_socket, receiver_id, message):
   command = f"send_message {receiver_id} {message}"
   client_socket.sendall(command.encode('utf-8'))

def main():
   hostname = "192.168.80.130"
   port = 1234
   username = input("Enter your username: ")

   client_socket = connect_to_server(hostname, port)
   send_username(client_socket, username)
   print("Your id is: ", receive_id(client_socket))

   while True:
       receiver_id = input("Enter the id of the user you want to send a message to: ")
       message = input("Enter your message: ")
       send_message(client_socket, receiver_id, message)

if __name__ == "__main__":
   main()
