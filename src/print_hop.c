/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_hop.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/12 20:27:02 by yforeau           #+#    #+#             */
/*   Updated: 2022/06/13 21:53:59 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

const char	*g_prstat_err[] = { "!N", "!H", "!P", "!F", "!S", NULL };

static char	*get_fqdn(char *buf, size_t size, t_ip *ip)
{
	t_ip	addr = { 0 };

	addr.family = ip->family;
	ft_memcpy((void *)ft_ip_addr(&addr),
		(void *)ft_ip_addr(ip), ft_ip_size(ip));
	if (!getnameinfo((struct sockaddr *)&addr,
		ft_ip_sock_size(ip), buf, size, NULL, 0, 0))
		return (buf);
	return (NULL);
}

static void	print_response(t_trcrt_config *cfg, int i)
{
	double	time;
	char	*fqdn;
	char	*ipstr;
	char	buf[NI_MAXHOST];

	if (cfg->probes[i].status == E_PRSTAT_TIMEOUT)
	{
		ft_printf("%s*", i == cfg->hop_first_id ? "" : " ");
		return ;
	}
	time = ts_msdiff(&cfg->probes[i].received_ts, &cfg->probes[i].sent_ts);
	if (i == cfg->hop_first_id || ft_ip_cmp(&cfg->probes[i].received_ip,
		&cfg->probes[i-1].received_ip))
	{
		ipstr = ft_ip_str(&cfg->probes[i].received_ip);
		if (!(fqdn = get_fqdn(buf, NI_MAXHOST, &cfg->probes[i].received_ip)))
			fqdn = ipstr;
		ft_printf("%s%s (%s)  %.3f ms",
			i == cfg->hop_first_id ? "" : " ", fqdn, ipstr, time);
	}
	else
		ft_printf("%s%.3f ms", i == cfg->hop_first_id ? "" : "  ", time);
}

int			print_hop(t_trcrt_config *cfg)
{
	int		hop_last_id, unreachable = 0;

	if (cfg->nprobes > (cfg->probe_id - cfg->hop_first_id))
		return (0);
	hop_last_id = cfg->hop_first_id + cfg->nprobes - 1;
	for (int i = cfg->hop_first_id; i <= hop_last_id; ++i)
		if (!cfg->probes[i].status)
			return (0);
	ft_printf("%2d  ", ++cfg->hop);
	for (int i = cfg->hop_first_id; i <= hop_last_id; ++i)
	{
		print_response(cfg, i);
		if (cfg->probes[i].status > E_PRSTAT_TIMEOUT)
			ft_printf(" %s",
				g_prstat_err[cfg->probes[i].status - E_PRSTAT_UNREACH_NET]);
		if (cfg->probes[i].status > E_PRSTAT_TIMEOUT
			&& cfg->probes[i].status != E_PRSTAT_UNREACH_PROTOCOL)
			++unreachable;
		if (!cfg->reached)
			cfg->reached = cfg->probes[i].status == E_PRSTAT_RECEIVED_PORT
				|| cfg->probes[i].status == E_PRSTAT_UNREACH_PROTOCOL;
	}
	ft_putchar('\n');
	cfg->unreachable = unreachable == cfg->nprobes;
	return (1);
}
