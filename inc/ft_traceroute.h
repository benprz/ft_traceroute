#ifndef TRACEROUTE_H
#define TRACEROUTE_H

#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/select.h>

#define MAX_HOPS 30
#define TIMEOUT_SEC 5
#define PORT 33435
#define NUM_PROBES 3

int	 run_probe(int udp_sock, int icmp_sock, struct sockaddr_in *dst,
				int probe, double *rtts,
				char router_ips[][INET_ADDRSTRLEN],
				char hostnames[][NI_MAXHOST],
				int *reached_destination);

void print_results(int ttl, char router_ips[][INET_ADDRSTRLEN],
				   char hostnames[][NI_MAXHOST], double *rtts);

// utils functions
int	ft_strcmp(const char *s1, const char *s2);
void *ft_memset(void *ptr, const unsigned int c, size_t len);

#endif
