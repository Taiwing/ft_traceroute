/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   traceroute.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/12 08:47:53 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/12 13:39:18 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

unsigned short	checksum(unsigned short *data, size_t sz)
{
	uint32_t		sum;
	unsigned short	res;

	for (sum = 0; sz >= sizeof(unsigned short); sz -= sizeof(unsigned short))
		sum += *data++;
	if (sz)
		sum += *((unsigned char *)data);
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	res = (unsigned short)~sum;
	return (res);
}

static char	*send_probe(t_trcrt_config *cfg)
{
	char	*err = NULL;
	int 	ttl = (cfg->probe_id / cfg->nprobes) + 1;

	if (setsockopt(cfg->send_socket, SOL_IP, IP_TTL,
		(void *)&ttl, sizeof(int)) < 0)
		ft_asprintf(&err, "setsockopt: %s", strerror(errno));
	if (!err && gettimeofday(&cfg->probes[cfg->probe_id].sent_ts, NULL) < 0)
		ft_asprintf(&err, "gettimeofday: %s", strerror(errno));
	cfg->destip.sin_port = htons(cfg->probes[cfg->probe_id].port);
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
		cfg->probes[cfg->probe_id].port =
			(cfg->port + cfg->probe_id) % (USHRT_MAX + 1);
		cfg->probes[cfg->probe_id].seq =
			(cfg->pid + cfg->probe_id) % (USHRT_MAX + 1);
		ft_memcpy((void *)cfg->probe_data,
			(void *)&cfg->probes[cfg->probe_id].seq, sizeof(uint16_t));
		if ((err = send_probe(cfg)))
			break ;
		++cfg->probe_id;
		++cfg->pending_probes;
	}
	return (err);
}

void		traceroute(t_trcrt_config *cfg)
{
	char	*err = NULL;

	cfg->max_probes = cfg->max_ttl * cfg->nprobes;
	while (!err && !cfg->reached && (cfg->pending_probes
		|| cfg->probe_id < cfg->max_probes))
	{
		//send as many probes as possible
		if (cfg->pending_probes < cfg->sprobes
			&& cfg->probe_id < cfg->max_probes)
			err = send_probes(cfg);
		//read responses if any
		//check pending probes and set timeouts if needed
		//print current hop if it is completed
	}
	if (err)
		ft_exit(err, EXIT_FAILURE);
}
