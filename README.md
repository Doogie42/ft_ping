# README

## ft_ping

ft_ping is a simple implementation of the ping utility in C made during my time at 42. It allows you to send ICMP echo request packets to a specified destination and measure the round-trip time (RTT) for each packet.

## Features

- Send ICMP echo request packets to a specified destination.
- Measure the round-trip time (RTT) for each packet.
- Display packet loss percentage and total time taken.
- Set custom options such as interval, count, and time-to-live (TTL).

## Usage

```
./ft_ping [options] <destination>
```

Options:

- `-i <interval>`: Set the interval between sending each packet in seconds.
- `-c <count>`: Stop after sending a specified number of packets.
- `-t <ttl>`: Set the time-to-live (TTL) value for the packets.

## Build

To build the ft_ping executable, run the following command:

```
git clone https://github.com/Doogie42/ft_ping.git
make
```

## Run

To run the ft_ping program, use the following command:

```
./ft_ping [options] <destination>
```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
