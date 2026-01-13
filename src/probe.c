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

int	run_probe(int udp_sock, int icmp_sock, struct sockaddr_in *addr,
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

	if (sendto(udp_sock, NULL, 0, 0, (struct sockaddr *)addr, sizeof(struct sockaddr_in)) < 0) {
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
		unsigned char buffer[1024];
		struct sockaddr_in recv_addr;
		socklen_t recv_addrlen = sizeof(recv_addr);
		int recv_len = recvfrom(icmp_sock, buffer, sizeof(buffer), 0,
								(struct sockaddr *)&recv_addr, &recv_addrlen);
		if (recv_len >= 0) {
			int ip_hdr_len = ((struct ip *)buffer)->ip_hl * 4; // 32 bits words to bytes (usually 5 words)
			struct icmphdr *icmp_hdr = (struct icmphdr *)(buffer + ip_hdr_len);
			if (icmp_hdr->type == ICMP_TIME_EXCEEDED || icmp_hdr->type == ICMP_DEST_UNREACH) {
				// A part of the UDP packet is embedded in the ICMP error message
				// Only process ICMP messages that are related to our probe requests
				struct ip *embedded_ip = (struct ip *)(buffer + ip_hdr_len + sizeof(struct icmphdr));

				if ((embedded_ip->ip_p != IPPROTO_UDP) || \
					(embedded_ip->ip_dst.s_addr != addr->sin_addr.s_addr))
					return 0;

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
