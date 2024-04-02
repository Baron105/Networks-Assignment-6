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

    // Construct the Ethernet header
    struct ethhdr *eth = (struct ethhdr *)buffer;
    memset(eth->h_dest, 0xFF, ETH_ALEN); // Broadcast MAC address
    memcpy(eth->h_source, (unsigned char[]){0x00, 0x08, 0xA1, 0x8E, 0xE4, 0x52}, ETH_ALEN); // Source MAC address
    eth->h_proto = htons(ETH_P_IP); // EtherType for IP

    // Construct the IP header
    struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen("Hello"));
    ip->id = htons(54321);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_TCP;
    ip->check = 0; // Will be filled in by the kernel
    ip->saddr = inet_addr("192.168.1.1"); // Source IP address
    ip->daddr = inet_addr("192.168.1.2"); // Destination IP address

    // Construct the TCP header
    struct tcphdr *tcp = (struct tcphdr *)(buffer + sizeof(struct ethhdr) + sizeof(struct iphdr));
    tcp->source = htons(12345); // Source port
    tcp->dest = htons(12345); // Destination port
    tcp->seq = htonl(1); // Sequence number
    tcp->ack_seq = 0; // Acknowledgment number
    tcp->doff = 5; // Data offset
    tcp->fin = 0;
    tcp->syn = 0;
    tcp->rst = 0;
    tcp->psh = 0;
    tcp->ack = 0;
    tcp->urg = 0;
    tcp->window = htons(5840); // Window size
    tcp->check = 0; // Will be filled in by the kernel
    tcp->urg_ptr = 0;

    // Add the "Hello" message as payload
    char *data = (char *)(buffer + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr));
    strcpy(data, "Hello");

    // Send the packet
    packet_len = sendto(sockfd, buffer, sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen("Hello"), 0, (struct sockaddr *)&sa, sizeof(struct sockaddr_ll));
    if (packet_len == -1) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }

    printf("Sent packet of length %zd\n", packet_len);

    close(sockfd);
    return 0;
}
