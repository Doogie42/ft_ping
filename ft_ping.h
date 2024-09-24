#pragma once

#include <sys/types.h>
#include <netdb.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/ip.h>

#define BUFFER_SIZE 1000

typedef enum ft_error
{
	ERROR = -1,
	SUCCESS = 0
} ft_error;

typedef enum ft_host_type
{
	IP,
	FULL_NAME
} ft_host_type;

typedef enum ft_packet_status
{
	not_received,
	error,
	received
} ft_packet_status;

typedef enum ft_loop_status
{
	RUNNING,
	STOP
} ft_loop_status;

typedef struct ft_icmp
{
	// ICMP HEADER
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint16_t id;
	uint16_t seq;
	// BODY OF ICMP
	struct timeval timestamp;
	char data[40];
} t_ft_icmp;

struct ft_flag
{
	long long max_count;
	bool verbose;
	unsigned int ttl;
	char *arg;
	double interval_ms;
};

struct ft_socket
{
	int socket_fd;
	struct sockaddr_in dest;
};

struct ft_packet_statistics
{
	ft_packet_status received;
	double time_taken_ms;
	unsigned int seq;
	size_t size_bytes_received;
	uint8_t icmp_type;
	unsigned int ttl;
};

struct ft_statistics
{
	struct timeval start;
	unsigned int sent_packet;
	unsigned int nb_packet_received;
	unsigned int packet_stat_size;
	struct ft_packet_statistics *packet_stat;
	char pretty_hostname[275];
	char hostname[254];
	char ip_string[16];
	char *request;
};

struct ft_flag get_flag(int argc, char **argv);
void pretty_print_stat(struct ft_statistics ft_stat);
void pretty_print(struct ft_packet_statistics packet_stat, char *serv_name);
void print_debug(char *buffer_recv);

void sigint_handler();
ft_error set_ttl(int socket_fd, int ttl);
ft_error init_ft_stat(struct ft_statistics *ft_stat);
void init_socket(struct ft_socket *ft_socket, char *target, char ip_string[16]);
int open_socket(char *address, struct sockaddr_in *dest, char ip_address_string[16]);
void error_exit(char *msg);
double calculate_time_taken(struct timeval start);
void receive_packet(struct ft_socket ft_socket, struct ft_statistics *ft_stat, struct ft_flag flag);

ft_error send_packet(struct ft_socket ft_socket);
void resolve_sender_host(struct ft_statistics *ft_stat, struct sockaddr_in *sender, socklen_t len);