/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_responses.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/12 17:50:49 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/15 22:39:55 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

static int	check_resp(t_trcrt_config *cfg, t_icmp_packet *resp,
	struct sockaddr_in *respip)
{
	int			id, seqid;
	uint16_t	seq, port;

	ft_memcpy((void *)&seq, (void *)resp->data, sizeof(uint16_t));
	port = ntohs(resp->data_udp.dstp);
	id = cfg->port <= (int)port ? (int)port - cfg->port
		: 0xffff - cfg->port + (int)port;
	seqid = cfg->ident <= (int)seq ? (int)seq - cfg->ident
		: 0xffff - cfg->ident + (int)seq;
	if ((id != seqid && resp->icmp.type != ICMP_DEST_UNREACH)
		|| id >= PROBES_MAX || cfg->probes[id].status)
		return (-1);
	else if (resp->icmp.type == ICMP_TIME_EXCEEDED)
		return (resp->icmp.code == ICMP_EXC_TTL ? id : -1);
	else if (resp->icmp.type == ICMP_DEST_UNREACH)
	{
		if (resp->icmp.code == ICMP_PORT_UNREACH
			&& respip->sin_addr.s_addr != cfg->destip.sin_addr.s_addr)
			return (-1);
		return (resp->icmp.code <= ICMP_SR_FAILED ? id : -1);
	}
	return (-1);
}

static int	read_response(t_trcrt_config *cfg, char **err)
{
	int					rd, id;
	t_icmp_packet		resp = { 0 };
	struct sockaddr_in	respip = { 0 };
	socklen_t			len = sizeof(respip);

	if ((rd = recvfrom(cfg->recv_socket, (void *)&resp, sizeof(resp), 0,
		(struct sockaddr *)&respip, &len)) < 0)
		ft_asprintf(err, "recvfrom: %s", strerror(errno));
	if (*err || rd < (int)RESP_HEADERS
		|| (id = check_resp(cfg, &resp, &respip)) < 0)
		return (-1);
	ft_memcpy((void *)&cfg->probes[id].received_ip, (void *)&respip.sin_addr,
		sizeof(struct in_addr));
	if (resp.icmp.type == ICMP_TIME_EXCEEDED)
		cfg->probes[id].status = E_PRSTAT_RECEIVED_TTL;
	else if (resp.icmp.code < ICMP_PORT_UNREACH)
		cfg->probes[id].status = E_PRSTAT_UNREACH_NET + resp.icmp.code;
	else if (resp.icmp.code == ICMP_PORT_UNREACH)
		cfg->probes[id].status = E_PRSTAT_RECEIVED_PORT;
	else
		cfg->probes[id].status = E_PRSTAT_UNREACH_NET + resp.icmp.code - 1;
	--cfg->pending_probes;
	return (id);
}

char		*read_responses(t_trcrt_config *cfg)
{
	fd_set			rfds;
	char			*err = NULL;
	int				sret = 0, id;
	struct timeval	after = { 0 }, to = { 0, SLCT_TMOUT };

	FD_ZERO(&rfds);
	FD_SET(cfg->recv_socket, &rfds);
	while (!err && cfg->pending_probes
		&& (sret = select(cfg->recv_socket + 1, &rfds, NULL, NULL, &to)) > 0)
	{
		if ((id = read_response(cfg, &err)) >= 0
			&& gettimeofday(&after, NULL) < 0)
			ft_asprintf(&err, "gettimeofday: %s", strerror(errno));
		else if (!err && id >= 0)
			ft_memcpy((void *)&cfg->probes[id].received_ts,
				(void *)&after, sizeof(struct timeval));
		FD_ZERO(&rfds);
		FD_SET(cfg->recv_socket, &rfds);
	}
	if (sret < 0)
		ft_asprintf(&err, "select: %s", strerror(errno));
	return (err);
}
