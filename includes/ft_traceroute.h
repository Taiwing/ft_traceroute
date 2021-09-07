/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/06 23:40:29 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/08 00:17:03 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_TRACEROUTE_H
# define FT_TRACEROUTE_H

# include "libft.h"

/*
** t_trcrt_config: traceroute configuration
**
** exec: executable name
** dest: destination argument
** max_ttl: max number of hops
*/
typedef struct	s_trcrt_config
{
	const char	*exec;
	const char	*dest;
	int			max_ttl;
}				t_trcrt_config;

/*
** traceroute macros
*/
# define	MAX_TTL_DEF			30
# define	FT_TRACEROUTE_OPT	"hm:"
# define	FT_TRACEROUTE_HELP	"Usage:\n\t%s [options] <destination>\n"\
	"Options:\n\t<destination>\t\thostname or IPv4 address\n"\
	"\t-h\t\t\tprint help and exit\n"\
	"\t-m max_ttl\t\tset max number of hops (max TTL)\n"

#endif
