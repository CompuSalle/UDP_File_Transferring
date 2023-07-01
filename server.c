#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_FILENAME_SIZE 40
#define MAX_BUFFER_SIZE 10000
#define MAX_DATA_SIZE 10000

struct XDATA {
    char filename[MAX_FILENAME_SIZE];
    char newFileName[MAX_FILENAME_SIZE];
    short packetNo;
    short nPackets;
    short packetSize;
    char data[MAX_DATA_SIZE];
};

void process_send1_command(const char* command, struct sockaddr_in* client_address, int server_socket, int packetSize) {
    struct XDATA packet;

    // Extract the filename and new filename from the command
    sscanf(command, "%*s %s %s", packet.filename, packet.newFileName);

    // Read the file
    FILE* file = fopen(packet.filename, "rb");
    if (file == NULL) {
        perror("File open failed");
        return;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // Calculate the number of packets and size of the final packet
    packet.nPackets = file_size / packetSize + (file_size % packetSize != 0);
    packet.packetSize = (file_size % packetSize != 0) ? (file_size % packetSize) : packetSize;

    // Send the file information to the client
    ssize_t bytes_sent = sendto(server_socket, &packet, sizeof(struct XDATA), 0,
                                (struct sockaddr*)client_address, sizeof(struct sockaddr_in));
    if (bytes_sent == -1) {
        perror("Sendto failed");
        fclose(file);
        return;
    }

    printf("Sent file information to the client: %ld bytes\n", bytes_sent);

    // Read and send the file data
    int packetNo = 0;
    while (1) {
        // Read a packet of data from the file
        ssize_t bytes_read = fread(packet.data, sizeof(char), packetSize, file);

        // Set the packet number
        packet.packetNo = packetNo;

        // Send the packet to the client
        bytes_sent = sendto(server_socket, &packet, sizeof(struct XDATA), 0,
                            (struct sockaddr*)client_address, sizeof(struct sockaddr_in));
        if (bytes_sent == -1) {
            perror("Sendto failed");
            fclose(file);
            return;
        }

        printf("Sent packet %d: %ld bytes\n", packetNo, bytes_sent);

        if (bytes_read < packetSize) {
            break; // Reached the end of the file
        }

        packetNo++;
    }

    fclose(file);
}

void process_send2_command(const char* command, struct sockaddr_in* client_address, int server_socket, int packetSize) {
    struct XDATA packet;

    // Extract the filename and new filename from the command
    sscanf(command, "%*s %s %s", packet.filename, packet.newFileName);

    // Read the file
    FILE* file = fopen(packet.filename, "rb");
    if (file == NULL) {
        perror("File open failed");
        return;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // Calculate the number of packets and size of the final packet
    packet.nPackets = file_size / packetSize + (file_size % packetSize != 0);
    packet.packetSize = (file_size % packetSize != 0) ? (file_size % packetSize) : packetSize;

    // Send the file information to the client
    ssize_t bytes_sent = sendto(server_socket, &packet, sizeof(struct XDATA), 0,
                                (struct sockaddr*)client_address, sizeof(struct sockaddr_in));
    if (bytes_sent == -1) {
        perror("Sendto failed");
        fclose(file);
        return;
    }

    printf("Sent file information to the client: %ld bytes\n", bytes_sent);

    // Read and send the file data
    int packetNo = 0;
    while (1) {
        // Read a packet of data from the file
        ssize_t bytes_read = fread(packet.data, sizeof(char), packetSize, file);

        // Set the packet number
        packet.packetNo = packetNo;

        // Randomly choose whether to send the packet or not
        int send_packet = rand() % 2;

        if (send_packet) {
            // Send the packet to the client
            bytes_sent = sendto(server_socket, &packet, sizeof(struct XDATA), 0,
                                (struct sockaddr*)client_address, sizeof(struct sockaddr_in));
            if (bytes_sent == -1) {
                perror("Sendto failed");
                fclose(file);
                return;
            }

            printf("Sent packet %d: %ld bytes\n", packetNo, bytes_sent);
        }

        if (bytes_read < packetSize) {
            break; // Reached the end of the file
        }

        packetNo++;
    }

    fclose(file);
}

void process_send3_command(const char* command, struct sockaddr_in* client_address, int server_socket, int packetSize) {
    struct XDATA packet;

    // Extract the filename and packet number from the command
    sscanf(command, "%*s %s %hd", packet.filename, &packet.packetNo);

    // Read the file
    FILE* file = fopen(packet.filename, "rb");
    if (file == NULL) {
        perror("File open failed");
        return;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // Calculate the number of packets and size of the final packet
    packet.nPackets = file_size / packetSize + (file_size % packetSize != 0);
    packet.packetSize = (file_size % packetSize != 0) ? (file_size % packetSize) : packetSize;

    // Check if the specified packet exists
    if (packet.packetNo >= packet.nPackets) {
        printf("Packet %d does not exist\n", packet.packetNo);
        fclose(file);
        return;
    }

    // Seek to the specified packet position
    fseek(file, packet.packetNo * packetSize, SEEK_SET);

    // Read the packet data from the file
    ssize_t bytes_read = fread(packet.data, sizeof(char), packet.packetSize, file);

    // Send the packet to the client
    ssize_t bytes_sent = sendto(server_socket, &packet, sizeof(struct XDATA), 0,
                                (struct sockaddr*)client_address, sizeof(struct sockaddr_in));
    if (bytes_sent == -1) {
        perror("Sendto failed");
        fclose(file);
        return;
    }

    printf("Sent packet %d: %ld bytes\n", packet.packetNo, bytes_sent);

    fclose(file);
}

int main() {
    int server_socket;
    struct sockaddr_in server_address, client_address;
    char buffer[MAX_BUFFER_SIZE];
    socklen_t client_address_length;
    int packetSize = 10000;

    // Create a UDP socket
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(1234); // Replace with the actual server port number

    // Bind the socket to the server address
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Display connection information
    printf("Server is listening on %s:%d\n", inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));

    // Receive and process commands from clients
    while (1) {
        // Receive a command from a client
        client_address_length = sizeof(client_address);
        ssize_t bytes_received = recvfrom(server_socket, buffer, MAX_BUFFER_SIZE - 1, 0,
                                          (struct sockaddr*)&client_address, &client_address_length);
        if (bytes_received == -1) {
            perror("Recvfrom failed");
            continue;
        }

        buffer[bytes_received] = '\0';
        printf("Received command from client %s:%d: %s\n",
               inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), buffer);

        // Process the command
        if (strncmp(buffer, "Send1", 5) == 0) {
            process_send1_command(buffer, &client_address, server_socket, packetSize);
        } else if (strncmp(buffer, "Send2", 5) == 0) {
            process_send2_command(buffer, &client_address, server_socket, packetSize);
        } else if (strncmp(buffer, "Send3", 5) == 0) {
            process_send3_command(buffer, &client_address, server_socket, packetSize);
        }
    }

    // Close the server socket
    close(server_socket);

    return 0;
}
