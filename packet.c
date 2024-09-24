#include "ft_ping.h"

// http://www.faqs.org/rfcs/rfc1071.html
static uint16_t calculate_icmp_checksum(t_ft_icmp icmp_header)
{
	icmp_header.checksum = 0;
	unsigned long checksum = 0;
	unsigned short *addr = (unsigned short *) &icmp_header;
	for (size_t i = 0; i < sizeof(icmp_header); i += 2){
		checksum += *(unsigned short *) addr;
		addr ++;
	}
	while (checksum > UINT16_MAX)
		checksum = checksum - UINT16_MAX ;
	// while (checksum>>16)
	// 	checksum = (checksum & 0xffff) + (checksum >> 16);
	checksum = ~checksum;
	return checksum;
}


static t_ft_icmp create_echo_packet(void)
{
	static int seq = 0;
	t_ft_icmp icmp_packet_sent;
	icmp_packet_sent.type = ICMP_ECHO;
	icmp_packet_sent.code = 0;
	icmp_packet_sent.id = getpid();
	icmp_packet_sent.seq = seq;
	seq ++;
	gettimeofday(&icmp_packet_sent.timestamp, NULL);
	for (int k = 0; k < 40; ++k){
		icmp_packet_sent.data[k] = 16 + k;
	}
	icmp_packet_sent.checksum = calculate_icmp_checksum(icmp_packet_sent);
	return icmp_packet_sent;
}


// 8th byte of ip packet
// Assume packet is valid 
static uint8_t extract_ttl(char *raw_packet)
{
	return (uint8_t) raw_packet[8];
}

static ft_error realloc_packet_stat(struct ft_statistics *ft_stat)
{
	size_t old_size = ft_stat->packet_stat_size;
	ft_stat->packet_stat_size *= 2;
	void *temp = realloc(ft_stat->packet_stat, 
							ft_stat->packet_stat_size * sizeof(struct ft_packet_statistics));
	if (temp == NULL){
		free(ft_stat->packet_stat);
		return ERROR;	
	}
	ft_stat->packet_stat = temp;
	for (size_t i = old_size; i < ft_stat->packet_stat_size; i++){
		ft_stat->packet_stat[i].received = not_received;
	}
	return SUCCESS;
}


void receive_packet(struct ft_socket ft_socket, struct ft_statistics *ft_stat, struct ft_flag flag)
{
	char buffer_recv[BUFFER_SIZE] = {0};
	struct sockaddr recv_addr;
	socklen_t len = sizeof(struct sockaddr);
	t_ft_icmp icmp_packet_recv = {0};
	
	ssize_t byte_recv = recvfrom(ft_socket.socket_fd, buffer_recv, BUFFER_SIZE, 
								MSG_DONTWAIT, &recv_addr, &len);
	if (byte_recv == -1){
		int a = errno;
		// we can't use poll/epoll/select so we check over and over if we have a message ...
		if (a == EAGAIN || a == EWOULDBLOCK)
			return ;
		perror("Error byte_receive");
		printf("errno value %d\n", a);
		return ;
	}	
	
	// len of ip header => https://en.wikipedia.org/wiki/IPv4#Packet_structure
	uint16_t ip_len = (buffer_recv[0] & 0x0f) * 32 / 8;

	// in case the response is not ECHOREPLY some value will be wrong => we correct later
	memcpy(&icmp_packet_recv, &buffer_recv[ip_len], sizeof(icmp_packet_recv));

	uint16_t seq = icmp_packet_recv.seq;
	// we dont read the other packet ==> mainly because when pinging localhost we will receive the ECHO_REQUEST
	if (icmp_packet_recv.type != ICMP_ECHOREPLY && flag.verbose)
		print_debug(buffer_recv);
	if (icmp_packet_recv.type != ICMP_ECHOREPLY && icmp_packet_recv.type !=  ICMP_TIME_EXCEEDED)
		return;
	if (icmp_packet_recv.id != getpid() && icmp_packet_recv.type == ICMP_ECHOREPLY) // Ignore other ping ==> happen when multiple ping runs
		return;
	if (icmp_packet_recv.type == ICMP_TIME_EXCEEDED){
		if (byte_recv < 54){
			printf("Error packet too small got %zd\n", byte_recv);
			return;
		}
		uint16_t id = 0;
	// CHECK ID of sender => 33-34th byte of ICMP ==> 52th byte of buffer (20 of ip + icmp)
		memcpy(&id, &buffer_recv[52], 2 * sizeof(char));
		if (id != getpid())
			return;
		// seq is after id of the buffer we received
		memcpy(&seq, &buffer_recv[54], 2 * sizeof(char));
	}
	if (seq >= ft_stat->packet_stat_size - 1){
		if (realloc_packet_stat(ft_stat) == ERROR){
			close(ft_socket.socket_fd);
			error_exit("Malloc failed");
		}
	}
	ft_stat->packet_stat[seq].time_taken_ms = calculate_time_taken(icmp_packet_recv.timestamp);
	if (recv_addr.sa_family == AF_INET)
		resolve_sender_host(ft_stat, (struct sockaddr_in *)&recv_addr , len);
	ft_stat->packet_stat[seq].received = received;
	if (icmp_packet_recv.type == ICMP_TIME_EXCEEDED){
		ft_stat->packet_stat[seq].received = error;
	}
	ft_stat->packet_stat[seq].seq = seq;
	ft_stat->packet_stat[seq].ttl = extract_ttl(buffer_recv);
	ft_stat->packet_stat[seq].icmp_type = icmp_packet_recv.type;
	ft_stat->packet_stat[seq].size_bytes_received = byte_recv - ip_len;		
	pretty_print(ft_stat->packet_stat[seq], ft_stat->pretty_hostname);
	ft_stat->nb_packet_received ++;
	
}

ft_error send_packet(struct ft_socket ft_socket)
{
	t_ft_icmp icmp_packet_sent = create_echo_packet();
	ssize_t byte_sent = sendto(ft_socket.socket_fd, 
							    &icmp_packet_sent, 
								sizeof (icmp_packet_sent), 
								0, 
								(struct sockaddr *) &ft_socket.dest, 
								sizeof(struct sockaddr_in));
	if (byte_sent == -1){
		// we have an error==> all possible error are fatal in our case
		perror("Error sendto:");
		close(ft_socket.socket_fd);
		return ERROR;
	}
	return SUCCESS;
}