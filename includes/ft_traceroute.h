/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/06 23:40:29 by yforeau           #+#    #+#             */
/*   Updated: 2022/06/27 10:10:36 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_TRACEROUTE_H
# define FT_TRACEROUTE_H

# include "libft.h"
# include <limits.h>
# include <float.h>
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

# define	PROBE_SIZE(domain)			(domain == AF_INET ? 60 : 24)
# define	PROBE_UDP_DATA_LEN(domain)	(domain == AF_INET ? 32 : 24)
# define	RESP_HEADERS(domain)		(\
		(domain == AF_INET ? sizeof(struct ip) : sizeof(struct ipv6hdr)) * 2\
		+ sizeof(struct icmphdr) + sizeof(t_udphdr))

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
	t_ip				received_ip;
}						t_probe;

/*
** traceroute macros
*/

# define	MAX_TTL_DEF			30
# define	MAX_TTL_MAX			UCHAR_MAX
# define	SPROBES_DEF			1
# define	SPROBES_MAX			512
# define	NPROBES_DEF			3
# define	NPROBES_MAX			10
# define	PORT_DEF			33434
# define	PROBES_MAX			(MAX_TTL_MAX * NPROBES_MAX)
# define	MAX_DEF				5.0
# define	HERE_DEF			3.0
# define	NEAR_DEF			10.0

// select timeout in microseconds (is equal to 505ms)
# define	SLCT_TMOUT		505000

# define	ICMP_PROTO(domain)	(\
	domain == AF_INET ? IPPROTO_ICMP : IPPROTO_ICMPV6)

# define	FT_TRACEROUTE_OPT	"hm:N:p:q:w:46"
# define	FT_TRACEROUTE_HELP	"Usage:\n\t%s [options] <destination>\n\n"\
	"Options:\n\t<destination>\t\thostname, IPv4 or IPv6 address\n\n"\
	"\t-h\t\t\tPrint help and exit.\n\n"\
	"\t-m max_ttl\t\tSet max number of hops and TTL value (def: %d).\n\n"\
	"\t-N sprobes\t\tNumber of probe packets sent out simultaneously (def: %d)"\
	".\n\n\t-p port\t\t\tDestination port base for UDP probes (def: %d).\n\n"\
	"\t-q nprobes\t\tNumber of probe packets per hop (def: %d).\n\n"\
	"\t-w max[,here,near]\tThree float values separated by ',' or '/'."\
	" 'max' is the\n\t\t\t\tmaximum time, in seconds, to wait for a"\
	" response in any\n\t\t\t\tcase (def: %.1f). The  optional 'here'"\
	" (def: %.1f) specifies\n\t\t\t\ta factor to multiply the round trip"\
	" time of an already\n\t\t\t\treceived response from the same hop"\
	" and sets it as a new\n\t\t\t\ttimeout value (if less than max)."\
	" 'near' is the same as\n\t\t\t\t'here' except that it will look"\
	" at the next hop (def: %.1f).\n\n"\
	"\t-4\t\t\tUse only IPv4.\n\n"\
	"\t-6\t\t\tUse only IPv6.\n"

/*
** t_trcrt_config: traceroute configuration
**
** exec: executable name
** dest: destination argument
** destip: destination ip
** domain: AF_INET or AF_INET6
** max_ttl: max number of hops
** sprobes: number of probe packets to send simultaneously
** nprobes: number of probe packets per hop
** port: destination port for UDP probes (port + id for sending probes)
** ident: ((pid % 0xffff) | 0x8000) seq for UDP ip headers (ident + id)
** send_socket: fd of SOCK_DGRAM/IPPROTO_UDP socket
** recv_socket: fd of SOCK_RAW/IPPROTO_ICMP socket
** max_probes: total maximum count of probes to send (max_ttl * nprobes)
** reached: boolean set to true when destination is reached
** unreachable: boolean set to true when destination is unreachable
** hop: last completed hop
** last_pending_hop: last hop where probes have been sent
** probe_id: id of the next probe to send (also total count of sent probes)
** hop_first_id: hop * nprobes is the id of the current hop's first probe
** pending_probes: number of probes sent and waiting for a response
** probes: array of probes (id is the index)
** probe_packet: actual probe packet to send
** max: multiplicative factor for max timeout (max is in s)
** here: multiplicative factor for adaptative timeout on same hop
** near: multiplicative factor for adaptative timeout on next hop
** max_timeout: maximum timeout value for probe in ms (max * 1000.0)
** hop_timeout: timeout for each hop
*/
typedef struct			s_trcrt_config
{
	const char			*exec;
	const char			*dest;
	t_ip				destip;
	int					domain;
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
	int					last_pending_hop;
	int					probe_id;
	int					hop_first_id;
	int					pending_probes;
	t_probe				probes[PROBES_MAX];
	char				probe_data[PROBE_UDP_DATA_LEN(AF_INET)];
	double				max;
	double				here;
	double				near;
	double				max_timeout;
	double				hop_timeout[MAX_TTL_MAX + 1];
}						t_trcrt_config;

# define	CONFIG_DEF			{\
	ft_exec_name(*argv), NULL, { 0 }, AF_UNSPEC, MAX_TTL_DEF, SPROBES_DEF,\
	NPROBES_DEF, PORT_DEF, (getpid() % 0xffff) | 0x8000, 0, 0, 0, 0, 0, 0, 0,\
	0, 0, 0, {{ 0 }}, { 0 }, MAX_DEF, HERE_DEF, NEAR_DEF, 0.0, { 0.0 }\
}

/*
** ft_traceroute functions
*/

char	*get_options(t_trcrt_config *cfg, int argc, char **argv);
void	traceroute(t_trcrt_config *cfg);
char	*read_responses(t_trcrt_config *cfg);
char	*check_pending_probes(t_trcrt_config *cfg);
double	ts_msdiff(struct timeval *a, struct timeval *b);
int		ts_diff(struct timeval *res, struct timeval *a, struct timeval *b);
int		print_hop(t_trcrt_config *cfg);

#endif
