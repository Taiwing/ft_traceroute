/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_responses.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/12 17:50:49 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/12 17:51:50 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

static int	check_resp(t_trcrt_config *cfg, t_icmp_packet *resp,
	struct sockaddr_in *respip)
{
	int			id, seqid;
	uint16_t	seq, port;

	ft_memcpy((void *)&seq, (void *)resp + RESP_HEADERS, sizeof(uint16_t));
	port = ntohs(resp->data_udp.dstp);
	id = cfg->port <= (int)port ? (int)port - cfg->port
		: USHRT_MAX + 1 - cfg->port + (int)port;
	seqid = cfg->pid <= (int)seq ? (int)seq - cfg->pid
		: USHRT_MAX + 1 - cfg->pid + (int)seq;
	if (id != seqid || cfg->probes[id].status)
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

static char	*read_response(t_trcrt_config *cfg, struct timeval *ts)
{
	int					rd, id;
	char				*err = NULL;
	t_icmp_packet		resp = { 0 };
	struct sockaddr_in	respip = { 0 };
	socklen_t			len = sizeof(respip);

	if ((rd = recvfrom(cfg->recv_socket, (void *)&resp, sizeof(resp), 0,
		(struct sockaddr *)&respip, &len)) < 0)
	{
		ft_asprintf(&err, "recvfrom: %s", strerror(errno));
		return (err);
	}
	else if (rd < (int)RESP_HEADERS_SEQ
		|| (id = check_resp(cfg, &resp, &respip)) < 0)
		return (NULL);
	ft_memcpy((void *)&cfg->probes[id].received_ts, (void *)ts, sizeof(*ts));
	if (resp.icmp.type == ICMP_TIME_EXCEEDED)
		cfg->probes[id].status = E_PRSTAT_RECEIVED_TTL;
	else if (resp.icmp.code < ICMP_PORT_UNREACH)
		cfg->probes[id].status = E_PRSTAT_UNREACH_NET + resp.icmp.code;
	else if (resp.icmp.code == ICMP_PORT_UNREACH)
		cfg->probes[id].status = E_PRSTAT_RECEIVED_PORT;
	else
		cfg->probes[id].status = E_PRSTAT_UNREACH_NET + resp.icmp.code - 1;
	--cfg->pending_probes;
	ft_printf("received packet %d\n", id);
	return (err);
}

char		*read_responses(t_trcrt_config *cfg)
{
	fd_set			rfds;
	int				sret = 0;
	char			*err = NULL;
	struct timeval	before = { 0 }, after = { 0 }, to = { 0, TRCRT_TMOUT };

	FD_ZERO(&rfds);
	FD_SET(cfg->recv_socket, &rfds);
	if (!err && gettimeofday(&before, NULL) < 0)
		ft_asprintf(&err, "gettimeofday: %s", strerror(errno));
	while (!err && to.tv_usec > 0
		&& (sret = select(cfg->recv_socket + 1, &rfds, NULL, NULL, &to)) > 0)
	{
		if (!err && gettimeofday(&after, NULL) < 0)
			ft_asprintf(&err, "gettimeofday: %s", strerror(errno));
		else
			to.tv_usec -= ((after.tv_sec - before.tv_sec) * 1000000)
				+ (after.tv_usec - before.tv_usec);
		if (!err)
			err = read_response(cfg, &after);
		FD_ZERO(&rfds);
		FD_SET(cfg->recv_socket, &rfds);
	}
	if (sret < 0)
		ft_asprintf(&err, "select: %s", strerror(errno));
	return (err);
}
