#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

int main() {
    int sockfd;
    struct sockaddr_in dest;
    char payload[64] = "Hello, ICMP via SOCK_DGRAM!";
    char reply[128];

    // Create a SOCK_DGRAM socket for ICMP
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // Configure destination (e.g., 8.8.8.8)
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr("8.8.8.8");

    // Send ICMP Echo Request
    if (sendto(sockfd, payload, sizeof(payload), 0, 
              (struct sockaddr*)&dest, sizeof(dest)) < 0) {
        perror("sendto");
        close(sockfd);
        exit(1);
    }

    printf("ICMP Echo Request sent. Waiting for reply...\n");

    // Receive ICMP Echo Reply
    socklen_t dest_len = sizeof(dest);
    ssize_t reply_len = recvfrom(sockfd, reply, sizeof(reply), 0, 
                                (struct sockaddr*)&dest, &dest_len);
    if (reply_len < 0) {
        perror("recvfrom");
        close(sockfd);
        exit(1);
    }

    printf("Received ICMP Echo Reply from %s: %.*s\n",
           inet_ntoa(dest.sin_addr), (int)reply_len, reply);

    close(sockfd);
    return 0;
}
