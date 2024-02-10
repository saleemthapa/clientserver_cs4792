#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " server_ip_address server_port file_to_send" << std::endl;
        return -1;
    }

    const char* serverIp = argv[1];
    int serverPort = atoi(argv[2]);
    const char* fileToSend = argv[3];

    // Create a socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }

    // Connect to the server
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    serverAddress.sin_addr.s_addr = inet_addr(serverIp);

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error connecting to server" << std::endl;
        close(clientSocket);
        return -1;
    }

    std::cout << "Connected to the server" << std::endl;

    // Read data from the file and send it to the server
    std::ifstream inputFile(fileToSend, std::ios::in | std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Error opening input file" << std::endl;
        close(clientSocket);
        return -1;
    }

    char buffer[1024];
    while (inputFile.read(buffer, sizeof(buffer))) {
        ssize_t bytesSent = send(clientSocket, buffer, inputFile.gcount(), 0);
        if (bytesSent == -1) {
            std::cerr << "Error sending data to server" << std::endl;
            close(clientSocket);
            inputFile.close();
            return -1;
        }
    }

    // Close the socket and the file
    close(clientSocket);
    inputFile.close();

    std::cout << "Data sent to the server" << std::endl;

    return 0;
}

