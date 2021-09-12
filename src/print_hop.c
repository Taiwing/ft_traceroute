/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_hop.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/12 20:27:02 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/12 21:11:50 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

const char	*g_prstat_err[] = { "*", "!N", "!H", "!P", "!F", "!S", NULL };

static void	print_valid_response(t_trcrt_config *cfg, int i)
{
	double			time;
	struct timeval	ts = { 0 };

	ts_diff(&ts, &cfg->probes[i].received_ts, &cfg->probes[i].sent_ts);
	time = (double)ts.tv_sec * 1000.0 + (double)ts.tv_usec / 1000.0;
	if (i == cfg->hop_first_id
		|| cfg->probes[i].received_ip.s_addr
		!= cfg->probes[i-1].received_ip.s_addr)
		ft_printf("%1$s%2$s (%2$s)  %3$.3f ms",
			i == cfg->hop_first_id ? "" : " ",
			inet_ntoa(cfg->probes[i].received_ip), time);
	else
		ft_printf("%s%.3f ms", i == cfg->hop_first_id ? "" : "  ", time);
}

int			print_hop(t_trcrt_config *cfg)
{
	int		hop_last_id;

	if (cfg->nprobes > (cfg->probe_id - cfg->hop_first_id))
		return (0);
	hop_last_id = cfg->hop_first_id + cfg->nprobes - 1;
	for (int i = cfg->hop_first_id; i <= hop_last_id; ++i)
		if (!cfg->probes[i].status)
			return (0);
	ft_printf(" %d  ", ++cfg->hop);
	for (int i = cfg->hop_first_id; i <= hop_last_id; ++i)
	{
		if (cfg->probes[i].status < E_PRSTAT_TIMEOUT)
			print_valid_response(cfg, i);
		else
			ft_printf(" %s",
				g_prstat_err[cfg->probes[i].status - E_PRSTAT_TIMEOUT]);
		if (!cfg->reached)
			cfg->reached = cfg->probes[i].status == E_PRSTAT_RECEIVED_PORT;
	}
	ft_putchar('\n');
	return (1);
}
