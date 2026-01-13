#include "ft_traceroute.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

static double	calculate_rtt(struct timeval *initial_time, struct timeval *received_time)
{
	return (received_time->tv_sec - initial_time->tv_sec) * 1000.0 +
		   (received_time->tv_usec - initial_time->tv_usec) / 1000.0;
}

static void	store_probe_info(struct sockaddr_in *addr, int probe,
								char router_ips[][INET_ADDRSTRLEN],
								char hostnames[][NI_MAXHOST])
{
	inet_ntop(AF_INET, &addr->sin_addr, router_ips[probe], INET_ADDRSTRLEN);
	getnameinfo((struct sockaddr *)addr, sizeof(*addr),
			   hostnames[probe], NI_MAXHOST, NULL, 0, 0);
}

int	run_probe(int udp_sock, int icmp_sock, struct sockaddr_in *dst,
				int probe, double *rtts,
				char router_ips[][INET_ADDRSTRLEN],
				char hostnames[][NI_MAXHOST],
				int *reached_destination)
{
	struct timeval	initial_time;
	if (gettimeofday(&initial_time, NULL) != 0) {
		fprintf(stderr, "gettimeofday error: %s\n", strerror(errno));
		return (-1);
	}
	
	if (sendto(udp_sock, NULL, 0, 0, (struct sockaddr *)dst, sizeof(struct sockaddr_in)) < 0) {
		fprintf(stderr, "sendto error: %s\n", strerror(errno));
		return (-1);
	}

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(icmp_sock, &readfds);
	struct timeval timeout = {TIMEOUT_SEC, 0};

	int ret = select(icmp_sock + 1, &readfds, NULL, NULL, &timeout);
	if (ret == -1) {
		fprintf(stderr, "select error: %s\n", strerror(errno));
		return (-1);
	}

	if (ret > 0 && FD_ISSET(icmp_sock, &readfds)) {
		char buffer[1024];
		struct sockaddr_in recv_addr;
		socklen_t recv_addrlen = sizeof(recv_addr);
		int recv_len = recvfrom(icmp_sock, buffer, sizeof(buffer), 0,
								(struct sockaddr *)&recv_addr, &recv_addrlen);
		if (recv_len >= 0) {
			struct ip *ip_hdr = (struct ip *)buffer;
			int ip_hdr_len = ip_hdr->ip_hl * 4;
			struct icmphdr *icmp_hdr = (struct icmphdr *)(buffer + ip_hdr_len);
			if (icmp_hdr->type == ICMP_TIME_EXCEEDED || icmp_hdr->type == ICMP_DEST_UNREACH) {
				struct timeval recv_time;
				if (gettimeofday(&recv_time, NULL) != 0) {
					fprintf(stderr, "gettimeofday error: %s\n", strerror(errno));
					return (-1);
				}
				rtts[probe] = calculate_rtt(&initial_time, &recv_time);
				store_probe_info(&recv_addr, probe, router_ips, hostnames);
				if (icmp_hdr->type == ICMP_DEST_UNREACH && icmp_hdr->code == ICMP_PORT_UNREACH)
					*reached_destination = 1;
			}
		}
	}
	return (0);
}
