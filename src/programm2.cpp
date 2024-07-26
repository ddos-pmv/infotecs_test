#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

class Server {
public:
    Server(int port) : port(port), server_fd(-1), client_socket(-1) {
        setupServer();
    }

    ~Server() {
        closeConnections();
    }

    void run() {
        while (true) {
            waitForConnection();
            handleClient();
            close(client_socket);
        }
    }

private:
    int port;
    int server_fd;
    int client_socket;
    struct sockaddr_in address;

    void setupServer() {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == 0) {
            perror("Socket failed");
            exit(EXIT_FAILURE);
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        int opt = 1;
        if(setsockopt(server_fd,SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0){
            perror("Set sock flags failed");
            exit(EXIT_FAILURE);
        }

        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("Bind failed");
            exit(EXIT_FAILURE);
        }

        if (listen(server_fd, 3) < 0) {
            perror("Listen failed");
            exit(EXIT_FAILURE);
        }

        std::cout << "Server listening on port " << port << "..." << std::endl;
    }

    void waitForConnection() {
        std::cout << "Waiting for connection..." << std::endl;
        client_socket = accept(server_fd, NULL, NULL);
        if (client_socket < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        std::cout << "Connection established." << std::endl;
    }

    void handleClient() {
        while (true) {
            int sum;
            int bytes_received = recv(client_socket, &sum, sizeof(sum), 0);
            if (bytes_received <= 0) {
                std::cout << "Connection lost, waiting for reconnection..." << std::endl;
                break;
            }

            std::cout << "Received sum: " << sum << std::endl;
            if (sum > 2 && sum % 32 == 0) {
                std::cout << "Valid data received." << std::endl;
            } else {
                std::cout << "Error: Invalid data." << std::endl;
            }
        }
    }

    void closeConnections() {
        if (client_socket >= 0) {
            close(client_socket);
        }
        if (server_fd >= 0) {
            close(server_fd);
        }
    }
};

int main() {
    Server server(8080);
    server.run();
    return 0;
}
