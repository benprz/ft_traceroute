#include "ft_traceroute.h"

#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <linux/icmp.h>
#include <unistd.h>
#include <linux/if_ether.h>

#define MAX_HOPS 30

// int parse_host(char *host)
// {
// 	struct addrinfo hints = {0};
// 	hints.ai_family = AF_INET;
// 	hints.ai_socktype = SOCK_STREAM;
// 	hints.ai_protocol = IPPROTO_TCP;

// 	if (getaddrinfo(host, NULL, &hints, &g_ping.host) != 0)
// 	{
// 		error(EXIT_FAILURE, 0, "unknown host");
// 	}

// 	g_ping.hostarg = host;

// 	struct sockaddr_in *ipv4 = (struct sockaddr_in *)g_ping.host->ai_addr;
// 	void *addr = &(ipv4->sin_addr);
// 	inet_ntop(g_ping.host->ai_family, addr, g_ping.hostip, INET_ADDRSTRLEN);

// 	return 0;
// }

int main() {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
    	fprintf(stderr, "socket error: %s\n", strerror(errno));
		return -1;
	}


    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(80),
        .sin_addr.s_addr = inet_addr("127.0.0.1")
    };


    unsigned char packet[PACKET_SIZE];

   	// icmp header template for echo request
	struct icmphdr icmp_hdr = {
		.type = ICMP_ECHO,
		.code = 0,
		.un.echo.id = getpid(),
	};

	bzero(packet, ICMP_PACKET_SIZE);
	icmp_hdr.un.echo.sequence = htons(0);
	memcpy(packet, &icmp_hdr, sizeof(struct icmphdr));
	gettimeofday((struct timeval *)(packet + sizeof(struct icmphdr)), NULL);
	memcpy(packet + sizeof(struct icmphdr) + sizeof(struct timeval), ICMP_PAYLOAD_CHUNK, ICMP_PAYLOAD_CHUNK_SIZE);

	uint16_t checksum = calculate_checksum((uint16_t *)packet, ICMP_PACKET_SIZE);
	packet[2] = checksum & 0xff; // big endian: second byte
	packet[3] = checksum >> 8; // big endian: first byte

    for (int i = 1; i < MAX_HOPS; i++) {
    	setsockopt(sockfd, IPPROTO_IP, IP_TTL, &i, sizeof(i));
     	sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));


      	char buffer[1024] = { 0 };
      	recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
       	struct ip* ip_header = (struct ip*)(buffer + sizeof(struct ethhdr));
        struct in_addr* recv_addr = (struct in_addr*)(&ip_header->ip_src);
        printf("%s (%s)  %f ms\n", recv_addr->)

    }

    return 0;
}
