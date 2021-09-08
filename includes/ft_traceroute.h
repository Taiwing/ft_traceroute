/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/06 23:40:29 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/08 21:43:45 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_TRACEROUTE_H
# define FT_TRACEROUTE_H

# include "libft.h"
# include <limits.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <netdb.h>
# include <arpa/inet.h>

/*
** t_udphdr: UDP header structure
**
** srcp: source port (unused, set to 0)
** dstp: destination port (set to port and incremented)
** len: length of udp header and data (fixed)
** sum: checksum of packet
*/
typedef struct	s_udphdr
{
	uint16_t	srcp;
	uint16_t	dstp;
	uint16_t	len;
	uint16_t	sum;
}				t_udphdr;

/*
** Probe macros
*/

# define	PROBE_SIZE			60
# define	PROBE_UDP_LEN		(PROBE_SIZE - sizeof(struct iphdr))
# define	PROBE_UDP_DATA_LEN	(PROBE_UDP_LEN - sizeof(t_udphdr))

/*
** t_probe: udp probe structure
**
** udp: udp header
** data: data buffer
*/
typedef struct	s_probe
{
	t_udphdr	udp;
	char		data[PROBE_UDP_DATA_LEN];
}				t_probe;

/*
** t_trcrt_config: traceroute configuration
**
** exec: executable name
** dest: destination argument
** destip: destination ip from getaddrinfo
** destip_str: destination ip string from inet_ntop
** max_ttl: max number of hops
** sprobes: number of probe packets to send simultaneously
** nprobes: number of probe packets per hop
** port: destination port for UDP probes
*/
typedef struct			s_trcrt_config
{
	const char			*exec;
	const char			*dest;
	struct sockaddr		destip;
	char				destip_str[INET_ADDRSTRLEN + 1];
	int					max_ttl;
	int					sprobes;
	int					nprobes;
	int					port;
}						t_trcrt_config;

/*
** traceroute macros
*/

# define	MAX_TTL_DEF			30
# define	SPROBES_DEF			16
# define	SPROBES_MAX			512
# define	NPROBES_DEF			3
# define	NPROBES_MAX			10
# define	PORT_DEF			33434

# define	CONFIG_DEF			{\
	NULL, NULL, { 0 }, { 0 }, MAX_TTL_DEF, SPROBES_DEF, NPROBES_DEF, PORT_DEF\
}

# define	FT_TRACEROUTE_OPT	"hm:N:p:q:"
# define	FT_TRACEROUTE_HELP	"Usage:\n\t%s [options] <destination>\n"\
	"Options:\n\t<destination>\t\thostname or IPv4 address\n"\
	"\t-h\t\t\tprint help and exit\n"\
	"\t-m max_ttl\t\tset max number of hops (max TTL)\n"\
	"\t-N sprobes\t\tnumber of probe packets sent out simultaneously\n"\
	"\t-p port\t\t\tdestination port base for UDP probes\n"\
	"\t-q nprobes\t\tnumber of probe packets per hop\n"

#endif
