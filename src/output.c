#include "ft_traceroute.h"

#include <stdio.h>
#include <stdbool.h>

static void	print_hostname_ip(char *hostname, char *ip)
{
	if (hostname[0]) {
		printf("%s (%s)  ", hostname, ip);
	} else {
		printf("%s (%s)  ", ip, ip);
	}
}

static void	print_single_line(int ttl, int valid_probe_index,
							  char router_ips[][INET_ADDRSTRLEN],
							  char hostnames[][NI_MAXHOST],
							  double *rtts)
{
	printf("%2d  ", ttl);
	print_hostname_ip(hostnames[valid_probe_index], router_ips[valid_probe_index]);
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
	bool first_printed_probe = true;
	for (int i = 0; i < NUM_PROBES; i++) {
		if (router_ips[i][0]) {
			if (first_printed_probe) {
				printf("%2d  ", ttl);
				first_printed_probe = false;
			} else {
				printf("     ");
			}
			print_hostname_ip(hostnames[i], router_ips[i]);
			printf("%.3f ms\n", rtts[i]);
		}
	}
}

void	print_current_hop_results(int ttl, char router_ips[][INET_ADDRSTRLEN],
						  char hostnames[][NI_MAXHOST], double *rtts)
{
	bool has_a_reply = false;
	bool all_replies_from_same_ip = true;
	int valid_probe_index = -1;

	for (int i = 0; i < NUM_PROBES; i++) {
		if (router_ips[i][0] != 0) {
			has_a_reply = true;
			if (valid_probe_index == -1) {
				valid_probe_index = i;
			} 
			// check if all replies are from the same IP
			else if (ft_strcmp(router_ips[valid_probe_index], router_ips[i]) != 0) {
				all_replies_from_same_ip = false;
			}
		}
	}

	if (!has_a_reply) {
		printf("%2d  * * *\n", ttl);
	} else if (all_replies_from_same_ip && valid_probe_index >= 0) {
		print_single_line(ttl, valid_probe_index, router_ips, hostnames, rtts);
	} else {
		print_multi_line(ttl, router_ips, hostnames, rtts);
	}
}
