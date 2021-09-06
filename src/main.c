/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/06 23:38:42 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/07 00:00:32 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_traceroute.h"

int	main(int argc, char **argv)
{
	t_trcrt_config	cfg = { 0 };

	(void)argc;
	cfg.exec = ft_exec_name(*argv);
	ft_printf("This is %s!\n", cfg.exec);
	return (EXIT_SUCCESS);
}
