#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 12345
#define MAX_PACKET_SIZE 100
#define FILE_NOT_FOUND "ERROR: File not found."

void send_file(FILE *fp, int client_socket) {
    char buffer[MAX_PACKET_SIZE];
    size_t bytes_read;
    
    // Read file in chunks and send to client
    while ((bytes_read = fread(buffer, 1, MAX_PACKET_SIZE, fp)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char filename[100];
    FILE *file;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Attaching socket to the port 12345
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Binding socket to the port 12345
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Accept connection from client
    if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Receive filename from client
    if (recv(client_socket, filename, sizeof(filename), 0) <= 0) {
        perror("recv");
        exit(EXIT_FAILURE);
    }

    // Open file
    if ((file = fopen(filename, "rb")) == NULL) {
        send(client_socket, FILE_NOT_FOUND, strlen(FILE_NOT_FOUND), 0);
    } else {
        // Send file to client
        send_file(file, client_socket);
        fclose(file);
    }

    // Close connection
    close(client_socket);
    close(server_fd);
    return 0;
}
