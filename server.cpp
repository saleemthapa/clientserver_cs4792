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

void setupSaveDirectory() {
    int status = system("sudo chmod 777 save");
    if (status == -1) {
        std::cerr << "Error running chmod command" << std::endl;
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " server_port save_directory" << std::endl;
        return -1;
    }

    int serverPort = atoi(argv[1]);
    const char* saveDirectory = argv[2];

    // Create the save directory with appropriate permissions
    setupSaveDirectory();

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

    // Initialize connection counter
    int connectionId = 0;

    while (true) {
        // Accept a connection from a client
        sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket == -1) {
            std::cerr << "Error accepting connection" << std::endl;
            continue; // Continue to accept other connections
        }

        // Increment connection counter
        connectionId++;

        std::cout << "Connection " << connectionId << " accepted from " << inet_ntoa(clientAddress.sin_addr) << std::endl;

        // Receive filename and data size from the client
        char filename[256];
        size_t dataSize;
        ssize_t bytesRead = recv(clientSocket, &dataSize, sizeof(dataSize), 0);
        if (bytesRead != sizeof(dataSize)) {
            std::cerr << "Error receiving data size" << std::endl;
            close(clientSocket);
            continue; // Continue to accept other connections
        }
        dataSize = ntohl(dataSize); // Convert from network byte order to host byte order

        bytesRead = recv(clientSocket, filename, sizeof(filename), 0);
        if (bytesRead <= 0) {
            std::cerr << "Error receiving filename" << std::endl;
            close(clientSocket);
            continue; // Continue to accept other connections
        }

        // Create folder if not exists
        std::string savePath = std::string(saveDirectory) + std::to_string(connectionId) + ".file";
        system(("mkdir -p " + std::string(saveDirectory)).c_str());

        // Open file in save directory
        std::ofstream outputFile(savePath, std::ios::out | std::ios::binary);
        if (!outputFile.is_open()) {
            std::cerr << "Error opening output file" << std::endl;
            close(clientSocket);
            continue; // Continue to accept other connections
        }

        char buffer[1024];
        size_t totalBytesReceived = 0;
        while (totalBytesReceived < dataSize) {
            bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0) {
                std::cerr << "Error receiving data from client" << std::endl;
                close(clientSocket);
                outputFile.close();
                break; // Continue to accept other connections
            }
            totalBytesReceived += bytesRead;
            outputFile.write(buffer, bytesRead);
        }

        // Close the socket and the file
        close(clientSocket);
        outputFile.close();

        std::cout << "Data received and saved to '" << savePath << "'" << std::endl;
    }

    // Close the server socket (this will never be reached in this loop)
    close(serverSocket);

    return 0;
}

