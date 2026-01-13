#include "ft_traceroute.h"

#include <stdio.h>

static void	print_hostname_ip(char *hostname, char *ip)
{
	if (hostname[0]) {
		printf("%s (%s)  ", hostname, ip);
	} else {
		printf("%s (%s)  ", ip, ip);
	}
}

static void	print_single_line(int ttl, int first_valid_idx,
							  char router_ips[][INET_ADDRSTRLEN],
							  char hostnames[][NI_MAXHOST],
							  double *rtts)
{
	printf("%2d  ", ttl);
	print_hostname_ip(hostnames[first_valid_idx], router_ips[first_valid_idx]);
	for (int i = 0; i < NUM_PROBES; i++) {
		if (rtts[i] >= 0) {
			printf("%.3f ms", rtts[i]);
		} else {
			printf("*");
		}
		if (i < NUM_PROBES - 1)
			printf("  ");
	}
	printf("\n");
}

static void	print_multi_line(int ttl, char router_ips[][INET_ADDRSTRLEN],
							 char hostnames[][NI_MAXHOST], double *rtts)
{
	int first_printed = 0;
	for (int i = 0; i < NUM_PROBES; i++) {
		if (rtts[i] >= 0 && router_ips[i][0]) {
			if (!first_printed) {
				printf("%2d  ", ttl);
				first_printed = 1;
			} else {
				printf("     ");
			}
			print_hostname_ip(hostnames[i], router_ips[i]);
			printf("%.3f ms\n", rtts[i]);
		}
	}
}

void	print_results(int ttl, char router_ips[][INET_ADDRSTRLEN],
						  char hostnames[][NI_MAXHOST], double *rtts)
{
	int has_reply = 0;
	int first_valid_idx = -1;
	int all_same = 1;

	for (int i = 0; i < NUM_PROBES; i++) {
		if (router_ips[i][0] != 0) {
			has_reply = 1;
			if (first_valid_idx == -1) {
				first_valid_idx = i;
			} else if (ft_strcmp(router_ips[first_valid_idx], router_ips[i]) != 0) {
				all_same = 0;
			}
		}
	}

	if (!has_reply) {
		printf("%2d  * * *\n", ttl);
	} else if (all_same && first_valid_idx >= 0) {
		print_single_line(ttl, first_valid_idx, router_ips, hostnames, rtts);
	} else {
		print_multi_line(ttl, router_ips, hostnames, rtts);
	}
}
