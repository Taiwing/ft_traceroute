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
