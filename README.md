Student Name :  Saleem Thapa
Student ID : 18209745
Project Name: Sending Files over TCP Sockets (Project 1)

Server Program:
Purpose:
The server program is responsible for accepting incoming connections from clients, receiving files sent by clients, and saving the received files to the specified directory.

Dependencies:
C++ standard libraries
POSIX sockets (sys/socket.h, netinet/in.h, arpa/inet.h)
Filesystem utilities (unistd.h, fstream, cstring, cstdlib)

Usage:
./server <server_port> <save_directory>
<server_port>: Port number on which the server listens for incoming connections.
<save_directory>: Directory where received files will be saved.

Implementation Details:
The server creates a socket and binds it to the specified port.
It listens for incoming connections from clients.
Upon connection, it accepts the client connection and assigns a unique connection ID.
The server receives the filename and data size from the client.
It creates a directory in the specified save directory if it does not exist.
The server receives the file data from the client and saves it to a file with the connection ID as the filename.

Client Program:
Purpose:
The client program connects to the server and sends a specified file to the server.

Dependencies:
C++ standard libraries
POSIX sockets (sys/socket.h, netinet/in.h, arpa/inet.h)
Filesystem utilities (unistd.h, fstream, cstring, cstdlib)

Usage:
./client <server_ip_address> <server_port> <file_to_send>
<server_ip_address>: IP address of the server.
<server_port>: Port number on which the server is listening.
<file_to_send>: Path to the file to be sent to the server.

Implementation Details:
The client creates a socket and connects to the server using the specified IP address and port number.
It opens the specified file for reading.
The client reads the file data in chunks and sends it to the server.
Once all data is sent, the client closes the socket and the file.

Problems during the projects:
Problem: Error handling for invalid port number and missing command-line arguments.
  Solution: Implemented checks in both server and client programs to ensure correct usage and provided informative error messages to the user.
Problem: Handling signals (e.g., SIGQUIT, SIGTERM) to exit gracefully.
  Solution: Implemented a signal handler function in the server program to catch signals and perform cleanup actions before exiting.
Problem: Resolving compilation warnings about the use of C++11 features.
  Solution: Updated code to explicitly declare types instead of using auto, ensuring compatibility with older C++ standards.

HOW TO RUN THE PROGRAM:
Step 1 : You need to have two machines either two different one or in a virtual environment.
Step 2 : Find the ipv4 address of the machine where the server will be runnning ( for mac users : type "ifconfig" , for windows users: "ipconfig")
** Note down that address**
Step 3 : Open terminal and run the command " make"
Step 4: Run the server first using  this command " ./server 8080 save "
Step 5 : In another machine , run the command "make"
Step 6 : Before running the client , make sure to have some file that needs to sent to server.
Step 7 : Run the client using this command " ./client <ip address of the server> <filename>
*** For example : ./client 192.168.1.1 abc.txt
