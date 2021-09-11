/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/06 23:40:29 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/11 14:17:09 by yforeau          ###   ########.fr       */
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
# define	PROBE_UDP_LEN		(PROBE_SIZE - sizeof(struct ip))
# define	PROBE_UDP_DATA_LEN	(PROBE_UDP_LEN - sizeof(t_udphdr))

/*
** t_probe_packet: udp packet probe structure
**
** ip: ip header
** udp: udp header
** data: data buffer
*/
typedef struct	s_probe_packet
{
	struct ip	ip;
	t_udphdr	udp;
	char		data[PROBE_UDP_DATA_LEN];
}				t_probe_packet;

enum e_probe_status	{ E_PRSTAT_SENT = 0, E_PRSTAT_RECEIVED, E_PRSTAT_TIMEOUT };

/*
** t_probe: probe info and status structure
**
** status: request sent, response received or timeout
** sent_ts: timestamp when sent (gettimeofday)
** received_ts: timestamp if received (gettimeofday)
** received_ip: source ip of response if any
*/
typedef struct			s_probe
{
	enum e_probe_status	status;
	struct timeval		sent_ts;
	struct timeval		received_ts;
	struct in_addr		received_ip;
}						t_probe;

/*
** traceroute macros
*/

# define	MAX_TTL_DEF			30
# define	MAX_TTL_MAX			UCHAR_MAX
# define	SPROBES_DEF			16
# define	SPROBES_MAX			512
# define	NPROBES_DEF			3
# define	NPROBES_MAX			10
# define	PORT_DEF			33434
# define	PROBES_MAX			(MAX_TTL_MAX * NPROBES_MAX)

# define	CONFIG_DEF			{\
	ft_exec_name(*argv), NULL, { 0 }, { 0 }, MAX_TTL_DEF, SPROBES_DEF,\
	NPROBES_DEF, PORT_DEF, getpid(), 0, 0, 0, 0, 0, 0,\
	{{ 0 }}, {{ 0 }, { 0 }, { 0 }}\
}

# define	FT_TRACEROUTE_OPT	"hm:N:p:q:"
# define	FT_TRACEROUTE_HELP	"Usage:\n\t%s [options] <destination>\n"\
	"Options:\n\t<destination>\t\thostname or IPv4 address\n"\
	"\t-h\t\t\tprint help and exit\n"\
	"\t-m max_ttl\t\tset max number of hops (max TTL)\n"\
	"\t-N sprobes\t\tnumber of probe packets sent out simultaneously\n"\
	"\t-p port\t\t\tdestination port base for UDP probes\n"\
	"\t-q nprobes\t\tnumber of probe packets per hop\n"

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
** port: destination port for UDP probes (port + id for sending probes)
** pid: seq value for UDP probes ip header (pid + id)
** send_socket: fd of SOCK_DGRAM/IPPROTO_UDP socket
** receive_socket: fd of SOCK_DGRAM/IPPROTO_ICMP socket
** hop: last completed hop
** probe_id: id of the next probe to send (also total count of sent probes)
** hop_first_id: hop * nprobes is the id of the current hop's first probe
** pending_probes: number of probes sent and waiting for a response
** probes: array of probes (id is the index)
** probe_packet: actual probe packet to send
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
	int					pid;
	int					send_socket;
	int					receive_socket;
	int					hop;
	int					probe_id;
	int					hop_first_id;
	int					pending_probes;
	t_probe				probes[PROBES_MAX];
	t_probe_packet		probe_packet;
}						t_trcrt_config;

#endif
