/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   traceroute.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/12 08:47:53 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/13 22:38:00 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

static char	*send_probe(t_trcrt_config *cfg)
{
	char		*err = NULL;
	uint16_t	port, seq;
	int			ttl = (cfg->probe_id / cfg->nprobes) + 1;

	port = (cfg->port + cfg->probe_id) % 0xffff;
	seq = (cfg->ident + cfg->probe_id) % 0xffff;
	ft_memcpy((void *)cfg->probe_data, (void *)&seq, sizeof(uint16_t));
	if (setsockopt(cfg->send_socket, SOL_IP, IP_TTL,
		(void *)&ttl, sizeof(int)) < 0)
		ft_asprintf(&err, "setsockopt: %s", strerror(errno));
	if (!err && gettimeofday(&cfg->probes[cfg->probe_id].sent_ts, NULL) < 0)
		ft_asprintf(&err, "gettimeofday: %s", strerror(errno));
	cfg->destip.sin_port = htons(port);
	if (!err && sendto(cfg->send_socket, (void *)cfg->probe_data,
		sizeof(cfg->probe_data), 0, (struct sockaddr *)&cfg->destip,
		sizeof(struct sockaddr_in)) < 0)
		ft_asprintf(&err, "sendto: %s", strerror(errno));
	return (err);
}

static char	*send_probes(t_trcrt_config *cfg)
{
	char	*err = NULL;

	while (!err && cfg->pending_probes < cfg->sprobes
		&& cfg->probe_id < cfg->max_probes)
	{
		if ((err = send_probe(cfg)))
			break ;
		++cfg->probe_id;
		++cfg->pending_probes;
	}
	return (err);
}

static char	*check_pending_probes(t_trcrt_config *cfg)
{
	int				timeout = 0;
	char			*err = NULL;
	struct timeval	now = { 0 }, diff = { 0 };

	if (gettimeofday(&now, NULL) < 0)
	{
		ft_asprintf(&err, "gettimeofday: %s", strerror(errno));
		return (err);
	}
	for (int i = cfg->probe_id - 1; i >= 0 && cfg->pending_probes; --i)
	{
		if (cfg->probes[i].status)
			continue ;
		if (!timeout)
		{
			ts_diff(&diff, &now, &cfg->probes[i].sent_ts);
			timeout = diff.tv_sec >= TRCRT_RESP_TMOUT;
		}
		if (timeout)
		{
			cfg->probes[i].status = E_PRSTAT_TIMEOUT;
			--cfg->pending_probes;
		}
	}
	return (err);
}

void		traceroute(t_trcrt_config *cfg)
{
	char			*err = NULL;

	cfg->max_probes = cfg->max_ttl * cfg->nprobes;
	while (!err && !cfg->reached && (cfg->pending_probes
		|| cfg->probe_id < cfg->max_probes))
	{
		//send as many probes as possible
		if (cfg->pending_probes < cfg->sprobes
			&& cfg->probe_id < cfg->max_probes)
			err = send_probes(cfg);
		//read responses if any
		if (!err)
			err = read_responses(cfg);
		//check pending probes and set timeouts if needed
		if (!err && cfg->pending_probes)
			err = check_pending_probes(cfg);
		//print hops if some are completed
		if (!err)
			while (!cfg->reached && print_hop(cfg))
				cfg->hop_first_id += cfg->nprobes;
	}
	if (err)
		ft_exit(err, EXIT_FAILURE);
}
