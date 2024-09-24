#include "ft_ping.h"

static void check_only_digit(char *str)
{
	size_t i = 0;
	while (str[i])
	{
		if (str[i] > '9' || str[i] < '0')
		{
			printf("ping: invalid argument: '%s'\n", str);
			exit(1);
		}
		i++;
	}
}

void print_help()
{
	printf("Usage\n\
  ping [options] <destination>\n\
\n\
Options:\n\
-i <interval>      seconds between sending each packet\n\
-c <count>         stop after sending NUMBER packets\n\
-t <ttl>           define time to live\n");
}
struct ft_flag get_flag(int argc, char **argv)
{
	struct ft_flag flag;
	// default ttl
	flag.ttl = 64;
	flag.max_count = __LONG_LONG_MAX__;
	flag.interval_ms = 1000.0;
	flag.verbose = false;
	while (1)
	{
		int c = getopt(argc, argv, "vt:i:c:?h");
		if (c == -1)
			break;
		switch (c)
		{
		case 'v':
			flag.verbose = true;
			break;
		case 'i':
			flag.interval_ms = atof(optarg) * 1000.0;
			break;
		case 'c':
			// ping: invalid argument: '-1': out of range: 1 <= value <= 9223372036854775807
			// 9223372036854775807
			check_only_digit(optarg);
			// max long long ... --'
			if (strcmp("9223372036854775807", optarg) < 0 || strlen(optarg) > 20)
			{
				printf("ping: invalid argument: '%s': out of range: 0 <= value <= 9223372036854775807\n", optarg);
				exit(1);
			}
			flag.max_count = atoll(optarg);
			break;
		case 't':
			check_only_digit(optarg);
			flag.ttl = atoi(optarg);
			if (strlen(optarg) > 3 || flag.ttl < 0 || flag.ttl > 255)
			{
				printf("ping: invalid argument: '%s': out of range: 0 <= value <= 255\n", optarg);
				exit(1);
			}
			break;
		case '\?':
		case 'h':
			print_help();
			exit(0);
			break;
		default:
			printf("ERROR");
			exit(1);
		}
	}
	// extern int optind,
	flag.arg = argv[optind];
	if (flag.arg == NULL)
	{
		printf("ping: usage error: Destination address required\n");
		exit(1);
	}
	return flag;
}