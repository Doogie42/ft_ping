#include "ft_ping.h"

extern ft_loop_status g_status;

void sigint_handler()
{
	g_status = STOP;
}

ft_error set_ttl(int socket_fd, int ttl)
{
	if (setsockopt(socket_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) == -1)
		return ERROR;
	return SUCCESS;
}

ft_error init_ft_stat(struct ft_statistics *ft_stat)
{
	ft_stat->packet_stat_size = 4;
	ft_stat->sent_packet = 0;
	ft_stat->nb_packet_received = 0;
	ft_stat->packet_stat = malloc(ft_stat->packet_stat_size * sizeof(struct ft_packet_statistics));
	if (ft_stat->packet_stat == NULL)
		return ERROR;
	for (size_t i = 0; i < ft_stat->packet_stat_size; i++)
	{
		ft_stat->packet_stat[i].received = not_received;
	}
	return SUCCESS;
}

void init_socket(struct ft_socket *ft_socket, char *target, char ip_string[16])
{
	bzero(&ft_socket->dest, sizeof(ft_socket->dest));
	ft_socket->dest.sin_family = AF_INET;
	ft_socket->dest.sin_port = AF_INET;
	ft_socket->socket_fd = open_socket(target, &ft_socket->dest, ip_string);
}

int open_socket(char *address, struct sockaddr_in *dest, char ip_address_string[16])
{
	struct addrinfo hints;
	struct addrinfo *results, *rp;
	int socket_fd;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	if (getaddrinfo(address, NULL, &hints, &results) != 0)
	{
		printf("ping : %s: Temporary failure in name resolution\n", address);
		exit(1);
	}
	if (results == NULL)
		error_exit("NO info found for host");

	for (rp = results; rp != NULL; rp = results->ai_next)
	{
		socket_fd = socket(rp->ai_family, rp->ai_socktype,
						   IPPROTO_ICMP);
		if (socket_fd == -1)
		{
			continue;
		}
		else
		{
			break;
		}
	}
	if (socket_fd == -1)
	{
		freeaddrinfo(results);
		perror("Error:");
		error_exit("Couldn't open socket");
	}
	char *ip_address = inet_ntoa(((struct sockaddr_in *)rp->ai_addr)->sin_addr);
	inet_pton(AF_INET, ip_address, &(dest->sin_addr.s_addr));
	strncpy(ip_address_string, ip_address, 16);
	freeaddrinfo(results);
	return socket_fd;
}