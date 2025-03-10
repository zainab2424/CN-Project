#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

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
    socklen_t addrlen = sizeof(address);
    char filename[100];
    FILE *file;

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is running on port %d...\n", PORT);

    // Accept connection from client
    if ((client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    printf("Client connected.\n");

    // Receive filename from client
    if (recv(client_socket, filename, sizeof(filename), 0) <= 0) {
        perror("Failed to receive filename");
        exit(EXIT_FAILURE);
    }

    // Try to open the requested file
    file = fopen(filename, "rb");
    if (file == NULL) {
        perror("File not found");  // Print error on the server side
        send(client_socket, FILE_NOT_FOUND, strlen(FILE_NOT_FOUND), 0); // Send error to client
    } else {
        printf("Sending file: %s\n", filename);
        send_file(file, client_socket);
        fclose(file);
    }

    // Close connection
    close(client_socket);
    close(server_fd);
    printf("Connection closed.\n");

    return 0;
}

