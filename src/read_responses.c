/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_responses.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/12 17:50:49 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/12 21:16:59 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

/*
** ts_diff: timestamp substraction
**
** Substracts b from a and put the result in res. a must therefore be bigger
** than b (so a later timestamp), otherwise ts_diff will return -1 and res
** will be set to 0.
*/
int			ts_diff(struct timeval *res, struct timeval *a, struct timeval *b)
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
		ft_asprintf(&err, "recvfrom: %s", strerror(errno));
	if (err || rd < (int)RESP_HEADERS_SEQ
		|| (id = check_resp(cfg, &resp, &respip)) < 0)
		return (err);
	ft_memcpy((void *)&cfg->probes[id].received_ts, (void *)ts, sizeof(*ts));
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
	return (err);
}

char		*read_responses(t_trcrt_config *cfg)
{
	fd_set			rfds;
	int				sret = 0;
	char			*err = NULL;
	struct timeval	before = { 0 }, after = { 0 }, to = { 0 };

	FD_ZERO(&rfds);
	FD_SET(cfg->recv_socket, &rfds);
	if (!err && gettimeofday(&before, NULL) < 0)
		ft_asprintf(&err, "gettimeofday: %s", strerror(errno));
	while (!err && !to.tv_sec && to.tv_usec < TRCRT_SLCT_TMOUT
		&& (sret = select(cfg->recv_socket + 1, &rfds, NULL, NULL, &to)) > 0)
	{
		if (!err && gettimeofday(&after, NULL) < 0)
			ft_asprintf(&err, "gettimeofday: %s", strerror(errno));
		else
			ts_diff(&to, &after, &before);
		if (!err)
			err = read_response(cfg, &after);
		FD_ZERO(&rfds);
		FD_SET(cfg->recv_socket, &rfds);
	}
	if (sret < 0)
		ft_asprintf(&err, "select: %s", strerror(errno));
	return (err);
}
