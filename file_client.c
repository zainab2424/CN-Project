#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define MAX_PACKET_SIZE 100

void receive_file(int server_socket, FILE *fp) {
    char buffer[MAX_PACKET_SIZE];
    int bytes_received;

    // Receive file data from server and write to file
    while ((bytes_received = recv(server_socket, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytes_received, fp);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_socket;
    struct sockaddr_in server_address;
    char *filename = argv[2];
    FILE *file;

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // Convert server IP from text to binary form
    if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Send filename to server
    send(server_socket, filename, strlen(filename), 0);

    // Open file for writing
    if ((file = fopen(filename, "wb")) == NULL) {
        perror("File open error");
        exit(EXIT_FAILURE);
    }

    // Receive file from server
    receive_file(server_socket, file);

    // Close file and socket
    fclose(file);
    close(server_socket);

    printf("File received successfully: %s\n", filename);
    return 0;
}

