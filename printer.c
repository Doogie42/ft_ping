#include "ft_ping.h"

void pretty_print_stat(struct ft_statistics ft_stat)
{
	// 	2 packets transmitted, 2 received, 0% packet loss, time 1002ms
	// rtt min/avg/max/mdev = 1.158/1.196/1.234/0.038 ms
	unsigned int nb_packet = 0;
	unsigned int nb_packet_error = 0;
	double min = (double)SIZE_MAX;
	double max = 0.0;
	double avg = 0.0;
	double mdev = 0.0;
	double variance = 0.0;
	double loss = 0.0;
	double time_taken_ms = 0.0;
	// remove ^C
	printf("\b\b");

	time_taken_ms = calculate_time_taken(ft_stat.start);
	for (size_t i = 0; i < ft_stat.packet_stat_size; ++i)
	{
		if (ft_stat.packet_stat[i].received == received)
		{
			nb_packet++;
			if (ft_stat.packet_stat[i].time_taken_ms > max)
				max = ft_stat.packet_stat[i].time_taken_ms;
			if (ft_stat.packet_stat[i].time_taken_ms < min)
				min = ft_stat.packet_stat[i].time_taken_ms;
			avg += ft_stat.packet_stat[i].time_taken_ms;
			variance += ft_stat.packet_stat[i].time_taken_ms * ft_stat.packet_stat[i].time_taken_ms;
		}
		if (ft_stat.packet_stat[i].received == error)
		{
			nb_packet_error++;
		}
	}
	printf("\n--- %s ping statistics ---\n", ft_stat.request);
	printf("%d packets transmitted, %d received, ", ft_stat.sent_packet, nb_packet);
	if (nb_packet_error != 0)
	{
		printf("+%d errors,", nb_packet_error);
	}
	loss = 1.0 - ((double)nb_packet / (double)ft_stat.sent_packet);
	printf("%.2f%% packet loss, time %dms\n", loss * 100, (int)time_taken_ms);
	if (nb_packet)
	{
		avg = avg / nb_packet;
		mdev = sqrt(variance / nb_packet - avg * avg);
		printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms", min, avg, max, mdev);
	}
	printf("\n");
}

void pretty_print(struct ft_packet_statistics packet_stat, char *serv_name)
{
	printf("%zd bytes from ", packet_stat.size_bytes_received);
	printf("%s :", serv_name);
	printf("icmp_seq=%d ", packet_stat.seq);
	if (packet_stat.icmp_type == ICMP_ECHOREPLY)
	{
		printf("ttl=%d ", packet_stat.ttl);
		printf("time=%.4g ms", packet_stat.time_taken_ms);
	}
	if (packet_stat.icmp_type == ICMP_TIME_EXCEEDED)
	{
		printf("Time to live exceeded");
	}
	// we drop all other packet
	printf("\n");
}

void print_debug(char *buffer_recv)
{
	// We only print the type and code of imcp

	uint16_t id = 0;
	uint16_t seq = 0;
	uint16_t type = buffer_recv[48];
	uint16_t code = buffer_recv[49];

	memcpy(&id, &buffer_recv[52], 2 * sizeof(char));
	memcpy(&seq, &buffer_recv[54], 2 * sizeof(char));

	printf("ICMP: type %d, code %d, id 0x%2.x, seq %d",
		   type, code, id, seq);
	printf("\n");
}