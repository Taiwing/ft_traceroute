/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/06 23:38:42 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/08 00:16:05 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

void		intopt(int *dest, t_optdata *optd, int min, int max)
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
	t_optdata	optd;
	char		**args;

	ft_bzero((void *)&optd, sizeof(t_optdata));
	init_getopt(&optd, FT_TRACEROUTE_OPT, NULL, NULL);
	args = ft_memalloc((argc + 1) * sizeof(char *));
	ft_memcpy((void *)args, (void *)argv, argc * sizeof(char *));
	*args = (char *)cfg->exec;
	while ((opt = ft_getopt(argc, args, &optd)) >= 0 || argc == 1)
		switch (opt)
		{
			case 'm': intopt(&cfg->max_ttl, &optd, 0, 255);	break;
			default:
				ft_printf(FT_TRACEROUTE_HELP, cfg->exec);
				ft_exit(NULL, opt != 'h');
		}
	ft_memdel((void **)&args);
	return (argv[optd.optind]);
}

int			main(int argc, char **argv)
{
	t_trcrt_config	cfg = { 0 };

	(void)argc;
	cfg.exec = ft_exec_name(*argv);
	ft_exitmsg((char *)cfg.exec);
	cfg.max_ttl = MAX_TTL_DEF;
	cfg.dest = get_options(&cfg, argc, argv);
	ft_printf("This is %s!\n", cfg.exec);
	ft_printf("traceroute to %s (%s), %d hops max, 60 byte packets\n",
		cfg.dest, "IP_PLACEHOLDER", cfg.max_ttl);
	return (EXIT_SUCCESS);
}
