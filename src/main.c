/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/06 23:38:42 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/11 12:50:35 by yforeau          ###   ########.fr       */
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
			sizeof(struct sockaddr));
		freeaddrinfo(destinfo);
		if (!(ip = inet_ntoa(((struct sockaddr_in *)&cfg->destip)->sin_addr)))
			ft_asprintf(&err, "inet_ntoa: failed to convert ip to string");
		else
			ft_strncpy(cfg->destip_str, ip, INET_ADDRSTRLEN);
	}
	if (err)
		ft_exit(err, EXIT_FAILURE);
}

static void	intopt(int *dest, t_optdata *optd, int min, int max)
{
	int		ret;
	char	*err;

	if ((ret = ft_secatoi(dest, min, max, optd->optarg)))
	{
		if (ret == FT_E_NOT_A_NUMBER)
			ft_asprintf(&err, "invalid argument: '%s'", optd->optarg);
		else
			ft_asprintf(&err, "invalid argument: '%s': "
				"out of range: %d <= value <= %d", optd->optarg, min, max);
		ft_exit(err, EXIT_FAILURE);
	}
}

static char	*get_options(t_trcrt_config *cfg, int argc, char **argv)
{
	int			opt;
	char		**args;
	t_optdata	optd = { 0 };

	init_getopt(&optd, FT_TRACEROUTE_OPT, NULL, NULL);
	args = ft_memalloc((argc + 1) * sizeof(char *));
	ft_memcpy((void *)args, (void *)argv, argc * sizeof(char *));
	*args = (char *)cfg->exec;
	while ((opt = ft_getopt(argc, args, &optd)) >= 0 || !argv[optd.optind])
		switch (opt)
		{
			case 'm': intopt(&cfg->max_ttl, &optd, 1, MAX_TTL_MAX);		break;
			case 'N': intopt(&cfg->sprobes, &optd, 1, SPROBES_MAX);		break;
			case 'p': intopt(&cfg->port, &optd, 0, USHRT_MAX);			break;
			case 'q': intopt(&cfg->nprobes, &optd, 1, NPROBES_MAX);		break;
			default:
				ft_printf(FT_TRACEROUTE_HELP, cfg->exec);
				ft_exit(NULL, opt != 'h');
		}
	ft_memdel((void **)&args);
	return (argv[optd.optind]);
}

int			main(int argc, char **argv)
{
	t_trcrt_config	cfg = CONFIG_DEF;

	ft_exitmsg((char *)cfg.exec);
	cfg.dest = get_options(&cfg, argc, argv);
	get_destinfo(&cfg);
	ft_printf("This is %s!\n", cfg.exec);
	ft_printf("traceroute to %s (%s), %d hops max, %zu byte packets\n",
		cfg.dest, cfg.destip_str, cfg.max_ttl, sizeof(t_probe_packet));
	return (EXIT_SUCCESS);
}
