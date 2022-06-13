/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   traceroute.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/12 08:47:53 by yforeau           #+#    #+#             */
/*   Updated: 2022/06/13 22:08:40 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

static void	send_probe(t_trcrt_config *cfg)
{
	uint16_t	port;
	int			ttl = (cfg->probe_id / cfg->nprobes) + 1;

	port = (cfg->port + cfg->probe_id) % 0xffff;
	if (cfg->port > port)
		++port;
	if (cfg->domain == AF_INET && setsockopt(cfg->send_socket, SOL_IP, IP_TTL,
		(void *)&ttl, sizeof(int)) < 0)
		ft_exit(EXIT_FAILURE, "setsockopt: %s", strerror(errno));
	else if (cfg->domain == AF_INET6 && setsockopt(cfg->send_socket, SOL_IPV6,
		IPV6_UNICAST_HOPS, (void *)&ttl, sizeof(int)) < 0)
		ft_exit(EXIT_FAILURE, "setsockopt: %s", strerror(errno));
	ft_ip_set_port(&cfg->destip, htons(port));
	if (gettimeofday(&cfg->probes[cfg->probe_id].sent_ts, NULL) < 0)
		ft_exit(EXIT_FAILURE, "gettimeofday: %s", strerror(errno));
	if (sendto(cfg->send_socket, (void *)cfg->probe_data,
		sizeof(cfg->probe_data), 0, (struct sockaddr *)&cfg->destip,
		ft_ip_sock_size(&cfg->destip)) < 0)
		ft_exit(EXIT_FAILURE, "sendto: %s", strerror(errno));
}

static void	send_probes(t_trcrt_config *cfg)
{
	while (cfg->pending_probes < cfg->sprobes
		&& cfg->probe_id < cfg->max_probes)
	{
		send_probe(cfg);
		++cfg->probe_id;
		++cfg->pending_probes;
	}
	cfg->last_pending_hop = ((cfg->probe_id - 1) / cfg->nprobes) + 1;
}

void		traceroute(t_trcrt_config *cfg)
{
	char			*err = NULL;

	cfg->max_probes = cfg->max_ttl * cfg->nprobes;
	cfg->max_timeout = cfg->max * 1000.0;
	for (int i = 0; i <= cfg->max_ttl; ++i)
		cfg->hop_timeout[i] = cfg->max_timeout;
	while (!err && !cfg->reached && !cfg->unreachable
		&& (cfg->pending_probes || cfg->probe_id < cfg->max_probes))
	{
		//send as many probes as possible
		if (cfg->pending_probes < cfg->sprobes
			&& cfg->probe_id < cfg->max_probes)
			send_probes(cfg);
		//read responses if any
		if (!err)
			err = read_responses(cfg);
		//check pending probes and set timeouts if needed
		if (!err && cfg->pending_probes)
			err = check_pending_probes(cfg);
		//print hops if some are completed
		if (!err)
			while (!cfg->reached && !cfg->unreachable && print_hop(cfg))
				cfg->hop_first_id += cfg->nprobes;
	}
	if (err)
		ft_exit(EXIT_FAILURE, err);
}
