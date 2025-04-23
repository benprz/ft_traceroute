#ifndef TRACEROUTE_H
#define TRACEROUTE_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#define PACKET_SIZE IP_HEADER_SIZE + ICMP_PACKET_SIZE
#define IP_HEADER_SIZE sizeof(struct ip)
#define ICMP_PACKET_SIZE sizeof(struct icmphdr) + ICMP_PAYLOAD_SIZE
#define ICMP_PAYLOAD_SIZE sizeof(struct timeval) + ICMP_PAYLOAD_CHUNK_SIZE

#define ICMP_PAYLOAD_CHUNK "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f" \
                           "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f" \
                           "\x20\x21\x22\x23\x24\x25\x26\x27"
#define ICMP_PAYLOAD_CHUNK_SIZE 40


#endif
