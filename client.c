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

typedef struct page_table_entry
{
    char query[1000];
    int id;
    int tries;
} page_table_entry;

page_table_entry page_table[20];

int main()
{

    memset(page_table, 0, sizeof(page_table));
    int sockfd;
    struct sockaddr_ll sa;
    char sendbuffer[BUFFER_SIZE] = {'\0'};
    char recvbuffer[BUFFER_SIZE] = {'\0'};
    ssize_t packet_len;

    // Create a raw socket
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set up the sockaddr_ll structure
    memset(&sa, 0, sizeof(struct sockaddr_ll));
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_ALL);
    sa.sll_ifindex = if_nametoindex("eth0");
    if (sa.sll_ifindex == 0)
    {
        perror("if_nametoindex");
        exit(EXIT_FAILURE);
    }

    // Construct the Ethernet header
    // struct ethhdr *eth = (struct ethhdr *)buffer;
    // memset(eth->h_dest, 0xFF, ETH_ALEN); // Broadcast MAC address
    // memcpy(eth->h_source, (unsigned char[]){0x00, 0x08, 0xA1, 0x8E, 0xE4, 0x52}, ETH_ALEN); // Source MAC address
    // eth->h_proto = htons(ETH_P_IP); // EtherType for IP

    // Construct the IP header
    // struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
    // ip->ihl = 5;
    // ip->version = 4;
    // ip->tos = 0;
    // ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen("Hello"));
    // ip->id = htons(54321);
    // ip->frag_off = 0;
    // ip->ttl = 64;
    // ip->protocol = IPPROTO_TCP;
    // ip->check = 0; // Will be filled in by the kernel
    // ip->saddr = inet_addr("192.168.1.1"); // Source IP address
    // ip->daddr = inet_addr("192.168.1.2"); // Destination IP address

    // Construct the TCP header
    // struct tcphdr *tcp = (struct tcphdr *)(buffer + sizeof(struct ethhdr) + sizeof(struct iphdr));
    // tcp->source = htons(12345); // Source port
    // tcp->dest = htons(12345); // Destination port
    // tcp->seq = htonl(1); // Sequence number
    // tcp->ack_seq = 0; // Acknowledgment number
    // tcp->doff = 5; // Data offset
    // tcp->fin = 0;
    // tcp->syn = 0;
    // tcp->rst = 0;
    // tcp->psh = 0;
    // tcp->ack = 0;
    // tcp->urg = 0;
    // tcp->window = htons(5840); // Window size
    // tcp->check = 0; // Will be filled in by the kernel
    // tcp->urg_ptr = 0;

    // Add the "Hello" message as payload
    // char *data = (char *)(buffer + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr));
    // strcpy(data, "Hello");

    // // Send the packet
    // packet_len = sendto(sockfd, buffer, sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen("Hello"), 0, (struct sockaddr *)&sa, sizeof(struct sockaddr_ll));
    // if (packet_len == -1) {
    //     perror("sendto");
    //     exit(EXIT_FAILURE);
    // }

    // printf("Sent packet of length %zd\n", packet_len);

    // close(sockfd);

    int id = 1;

    fd_set fd;
    FD_ZERO(&fd);
    FD_SET(sockfd, &fd);
    FD_SET(stdin, &fd);

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    while (1)
    {
        fd_set read_fd = fd;
        int ret = select(sockfd + 1, &read_fd, NULL, NULL, &tv);

        if (ret == -1)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }
        else if (ret == 0)
        {
            // timeout
        }

        else
        {
            if (FD_ISSET(stdin, &read_fd))
            {
                // asking user for query input
                printf("Enter the query: (getIP N <domain-1> <domain-2> <domain-3> … <domain-N>) or EXIT\n");
                char query[1000];

                scanf("%[^\n]", query);

                if (strncmp(query, "EXIT", 4) == 0)
                {
                    // close the socket and exit
                    close(sockfd);
                    return 0;
                }

                // checking if the query is valid
                if (strncmp(query, "getIP", 5) != 0)
                {
                    printf("Invalid format\n");
                    continue;
                }

                // checking value of N
                int n;
                sscanf(query, "getIP %d", &n);

                if (n > 8)
                {
                    printf("N should be less than or equal to 8\n");
                    continue;
                }

                // checking if actual number of domains are equal to N
                int count = 0;
                for (int i = 0; i < strlen(query); i++)
                {
                    if (query[i] == ' ')
                    {
                        count++;
                    }
                }

                if (count != n)
                {
                    printf("Number of domains should be equal to N\n");
                    continue;
                }

                // check if the has only alphanumeric characters and dots and hyphens
                int flag = 0;
                for (int i = 0; i < strlen(query); i++)
                {
                    if ((query[i] >= 'a' && query[i] <= 'z') || (query[i] >= 'A' && query[i] <= 'Z') || (query[i] >= '0' && query[i] <= '9') || query[i] == '.' || query[i] == '-' || query[i] == ' ')
                    {
                        if (query[i] == '-')
                        {
                            if (i == 0)
                            {
                                flag = 1;
                                break;
                            }
                            else
                            {
                                if (query[i - 1] == '-' || query[i - 1] == ' ')
                                {
                                    flag = 1;
                                    break;
                                }
                            }
                        }
                        continue;
                    }
                    else
                    {
                        flag = 1;
                        break;
                    }
                }

                if (flag == 1)
                {
                    printf("Invalid domain name\n");
                    continue;
                }

                int j = 15;
                // fill the first 16 char with the id
                while (j >= 0)
                {
                    simDNSquery[j] = id % 2 + '0';
                    id /= 2;
                    j--;
                }
                // it is a query message
                simDNSquery[16] = '0';

                j = 19;
                int temp = n;
                while (j > 16)
                {
                    simDNSquery[j] = '0' + temp % 2;
                    temp /= 2;
                    j--;
                }
                j = 20;
                // find length of each domain and the domain
                int t = 7;
                while (n--)
                {
                    j--;
                    while (query[t] == ' ' || query[t] == '\0')
                    {
                        t++;
                    }
                    int len = 0;
                    int start = t;

                    while (query[t] != ' ' && query[t] != '\0')
                    {
                        len++;
                        t++;
                    }

                    // put the len in the 4 bits from j+1 to j+4
                    int temp = len;
                    int m = 4;
                    while (m)
                    {
                        simDNSquery[j + m] = '0' + temp % 2;
                        temp /= 2;
                        m--;
                    }

                    j = j + 5;

                    // put the domain in the message
                    for (int i = start; i < start + len; i++)
                    {
                        simDNSquery[j] = query[i];
                        j++;
                    }
                }

                // creating the Ethernet header
                struct ethhdr *eth = (struct ethhdr *)sendbuffer;
                memset(eth->h_dest, 0xFF, ETH_ALEN);                                                    // Broadcast MAC address
                memcpy(eth->h_source, (unsigned char[]){0x3c, 0xa6, 0xf6, 0x40, 0xc3, 0x6d}, ETH_ALEN); // Source MAC address
                eth->h_proto = htons(ETH_P_IP);                                                         // EtherType for IP

                // creating the IP header
                struct iphdr *ip = (struct iphdr *)(sendbuffer + sizeof(struct ethhdr));
                ip->ihl = 5;
                ip->version = 4;
                ip->tos = 0;
                ip->tot_len = htons(sizeof(struct iphdr) + strlen(simDNSquery));
                ip->id = htons(54321);
                ip->frag_off = 0;
                ip->ttl = 8;
                ip->protocol = 254;
                ip->check = 0;
                ip->saddr = inet_addr("127.0.0.1");
                ip->daddr = inet_addr("127.0.0.1");

                // adding the simDNSquery to the buffer
                char *data = (char *)(sendbuffer + sizeof(struct ethhdr) + sizeof(struct iphdr));
                strcpy(data, simDNSquery);

                // sending the packet
                int len = sendto(sockfd, sendbuffer, sizeof(struct ethhdr) + sizeof(struct iphdr) + strlen(simDNSquery), 0, (struct sockaddr *)&sa, sizeof(struct sockaddr_ll));

                if (len == -1)
                {
                    perror("sendto");
                    exit(EXIT_FAILURE);
                }

                printf("Sent packet of length %d\n", len);
                id++;

                // add it in the page query table
            }

            if (FD_ISSET(sockfd, &read_fd))
            {
                // received a reponse
            }
        }
    }
}
