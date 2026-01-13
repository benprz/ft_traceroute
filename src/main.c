#include "ft_traceroute.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

static int	resolve_addr_infos(const char *host, struct addrinfo **addr_infos,
							struct sockaddr_in **addr, char addr_ip[INET_ADDRSTRLEN])
{
	struct addrinfo hints = {0};

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	if (getaddrinfo(host, NULL, &hints, addr_infos) != 0) {
		fprintf(stderr, "traceroute: unknown host %s\n", host);
		return (-1);
	}
	if ((*addr_infos)->ai_addrlen != sizeof(struct sockaddr_in) || (*addr_infos)->ai_family != AF_INET) {
		fprintf(stderr, "traceroute: invalid address length or family\n");
		return (-1);
	}
	*addr = (struct sockaddr_in *)(*addr_infos)->ai_addr;
	inet_ntop(AF_INET, &(*addr)->sin_addr, addr_ip, INET_ADDRSTRLEN);
	return (0);
}

static int	setup_sockets(struct addrinfo *addr_infos, int *udp_sock, int *icmp_sock)
{
	*udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (*udp_sock < 0)
		return (fprintf(stderr, "socket error: %s\n", strerror(errno)), -1);
	*icmp_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (*icmp_sock < 0) {
		fprintf(stderr, "socket error (ICMP): %s\n", strerror(errno));
		close(*udp_sock);
		return (-1);
	}

	struct sockaddr_in srcaddr = {
		.sin_family = AF_INET,
		.sin_port = htons(0),
		.sin_addr.s_addr = INADDR_ANY
	};
	if (bind(*udp_sock, (struct sockaddr *)&srcaddr, sizeof(srcaddr)) < 0) {
		fprintf(stderr, "bind error: %s\n", strerror(errno));
		close(*udp_sock);
		close(*icmp_sock);
		return (-1);
	}

	int val = IP_PMTUDISC_DONT;
	setsockopt(*udp_sock, SOL_IP, IP_MTU_DISCOVER, &val, sizeof(val));

	((struct sockaddr_in *)addr_infos->ai_addr)->sin_port = htons(PORT);
	return (0);
}

int main(int argc, char *argv[])
{
	if (argc < 2 || argc > 3 || (argc >= 2 && ft_strcmp(argv[1], "-h") == 0)) {
		fprintf(stderr, "Usage: %s <host>\n", argv[0]);
		fprintf(stderr, "Options:\n  -h\tShow this help message\n");
		return 1;
	}

	if (getuid() != 0) {
		fprintf(stderr, "Error: This program requires root privileges.\n");
		return 1;
	}

	struct addrinfo *addr_infos = NULL;
	struct sockaddr_in *addr = NULL;
	char addr_ip[INET_ADDRSTRLEN];

	if (resolve_addr_infos(argv[1], &addr_infos, &addr, addr_ip) != 0) {
		if (addr_infos)
			freeaddrinfo(addr_infos);
		return 1;
	}
	printf("traceroute to %s (%s), %d hops max\n", argv[1], addr_ip, MAX_HOPS);

	int udp_sock, icmp_sock;
	if (setup_sockets(addr_infos, &udp_sock, &icmp_sock) != 0) {
		if (addr_infos)
			freeaddrinfo(addr_infos);
		return 1;
	}

	for (int ttl = 1; ttl <= MAX_HOPS; ttl++) {
		if (setsockopt(udp_sock, SOL_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
			fprintf(stderr, "setsockopt error (set TTL): %s\n", strerror(errno));
			break;
		}
		double rtts[NUM_PROBES];
		int reached_destination = 0;
		char router_ips[NUM_PROBES][INET_ADDRSTRLEN] = {0};
		char hostnames[NUM_PROBES][NI_MAXHOST] = {0};
		ft_memset(rtts, -1, sizeof(rtts));

		for (int probe = 0; probe < NUM_PROBES; probe++)
			run_probe(udp_sock, icmp_sock, addr, probe,
					  rtts, router_ips, hostnames, &reached_destination);

		print_results(ttl, router_ips, hostnames, rtts);

		if (reached_destination) {
			close(udp_sock);
			close(icmp_sock);
			freeaddrinfo(addr_infos);
			return 0;
		}
	}
	close(udp_sock);
	close(icmp_sock);
	freeaddrinfo(addr_infos);
	return 0;
}
