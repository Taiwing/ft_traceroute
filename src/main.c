/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/06 23:38:42 by yforeau           #+#    #+#             */
/*   Updated: 2022/06/09 22:27:39 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

static void	cleanup(t_trcrt_config *cfg)
{
	if (cfg->send_socket > 0)
		close(cfg->send_socket);
	if (cfg->recv_socket > 0)
		close(cfg->recv_socket);
}

int			main(int argc, char **argv)
{
	int				ret;
	t_trcrt_config	cfg = CONFIG_DEF;
	void			cleanup_handler(void) { cleanup(&cfg); };

	ft_exitmsg((char *)cfg.exec);
	ft_atexit(cleanup_handler);
	cfg.dest = get_options(&cfg, argc, argv);
	if (getuid())
		ft_exit(EXIT_FAILURE, "user is not root");
	else if ((ret = ft_get_ip(&cfg.destip, cfg.dest, AF_INET)))
		ft_exit(EXIT_FAILURE, "%s: %s", cfg.dest, gai_strerror(ret));
	cfg.domain = cfg.destip.family;
	if ((cfg.send_socket = socket(cfg.domain, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		ft_exit(EXIT_FAILURE, "send_socket: socket: %s", strerror(errno));
	else if ((cfg.recv_socket = socket(cfg.domain, SOCK_RAW, IPPROTO_ICMP)) < 0)
		ft_exit(EXIT_FAILURE, "recv_socket: socket: %s", strerror(errno));
	ft_printf("traceroute to %s (%s), %d hops max, %zu byte packets\n",
		cfg.dest, ft_ip_str(&cfg.destip), cfg.max_ttl, PROBE_SIZE(cfg.domain));
	for (int i = 0; i < (int)PROBE_UDP_DATA_LEN(cfg.domain); ++i )
		cfg.probe_data[i] = (i % 0x40) + 0x40;
	traceroute(&cfg);
	ft_exit(EXIT_SUCCESS, NULL);
	return (EXIT_SUCCESS);
}
