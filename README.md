# ft\_traceroute

This is a re-implementation of the traceroute utility in C. This program sends
probes to trace the path of network packets to a given host. It shows each hop
a packet can go through. This is useful for network diagnostics.

<br />
<p align="center">
	<img src="https://github.com/Taiwing/ft_traceroute/blob/main/resources/router.jpg?raw=true" alt="router" style="width: 50%;"/>
</p>

## Setup

```shell
# clone it with the libft submodule
git clone --recurse-submodules https://github.com/Taiwing/ft_traceroute
# build it
cd ft_traceroute/ && make
# run it
sudo ./ft_traceroute example.com
```

As shown above this program needs sudo rights. This is because ft\_traceroute
uses raw sockets to read ICMP responses. If you do not have root access on your
machine but have docker available you can use the `setup-docker.bash` script to
run ft\_traceroute. Just run the following commands after having cloned this
repository:

```shell
# build docker image and run it
./setup-docker.bash
# run ft_traceroute inside the container
./ft_traceroute example.com
```

## Usage

ft\_traceroute sends UDP probe packets by batches starting with a ttl of 1 and
incrementing it for each batch. At each intermediary hop between the source
computer and the remote host the probe will trigger an ICMP error response
because of the expired TTL. This is how ft\_traceroute gathers the addresses
and network stats of routing nodes before the target host.

```
Usage:
	ft_traceroute [options] <destination>

Options:
	<destination>		hostname or IPv4 address

	-h			Print help and exit.

	-m max_ttl		Set max number of hops and TTL value (def: 30).

	-N sprobes		Number of probe packets sent out simultaneously (def: 16).

	-p port			Destination port base for UDP probes (def: 33434).

	-q nprobes		Number of probe packets per hop (def: 3).

	-w max[,here,near]	Three float values separated by ',' or '/'. 'max' is the
				maximum time, in seconds, to wait for a response in any
				case (def: 5.0). The  optional 'here' (def: 3.0) specifies
				a factor to multiply the round trip time of an already
				received response from the same hop and sets it as a new
				timeout value (if less than max). 'near' is the same as
				'here' except that it will look at the next hop (def: 10.0).
```

> This program only handles valid local or remote hostnames and IPv4.

#### example:

```shell
# traceroute google.com
sudo ./ft_traceroute google.com
```
