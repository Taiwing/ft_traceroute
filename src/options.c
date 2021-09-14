/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   options.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/14 18:03:02 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/14 18:56:38 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

static char	*valid_float_string(char *str)
{
	char	*start;

	while (ft_isspace(*str))
		++str;
	start = str;
	while (ft_isdigit(*str))
		++str;
	if (*str == '.')
		++str;
	while (ft_isdigit(*str))
		++str;
	if (str == start || (str - start == 1 && *start == '.'))
		return (NULL);
	return (str);
}

#define	MAX_FLOAT_STR_LEN	64

static void	floatopt(double *dest, char *str, size_t len)
{
	char	buf[MAX_FLOAT_STR_LEN + 1];

	len = len > MAX_FLOAT_STR_LEN ? MAX_FLOAT_STR_LEN : len;
	ft_strncpy(buf, str, len);
	buf[len] = 0;
	*dest = atof(buf);
}

#define	WAIT_TIMES_SEPS		"/,"

static void	set_wait_times(t_trcrt_config *cfg, char *arg)
{
	char	*err = NULL;
	char	*start, *end;

	if (!(end = valid_float_string(arg))
		|| (*end && !ft_strchr(WAIT_TIMES_SEPS, *end)))
		ft_asprintf(&err, "invalid argument: '%s'", arg);
	else
		floatopt(&cfg->max, arg, end - arg);
	start = end;
	if (!err && ((*end && !(end = valid_float_string(++start)))
		|| (*end && !ft_strchr(WAIT_TIMES_SEPS, *end))))
		ft_asprintf(&err, "invalid argument: '%s'", arg);
	else if (!err)
		floatopt(&cfg->here, start, end - start);
	start = end;
	if (!err && ((*end && !(end = valid_float_string(++start))) || *end))
		ft_asprintf(&err, "invalid argument: '%s'", arg);
	else if (!err)
		floatopt(&cfg->near, start, end - start);
	if (err)
		ft_exit(err, EXIT_FAILURE);
	ft_printf("max: %g\nhere: %g\nnear: %g\n", cfg->max, cfg->here, cfg->near);
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

char		*get_options(t_trcrt_config *cfg, int argc, char **argv)
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
			case 'w': set_wait_times(cfg, optd.optarg);					break;
			default:
				ft_printf(FT_TRACEROUTE_HELP, cfg->exec, MAX_TTL_DEF,
					SPROBES_DEF, PORT_DEF, NPROBES_DEF, MAX_DEF, HERE_DEF,
					NEAR_DEF);
				ft_exit(NULL, opt != 'h');
		}
	ft_memdel((void **)&args);
	return (argv[optd.optind]);
}
