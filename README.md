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

> If you need to run this on a machine on which you do not have sudo rights this
> will only work with a virtual machine. Docker's containerization will fail
> because its network driver does not keep the TTL values of outgoing packets
> which totally breaks any traceroute utility.

## Usage

```
Usage:
	ft_traceroute [options] <destination>

Options:
	<destination>		hostname, IPv4 or IPv6 address

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

	-4			Use only IPv4.

	-6			Use only IPv6.
```

> This program handles local and remote hostnames as well IPv4 and IPv6
> addresses.

#### example:

```shell
# traceroute google.com
sudo ./ft_traceroute google.com
```

possible output:

```
traceroute to google.com (216.58.213.14), 30 hops max, 60 byte packets
 1  77-240-12-190.rdns.nuttyabouthosting.co.uk (77.240.12.190)  1.069 ms  1.113 ms  1.354 ms
 2  1001.te3-1.core2.dc7.as20860.net (77.240.7.139)  8.751 ms  8.952 ms  8.948 ms
 3  * * *
 4  be2.asr01.dc7.as20860.net (130.180.202.2)  9.045 ms  11.011 ms  11.407 ms
 5  * * *
 6  * * *
 7  108.170.244.193 (108.170.246.161)  7.285 ms 108.170.225.172 (108.170.225.172)  7.573 ms  7.871 ms
 8  142.250.224.199 (172.253.65.211)  8.164 ms  8.483 ms  9.070 ms
 9  lhr25s25-in-f14.1e100.net (216.58.213.14)  8.442 ms  9.370 ms  9.590 ms
 ```

## How it works

ft\_traceroute crafts
[UDP](https://en.wikipedia.org/wiki/User_Datagram_Protocol) probe packets
(User Datagram Protocol) filled with random data. UDP is a layer 4 protocol, so
it is a transport protocol like TCP. Its goal is to deliver a data payload to a
given target. However, this is not really important in an ft\_traceroute
implementation. Everything needed for the traceroute functions is provided by
the IP header.

The main trick for getting the intermediary hops list to the given target is to
use the TTL field of the probes'
[IP header](https://en.wikipedia.org/wiki/IPv4#Header). The Time To Live field
sets a limit for the number of hops a packet can pass through (that's why it has
been renamed to 'hop\_limit' in the IPv6 protocol). Since the TTL field is
decremented at each hop it will inevitably reach 0 if the host is not found.
This avoids having packets wandering around the internet forever with no valid
destination. When it does reach 0 the router is supposed to return an ICMP error
packet of the type 'Time Exceeded' back to the sender.

So ft\_traceroute sends its first probe with a TTL of one for the packet to
expire on the first hop it goes through. Then, through the IP header of the ICMP
response, it has access to the router's address. After that the TTL of the UDP
probe is incremented by one to reach the next hop. This repeats until the target
host is found or the max\_ttl limit is reached (which can be set with the -m
option).

#### UDP probe example:

IPv4 header:

| version | total length | time to live | protocol | source ip | destination ip |
|---------|--------------|--------------|----------|-----------|----------------|
| 4       | 60           | 1            | UDP      | 1.2.3.4   | 216.58.213.14  |

> Some fields have been removed for clarity. For a complete overview of the IPv4
> protocol check [this page](https://en.wikipedia.org/wiki/IPv4).

UDP header:

| source port | destination port | length | checksum |
|-------------|------------------|--------|----------|
| 1234        | 33434            | 40     | 0xffff   |

The IP header is 20 bytes long, UDP is 8 bytes and there is 32 bytes of data,
which is why the total length IP field amounts to 60 bytes. The UDP length is 40
because it takes into account its own length as well as the data it carries. The
destination port of the UDP header does not matter much as long as it is not a
used or filtered port which is very unlikely for intermediary hops anyway.

Also, ft\_traceroute sends multiple probes per hop (nprobes as set with the -q
option). This is both because UDP can be unreliable in certain conditions, as it
does not natively provide any control mechanism on the reception of the packet,
and because network conditions can change the path of a probe. So a hop can have
multiple addresses when the route taken changes (as the in the example above for
the senventh hop). This is useful to check how stable the network is at a given
time or to diagnostic some problem along the route.
