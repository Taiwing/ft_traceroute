/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/14 13:48:54 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/15 22:31:19 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

/*
** ts_msdiff: timestamp ms substraction
**
** Substracts b from a and returns the result in ms.
*/
double	ts_msdiff(struct timeval *a, struct timeval *b)
{
	double s;
	double us;

	s = ((double)(a->tv_sec - b->tv_sec)) * 1000.0;
	us = ((double)(a->tv_usec - b->tv_usec)) / 1000.0;
	return (s + us);
}

/*
** ts_diff: timestamp substraction
**
** Substracts b from a and put the result in res. a must therefore be bigger
** than b (so a later timestamp), otherwise ts_diff will return -1 and res
** will be set to 0.
*/
int		ts_diff(struct timeval *res, struct timeval *a, struct timeval *b)
{
	ft_bzero((void *)res, sizeof(struct timeval));
	if (b->tv_sec > a->tv_sec
		|| (b->tv_sec == a->tv_sec && b->tv_usec > a->tv_usec))
		return (-1);
	res->tv_sec = a->tv_sec - b->tv_sec;
	if (a->tv_usec >= b->tv_usec)
		res->tv_usec = a->tv_usec - b->tv_usec;
	else
	{
		--res->tv_sec;
		res->tv_usec = 1000000 - b->tv_usec + a->tv_usec;
	}
	return (0);
}

static double	get_hop_timeout(t_trcrt_config *cfg, int i)
{
	double	here = cfg->max_timeout;

	for (int j = i; j < i + cfg->nprobes && j < cfg->probe_id; ++j)
		if (cfg->probes[j].status && cfg->probes[j].status < E_PRSTAT_TIMEOUT) 
		{
			here = ts_msdiff(&cfg->probes[j].received_ts,
				&cfg->probes[j].sent_ts) * cfg->here;
			break ;
		}
	return (here < cfg->max_timeout ? here : cfg->max_timeout);
}

char	*check_pending_probes(t_trcrt_config *cfg)
{
	int				fp;
	char			*err = NULL;
	struct timeval	now = { 0 };
	double			timeout = cfg->max_timeout, near = cfg->max_timeout;

	if (gettimeofday(&now, NULL) < 0)
		ft_asprintf(&err, "gettimeofday: %s", strerror(errno));
	for (int i = cfg->last_pending_hop - 1; i >= cfg->hop; --i)
	{
		fp = i * cfg->nprobes;
		if (cfg->hop_timeout[i] == cfg->max_timeout && cfg->here)
			cfg->hop_timeout[i] = get_hop_timeout(cfg, fp);
		if (cfg->hop_timeout[i] != cfg->max_timeout && cfg->near)
			near = cfg->hop_timeout[i] * cfg->near < cfg->max_timeout ?
				cfg->hop_timeout[i] * cfg->near : near;
		timeout = near < cfg->hop_timeout[i] ? near : cfg->hop_timeout[i];
		for (int j = fp; j < fp + cfg->nprobes && j < cfg->probe_id; ++j)
			if (!cfg->probes[j].status
				&& ts_msdiff(&now, &cfg->probes[j].sent_ts) >= timeout)
			{
				cfg->probes[j].status = E_PRSTAT_TIMEOUT;
				--cfg->pending_probes;
			}
	}
	return (err);
}
