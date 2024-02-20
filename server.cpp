#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void setupSaveDirectory(const char* saveDirectory) {
    std::string absolutePath = "./" + std::string(saveDirectory);
    int status = system(("mkdir -p " + absolutePath).c_str());
    if (status == -1) {
        std::cerr << "Error creating directory" << std::endl;
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
    setupSaveDirectory(saveDirectory);

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

        // Create folder if not exists
        std::string savePath = std::string(saveDirectory) + "/" + std::to_string(connectionId) + ".file";
        setupSaveDirectory(saveDirectory); // Use the provided save directory

        // Open file in save directory
        std::ofstream outputFile(savePath, std::ios::out | std::ios::binary);
        if (!outputFile.is_open()) {
            std::cerr << "Error opening output file" << std::endl;
            close(clientSocket);
            continue; // Continue to accept other connections
        }

        // Receive data from the client
        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
            outputFile.write(buffer, bytesRead);
        }

        if (bytesRead < 0) {
            std::cerr << "Error receiving data from client" << std::endl;
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
