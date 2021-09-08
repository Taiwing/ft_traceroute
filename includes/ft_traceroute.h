/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/06 23:40:29 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/08 07:30:39 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_TRACEROUTE_H
# define FT_TRACEROUTE_H

# include "libft.h"
# include <limits.h>

/*
** t_trcrt_config: traceroute configuration
**
** exec: executable name
** dest: destination argument
** max_ttl: max number of hops
** sprobes: number of probe packets to send simultaneously
** nprobes: number of probe packets per hop
** port: destination port for UDP probes
*/
typedef struct	s_trcrt_config
{
	const char	*exec;
	const char	*dest;
	int			max_ttl;
	int			sprobes;
	int			nprobes;
	int			port;
}				t_trcrt_config;

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
	NULL, NULL, MAX_TTL_DEF, SPROBES_DEF, NPROBES_DEF, PORT_DEF\
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
