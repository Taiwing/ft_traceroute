/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/06 23:40:29 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/14 14:40:16 by yforeau          ###   ########.fr       */
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
# include <sys/time.h>
# include <errno.h>

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

# define	PROBE_SIZE			1024
# define	PROBE_UDP_LEN		(PROBE_SIZE - sizeof(struct ip))
# define	PROBE_UDP_DATA_LEN	(PROBE_UDP_LEN - sizeof(t_udphdr))
# define	RESP_HEADERS		(sizeof(struct ip) + sizeof(struct icmphdr)\
		+ sizeof(struct ip) + sizeof(t_udphdr))
# define	RESP_HEADERS_SEQ	(RESP_HEADERS + sizeof(uint16_t))

/*
** t_icmp_packet: icmp packet structure
**
** ip: ip header of response
** icmp: icmp header of response
** data_ip: ip header of the probe packet
** data_udp: udp header of the probe packet
** data: data of the probe packet
*/
typedef struct		s_icmp_packet
{
	struct ip		ip;
	struct icmphdr	icmp;
	struct ip		data_ip;
	t_udphdr		data_udp;
	char			data[PROBE_UDP_DATA_LEN];
}					t_icmp_packet;

/*
** enum e_probe_status: list of every possible states for a probe
*/
enum e_probe_status	{
	E_PRSTAT_SENT = 0, E_PRSTAT_RECEIVED_TTL, E_PRSTAT_RECEIVED_PORT,
	E_PRSTAT_TIMEOUT, E_PRSTAT_UNREACH_NET, E_PRSTAT_UNREACH_HOST,
	E_PRSTAT_UNREACH_PROTOCOL, E_PRSTAT_UNREACH_NEEDFRAG,
	E_PRSTAT_UNREACH_SRCFAIL
};

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
# define	MAX_DEF				5.0
# define	HERE_DEF			3.0
# define	NEAR_DEF			10.0

# define	CONFIG_DEF			{\
	ft_exec_name(*argv), NULL, { 0 }, { 0 }, MAX_TTL_DEF, SPROBES_DEF,\
	NPROBES_DEF, PORT_DEF, (getpid() % 0xffff) | 0x8000, 0, 0, 0, 0,\
	0, 0, 0, 0, 0, {{ 0 }}, { 0 }, MAX_DEF, HERE_DEF, NEAR_DEF, 0.0\
}

// select timeout in microseconds (is equal to 505ms)
# define	SLCT_TMOUT		505000
// package response in seconds

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
** ident: ((pid % 0xffff) | 0x8000) seq for UDP ip headers (ident + id)
** send_socket: fd of SOCK_DGRAM/IPPROTO_UDP socket
** recv_socket: fd of SOCK_DGRAM/IPPROTO_ICMP socket
** max_probes: total maximum count of probes to send (max_ttl * nprobes)
** reached: boolean set to true when destination is reached
** unreachable: boolean set to true when destination is unreachable
** hop: last completed hop
** probe_id: id of the next probe to send (also total count of sent probes)
** hop_first_id: hop * nprobes is the id of the current hop's first probe
** pending_probes: number of probes sent and waiting for a response
** probes: array of probes (id is the index)
** probe_packet: actual probe packet to send
** max: multiplicative factor for max timeout (max is in s)
** here: multiplicative factor for adaptative timeout on same hop
** near: multiplicative factor for adaptative timeout on next hop
** max_timeout: maximum timeout value for probe in ms (max * 1000.0)
*/
typedef struct			s_trcrt_config
{
	const char			*exec;
	const char			*dest;
	struct sockaddr_in	destip;
	char				destip_str[INET_ADDRSTRLEN + 1];
	int					max_ttl;
	int					sprobes;
	int					nprobes;
	int					port;
	int					ident;
	int					send_socket;
	int					recv_socket;
	int					max_probes;
	int					reached;
	int					unreachable;
	int					hop;
	int					probe_id;
	int					hop_first_id;
	int					pending_probes;
	t_probe				probes[PROBES_MAX];
	char				probe_data[PROBE_UDP_DATA_LEN];
	double				max;
	double				here;
	double				near;
	double				max_timeout;
}						t_trcrt_config;

/*
** ft_traceroute functions
*/

void	traceroute(t_trcrt_config *cfg);
char	*read_responses(t_trcrt_config *cfg);
char	*check_pending_probes(t_trcrt_config *cfg);
double	ts_msdiff(struct timeval *a, struct timeval *b);
int		ts_diff(struct timeval *res, struct timeval *a, struct timeval *b);
int		print_hop(t_trcrt_config *cfg);

#endif
