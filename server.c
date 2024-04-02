#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 65536

int main() {
    int sockfd;
    struct sockaddr_ll sa;
    char buffer[BUFFER_SIZE];
    ssize_t packet_len;

    // Create a raw socket
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set up the sockaddr_ll structure
    memset(&sa, 0, sizeof(struct sockaddr_ll));
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_ALL);
    sa.sll_ifindex = if_nametoindex("eth0"); // Replace "eth0" with your interface name
    if (sa.sll_ifindex == 0) {
        perror("if_nametoindex");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the interface
    if (bind(sockfd, (struct sockaddr *)&sa, sizeof(struct sockaddr_ll)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Receive packets
    while (1) {
        packet_len = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (packet_len == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
        }

        // Print the payload of the received packet
        printf("Received packet: %ld\n", strlen(buffer));
    }

    close(sockfd);
    return 0;
}
