/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/06 23:38:42 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/15 19:12:56 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

static void	get_destinfo(t_trcrt_config *cfg)
{
	int					ret;
	struct addrinfo		*destinfo;
	char				*ip = NULL;
	char				*err = NULL;
	struct addrinfo		hints = { 0 };

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	if ((ret = getaddrinfo(cfg->dest, NULL, &hints, &destinfo)))
		ft_asprintf(&err, "%s: %s", cfg->dest, gai_strerror(ret));
	else
	{
		ft_memcpy((void *)&cfg->destip, (void *)destinfo->ai_addr,
			sizeof(struct sockaddr_in));
		freeaddrinfo(destinfo);
		if (!(ip = inet_ntoa(cfg->destip.sin_addr)))
			ft_asprintf(&err, "inet_ntoa: failed to convert ip to string");
		else
			ft_strncpy(cfg->destip_str, ip, INET_ADDRSTRLEN);
	}
	if (err)
		ft_exit(err, EXIT_FAILURE);
}

static void	cleanup(t_trcrt_config *cfg)
{
	if (cfg->send_socket > 0)
		close(cfg->send_socket);
	if (cfg->recv_socket > 0)
		close(cfg->recv_socket);
}

int			main(int argc, char **argv)
{
	char			*err = NULL;
	t_trcrt_config	cfg = CONFIG_DEF;
	void			cleanup_handler(void) { cleanup(&cfg); };

	ft_exitmsg((char *)cfg.exec);
	ft_atexit(cleanup_handler);
	cfg.dest = get_options(&cfg, argc, argv);
	if (getuid())
		ft_exit("user is not root", EXIT_FAILURE);
	get_destinfo(&cfg);
	if ((cfg.send_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		ft_asprintf(&err, "send_socket: socket: %s", strerror(errno));
	else if ((cfg.recv_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
		ft_asprintf(&err, "recv_socket: socket: %s", strerror(errno));
	if (err)
		ft_exit(err, EXIT_FAILURE);
	ft_printf("traceroute to %s (%s), %d hops max, %zu byte packets\n",
		cfg.dest, cfg.destip_str, cfg.max_ttl, PROBE_SIZE);
	for (int i = 2, byte = 0; i < (int)PROBE_UDP_DATA_LEN; ++i, ++byte)
		cfg.probe_data[i] = (byte % 0x40) + 0x40;
	traceroute(&cfg);
	ft_exit(NULL, EXIT_SUCCESS);
	return (EXIT_SUCCESS);
}
