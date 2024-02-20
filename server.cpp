#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstdlib>
#include <csignal>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctime>
#include <chrono>

volatile sig_atomic_t signal_received = 0;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Signal handler function
void signal_handler(int signum) {
    // Set the flag to indicate that a signal has been received
    signal_received = 1;
if (signum == SIGINT) {
        std::cerr << "Received SIGINT (Ctrl+C). Exiting gracefully." << std::endl;
        // Perform cleanup actions here if needed
        exit(signum);
    }
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

	 // Set up signal handling
    // Set up signal handling for SIGINT (Ctrl+C)
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGTERM, signal_handler);
while(!signal_received){
    int serverPort = atoi(argv[1]);
    const char* saveDirectory = argv[2];

	  // Validate server port
    if (serverPort <= 0 || serverPort > 65535) {
        error("Invalid port number");
    }


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
    //int connectionId = 0;

    while (true) {
        // Accept a connection from a client
        sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket == -1) {
            std::cerr << "Error accepting connection" << std::endl;
            continue; // Continue to accept other connections
        }
	//Resets timer 
	std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

	//Generate connection ID
	static int connectionId = 0;

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
/*
        // Receive data from the client
        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
            outputFile.write(buffer, bytesRead);

// Check if 10 seconds have passed without receiving data
            std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
	    std::chrono::seconds elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time);    
	if (elapsed_seconds > 10) {
                std::cerr << "Error: No data received for over 10 seconds" << std::endl;
                outputFile << "ERROR";
                outputFile.flush(); // Flush the output buffer
                break;
            }

		
        }
*/
	// Receive data from the client
char buffer[1024];
ssize_t totalBytesReceived = 0;
ssize_t bytesRead;
while ((totalBytesReceived < 100 * 1024 * 1024) && ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)) {
    // Write received data to the output file
    outputFile.write(buffer, bytesRead);
    totalBytesReceived += bytesRead;

    std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
    std::chrono::seconds elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time);
    // Check if 10 seconds have passed without receiving data
	
    	if (elapsed_seconds > std::chrono::seconds(10)) {
        	std::cerr << "Error: No data received for over 10 seconds" << std::endl;
        	outputFile << "ERROR"; // Write "ERROR" to the file
        	outputFile.flush(); // Flush the output buffer to ensure the data is written immediately
       		break; // Exit the loop
    	}

	// Update the start time for the next iteration of the loop
	std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

	}

	// If the loop exits due to reaching the 100 MiB limit, print an error message
	if (totalBytesReceived >= 100 * 1024 * 1024) {
    	std::cerr << "Error: Maximum file size exceeded (100 MiB)" << std::endl;
    	outputFile << "ERROR"; // Write "ERROR" to the file
    	outputFile.flush(); // Flush the output buffer to ensure the data is written immediately
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

	// Exit with appropriate code
    if (signal_received == 1) {
        exit(EXIT_SUCCESS);
    } else {
        exit(EXIT_FAILURE);
    }

}
    return 0;
}//main loop
