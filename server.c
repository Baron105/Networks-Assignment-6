#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 65536

int main()
{
    int sockfd;
    struct sockaddr_ll sa;
    char buffer[BUFFER_SIZE];
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
    sa.sll_ifindex = if_nametoindex("eth0"); // Replace "eth0" with your interface name
    if (sa.sll_ifindex == 0)
    {
        perror("if_nametoindex");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the interface
    if (bind(sockfd, (struct sockaddr *)&sa, sizeof(struct sockaddr_ll)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // receive packets on this socket
    while (1)
    {
        int packet_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (packet_len == -1)
        {
            perror("recvfrom");
            exit(EXIT_FAILURE);
        }

        // check the ip header
        // check if destination ip address matches the ip address of the server
        struct iphdr *ip_header = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        struct in_addr dest_addr = {ip_header->daddr};

        char dest_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &dest_addr, dest_ip, INET_ADDRSTRLEN);

        if (strcmp(dest_ip, "127.0.0.1") != 0)
        {
            continue;
        }

        // check the protocol field in the ip header
        if (ip_header->protocol != 254)
        {
            continue;
        }

        char simDNSquery[1000] = {'\0'};

        // store the query in simDNSquery
        int start = sizeof(struct ethhdr) + sizeof(struct iphdr);
        int len = packet_len - start;
        int j = 0;
        for (int i = start; i < start + len; i++)
        {
            simDNSquery[j] = buffer[i];
            j++;
        }

        // printf("%s\n", simDNSquery);

        int qid = 0;

        // extract the query id
        for (int i = 0; i < 16; i++)
        {
            qid = qid * 2 + simDNSquery[i] - '0';
        }

        if (simDNSquery[16] == '1')
        {
            continue;
        }

        int n = 0;

        n = n * 2 + simDNSquery[17] - '0';
        n = n * 2 + simDNSquery[18] - '0';
        n = n * 2 + simDNSquery[19] - '0';
        n++;

        j = 20;

        long ip[n + 1];

        for (int t = 0; t < n; t++)
        {
            int l = 0;
            // extract the value in the query from j to j+3
            for (int i = 0; i < 4; i++)
            {
                l = l * 2 + simDNSquery[j] - '0';
                j++;
            }

            char domain[1000] = {'\0'};

            // extract the domain from the query from j to j+l
            for (int i = 0; i < l; i++)
            {
                domain[i] = simDNSquery[j];
                j++;
            }

            // get the ip of the domain
            struct hostent *host;
            struct in_addr **addr_list;
            int flag = 0;
            host = gethostbyname(domain);
            long ip_addr;
            if (host == NULL)
            {
                flag = 1;
            }
            else
            {
                addr_list = (struct in_addr **)host->h_addr_list;
                ip_addr = ntohl(addr_list[0]->s_addr);
            }

            // put the ip in the ip array, if the domain is not found put -1
            ip[t] = (flag ? -1 : ip_addr);
        }

        // creating the response packet
        char simDNSresponse[1000] = {'\0'};

        j = 15;
        // fill the first 16 char with the id
        while (j >= 0)
        {
            simDNSresponse[j] = qid % 2 + '0';
            qid /= 2;
            j--;
        }

        // it is a response message
        simDNSresponse[16] = '1';

        j = 19;
        int temp = n - 1;
        while (j > 16)
        {
            simDNSresponse[j] = '0' + temp % 2;
            temp /= 2;
            j--;
        }

        j = 20;
        for (int t = 0; t < n; t++)
        {
            if (ip[t] == -1)
            {
                simDNSresponse[j] = '0';
                j++;
                for (int e = 31; e >= 0; e--)
                {
                    simDNSresponse[j + e] = '1';
                }
                j += 32;
            }
            else
            {
                // first bit tells if it valid or not
                simDNSresponse[j] = '1';

                j++;

                // put the ip address
                for (int e = 31; e >= 0; e--)
                {
                    simDNSresponse[j + e] = '0' + ip[t] % 2;
                    ip[t] /= 2;
                }
                j += 32;
            }
        }

        memset(buffer, '\0', sizeof(buffer));

        // creating the ethernet header
        struct ethhdr *eth_header = (struct ethhdr *)buffer;
        memset(eth_header->h_dest, 0xff, ETH_ALEN);
        memcpy(eth_header->h_source, (unsigned char[]){0x3c, 0xa6, 0xf6, 0x40, 0xc3, 0x6d}, ETH_ALEN); // Source MAC address
        eth_header->h_proto = htons(ETH_P_IP);

        // creating the ip header
        ip_header = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        ip_header->ihl = 5;
        ip_header->version = 4;
        ip_header->tos = 0;
        ip_header->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen(simDNSresponse));
        ip_header->id = htons(0);
        ip_header->frag_off = 0;
        ip_header->ttl = 8;
        ip_header->protocol = 254;
        ip_header->saddr = inet_addr("127.0.0.1");
        ip_header->daddr = dest_addr.s_addr;

        // adding the sinDNSresponse to the buffer
        char *data = (char *)(buffer + sizeof(struct ethhdr) + sizeof(struct iphdr));
        strcpy(data, simDNSresponse);

        // send the response
        // printf("Sending response %s\n", simDNSresponse);
        len = sendto(sockfd, buffer, sizeof(struct ethhdr) + sizeof(struct iphdr) + strlen(simDNSresponse), 0, (struct sockaddr *)&sa, sizeof(struct sockaddr_ll));
        if (len == -1)
        {
            perror("sendto");
            exit(EXIT_FAILURE);
        }
        memset(buffer, '\0', sizeof(buffer));
        memset(simDNSresponse, '\0', sizeof(simDNSresponse));
    }
}
