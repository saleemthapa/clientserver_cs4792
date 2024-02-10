#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <cstdlib> // For exit()
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " port_number" << std::endl;
        return -1;
    }

    int serverPort = atoi(argv[1]);

    // Create a socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }

    // Bind the socket to an IP address and port
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        close(serverSocket);
        return -1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Error listening for connections" << std::endl;
        close(serverSocket);
        return -1;
    }

    std::cout << "Server listening on port " << serverPort << "..." << std::endl;

    // Accept a connection from a client
    sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
    if (clientSocket == -1) {
        std::cerr << "Error accepting connection" << std::endl;
        close(serverSocket);
        return -1;
    }

    std::cout << "Connection accepted from " << inet_ntoa(clientAddress.sin_addr) << std::endl;

    // Receive filename and data size from the client
    char filename[256];
    size_t dataSize;
    ssize_t bytesRead = recv(clientSocket, &dataSize, sizeof(dataSize), 0);
    if (bytesRead != sizeof(dataSize)) {
        std::cerr << "Error receiving data size" << std::endl;
        close(clientSocket);
        close(serverSocket);
        return -1;
    }
    dataSize = ntohl(dataSize); // Convert from network byte order to host byte order

    bytesRead = recv(clientSocket, filename, sizeof(filename), 0);
    if (bytesRead <= 0) {
        std::cerr << "Error receiving filename" << std::endl;
        close(clientSocket);
        close(serverSocket);
        return -1;
    }

    // Create folder if not exists
    system("mkdir -p /save/");

    // Open file in save folder
    std::ofstream outputFile("/save/" + std::string(filename), std::ios::out | std::ios::binary);
    if (!outputFile.is_open()) {
        std::cerr << "Error opening output file" << std::endl;
        close(clientSocket);
        close(serverSocket);
        return -1;
    }

    char buffer[1024];
    size_t totalBytesReceived = 0;
    while (totalBytesReceived < dataSize) {
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            std::cerr << "Error receiving data from client" << std::endl;
            close(clientSocket);
            close(serverSocket);
            outputFile.close();
            return -1;
        }
        totalBytesReceived += bytesRead;
        outputFile.write(buffer, bytesRead);
    }

    // Close the sockets and the file
    close(clientSocket);
    close(serverSocket);
    outputFile.close();

    std::cout << "Data received and saved to '/save/" << filename << "'" << std::endl;

    return 0;
}
