#include "ft_ping.h"

ft_loop_status g_status = RUNNING;

void error_exit(char *msg)
{
	printf("%s\n", msg);
	exit(1);
}

int time_elapsed(struct timespec start, double time_ms)
{
	struct timespec end;
	clock_gettime(CLOCK_REALTIME, &end);

	long ns_diff = end.tv_nsec - start.tv_nsec;
	long s_diff = end.tv_sec - start.tv_sec;
	return ((double)s_diff * 1000.0 + (double)ns_diff / 1000000.0 > time_ms);
}

double calculate_time_taken(struct timeval start)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	long diff_usec = now.tv_usec - start.tv_usec;
	long diff_sec = now.tv_sec - start.tv_sec;

	double time = (double)(diff_sec * 1000) + (double)((double)diff_usec / 1000.0);
	return time;
}

void resolve_sender_host(struct ft_statistics *ft_stat, struct sockaddr_in *sender, socklen_t len)
{
	// we resolve only when sender ip change
	// max ip is 16 bytes long
	static char last_ip[16];
	char *ip_address = inet_ntoa(sender->sin_addr);

	if (memcmp(last_ip, ip_address, 16) == 0)
		return;
	strncpy(last_ip, ip_address, 16);
	// hostname is 253 bytes max
	char hostname[254];
	bzero(hostname, 254);

	int error = getnameinfo((struct sockaddr *)sender, len, hostname, 254, 0, 0, 0);
	if (error == 0 && strcmp(ip_address, ft_stat->request) != 0 && strcmp(ip_address, hostname) != 0 && hostname[0] != 0)
		sprintf(ft_stat->pretty_hostname, "%s (%s)", hostname, ip_address);
	else
		sprintf(ft_stat->pretty_hostname, "%s ", ip_address);
}

int main(int argc, char **argv)
{
	signal(SIGINT, sigint_handler);

	struct ft_socket ft_socket;
	struct ft_statistics ft_stat;
	struct ft_flag flag;

	flag = get_flag(argc, argv);
	init_socket(&ft_socket, flag.arg, ft_stat.ip_string);
	if (set_ttl(ft_socket.socket_fd, flag.ttl))
	{
		printf("Error setting ttl\n");
		close(ft_socket.socket_fd);
		return 1;
	}
	ft_stat.request = flag.arg;
	if (init_ft_stat(&ft_stat) == ERROR)
	{
		close(ft_socket.socket_fd);
		error_exit("Malloc failed\n");
	}

	struct timespec last_ping;
	gettimeofday(&ft_stat.start, 0);
	clock_gettime(CLOCK_REALTIME, &last_ping);
	// we dont change the data so it will always be the same size
	printf("PING %s (%s) 56 bytes of data.\n", ft_stat.request, ft_stat.ip_string);
	// we send the first packet right away
	if (send_packet(ft_socket) == ERROR)
	{
		free(ft_stat.packet_stat);
		close(ft_socket.socket_fd);
		return 1;
	}
	ft_stat.sent_packet++;
	while (g_status == RUNNING)
	{
		if (time_elapsed(last_ping, flag.interval_ms) && ft_stat.sent_packet < flag.max_count)
		{
			clock_gettime(CLOCK_REALTIME, &last_ping);
			if (send_packet(ft_socket) == ERROR)
			{
				free(ft_stat.packet_stat);
				return 1;
			}
			ft_stat.sent_packet++;
		}
		// We cant use select / poll because of the subject so we read the socket in non blocking mode ...
		// not very efficient but it works
		receive_packet(ft_socket, &ft_stat, flag);
		if (ft_stat.nb_packet_received >= flag.max_count)
			break;
		// timeout ==> default is 10 s
		if (time_elapsed(last_ping, 10 * 1000))
			break;
	}
	pretty_print_stat(ft_stat);
	free(ft_stat.packet_stat);
	close(ft_socket.socket_fd);
	return 0;
}
