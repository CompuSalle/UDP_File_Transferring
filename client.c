#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_FILENAME_SIZE 40
#define MAX_DATA_SIZE 10000
#define MAX_BUFFER_SIZE 1024

struct XDATA {
    char filename[MAX_FILENAME_SIZE];
    char newFileName[MAX_FILENAME_SIZE];
    short packetNo;
    short nPackets;
    short packetSize;
    char data[MAX_DATA_SIZE];
};

void* receive_packets(void* arg) {
    int client_socket = *((int*)arg);
    struct sockaddr_in server_address;
    char buffer[sizeof(struct XDATA)];
    ssize_t bytes_received;

    while (1) {
        // Receive a packet from the server
        bytes_received = recv(client_socket, buffer, sizeof(struct XDATA), 0);
        if (bytes_received == -1) {
            perror("Receive failed");
            exit(EXIT_FAILURE);
        }

        if (bytes_received == 0) {
            break; // Server closed the connection
        }

        // Process the received packet
        struct XDATA* packet = (struct XDATA*)buffer;
        printf("Received packet %d: %d bytes\n", packet->packetNo, bytes_received);
        printf("Filename: %s\n", packet->filename);
        printf("New Filename: %s\n", packet->newFileName);
        printf("Packet Size: %d\n", packet->packetSize);
    }

    pthread_exit(NULL);
}

int main() {
    int client_socket;
    struct sockaddr_in server_address;
    char buffer[MAX_BUFFER_SIZE];

    // Create a UDP socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(1234); // Replace with the actual server port number

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read the command from the user
        memset(buffer, 0, MAX_BUFFER_SIZE);
        fgets(buffer, MAX_BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        // Send the command to the server
        ssize_t bytes_sent = send(client_socket, buffer, strlen(buffer), 0);
        if (bytes_sent == -1) {
            perror("Send failed");
            exit(EXIT_FAILURE);
        }

        if (strncmp(buffer, "Send1", 5) == 0 || strncmp(buffer, "Send2", 5) == 0) {
            // Check if the new file exists and delete it if it does
            char newFileName[MAX_FILENAME_SIZE];
            sscanf(buffer, "%*s %*s %s", newFileName);

            if (access(newFileName, F_OK) != -1) {
                printf("File %s exists. Deleting...\n", newFileName);
                if (remove(newFileName) != 0) {
                    perror("File deletion failed");
                    exit(EXIT_FAILURE);
                }
                printf("File deleted.\n");
            }
        }

        if (strncmp(buffer, "Send1", 5) == 0) {
            // Create a background thread to receive packets
            pthread_t receive_thread;
            pthread_create(&receive_thread, NULL, receive_packets, (void*)&client_socket);
        }
    }

    // Close the client socket
    close(client_socket);

    return 0;
}
