/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_responses.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/12 17:50:49 by yforeau           #+#    #+#             */
/*   Updated: 2022/06/13 22:14:56 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

static int	valid_ipv6_response(t_trcrt_config *cfg, t_packet *resp, t_ip *ip)
{
	if (resp->nextip->v6.nexthdr != IPPROTO_UDP)
		return (0);
	else if (resp->next->icmp6.icmp6_type == ICMPV6_TIME_EXCEED)
		return (resp->next->icmp6.icmp6_code == ICMPV6_EXC_HOPLIMIT);
	else if (resp->next->icmp6.icmp6_type == ICMPV6_DEST_UNREACH)
	{
		if (resp->next->icmp6.icmp6_code == ICMPV6_PORT_UNREACH
			&& ft_ip_cmp(ip, &cfg->destip))
			return (0);
		return (1);
	}
	return (0);
}

static int	valid_ipv4_response(t_trcrt_config *cfg, t_packet *resp, t_ip *ip)
{
	if (resp->nextip->v4.protocol != IPPROTO_UDP)
		return (0);
	else if (resp->next->icmp.type == ICMP_TIME_EXCEEDED)
		return (resp->next->icmp.code == ICMP_EXC_TTL);
	else if (resp->next->icmp.type == ICMP_DEST_UNREACH)
	{
		if (resp->next->icmp.code == ICMP_PORT_UNREACH
			&& ft_ip_cmp(ip, &cfg->destip))
			return (0);
		return (resp->next->icmp.code <= ICMP_SR_FAILED);
	}
	return (0);
}

//TODO: Agnostifying type and code wont work since the values of the ICMP and
//ICMP6 errors are not the same. This will require two different functions. One
//for IPv4 and one for IPv6.
//TODO: check headers first (we know the ip header is good since the socket will
//be in the right domain and that the packet is ICMP since it's the protocol we
//are listening to. But the second IP header and the UDP header need to be
//checked.
static int	check_resp(t_trcrt_config *cfg, t_packet *resp, t_ip *respip)
{
	int			id;
	uint16_t	port = ntohs(resp->last->udp.dest);

	id = cfg->port <= (int)port ? (int)port - cfg->port
		: 0xffff - cfg->port + (int)port - 1;
	if (id >= cfg->probe_id || id >= PROBES_MAX || cfg->probes[id].status)
		return (-1);
	if ((cfg->domain == AF_INET && !valid_ipv4_response(cfg, resp, respip))
		|| (cfg->domain == AF_INET6 && !valid_ipv6_response(cfg, resp, respip)))
		return (-1);
	return (id);
}

static enum e_probe_status	get_probe_status(t_trcrt_config *cfg, uint8_t type,
	uint8_t code)
{
	if (cfg->domain == AF_INET6)
	{
		if (type == ICMPV6_TIME_EXCEED)
			return (E_PRSTAT_RECEIVED_TTL);
		else if (code == ICMPV6_PORT_UNREACH)
			return (E_PRSTAT_RECEIVED_PORT);
		return (E_PRSTAT_UNREACH_NET);
	}
	if (type == ICMP_TIME_EXCEEDED)
		return (E_PRSTAT_RECEIVED_TTL);
	else if (code == ICMP_PORT_UNREACH)
		return (E_PRSTAT_RECEIVED_PORT);
	else if (code < ICMP_PORT_UNREACH)
		return (E_PRSTAT_UNREACH_NET + code);
	else
		return (E_PRSTAT_UNREACH_NET + code - 1);
}

static int	read_response(t_trcrt_config *cfg, char **err)
{
	int					rd, id;
	t_packet			resp = { 0 };
	t_ip				respip = { 0 };
	socklen_t			len = sizeof(respip);

	if ((rd = recvfrom(cfg->recv_socket, (void *)&resp.buf, sizeof(resp.buf), 0,
		(struct sockaddr *)&respip, &len)) < 0)
		ft_asprintf(err, "recvfrom: %s", strerror(errno));
	ft_packet_init(&resp, cfg->domain == AF_INET ? E_IH_V4 : E_IH_V6, NULL);
	if (*err || rd < (int)RESP_HEADERS(cfg->domain)
		|| (id = check_resp(cfg, &resp, &respip)) < 0)
		return (-1);
	ft_memcpy((void *)&cfg->probes[id].received_ip,
		(void *)&respip, sizeof(t_ip));
	cfg->probes[id].status = get_probe_status(cfg, cfg->domain == AF_INET ?
		resp.next->icmp.type : resp.next->icmp6.icmp6_type,
		cfg->domain == AF_INET ?
		resp.next->icmp.code : resp.next->icmp6.icmp6_code);
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
