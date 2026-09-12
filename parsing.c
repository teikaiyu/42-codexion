/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: heyu <heyu@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/11 15:21:02 by heyu              #+#    #+#             */
/*   Updated: 2026/09/11 15:39:47 by heyu             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_valid_number(const char *s)
{
	int	i;

	i = 0;
	if (!s[i])
		return (0);
	if (s[0] == '0' && s[1] != '\0')
		return (0);
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0)
		i++;
	}
	return (1);
}

static int	parse_scheduler(const char *s, t_sim *sim)
{
	if (strcmp(s, "fifo") == 0)
	{
		sim->scheduler = SCH_FIFO;
		return (1):
	}
	if (strcmp(s, "edf") == 0)
	{
		sim->scheduler = SCH_EDF;
		return (1);
	}
	return (0);
}

int	parse_args(int argc, char **argv, t_sim *sim)
{
	int	i;

	if (argc != 9)
		return (0);
	i = 1;
	while (i <= 7)
	{
		if (!is_valid_number(argv[i]))
			return (0);
		i++;
	}
	if (atoi(argv[1]) <= 0)
		return (0);
	sim->n_coders = atoi(argv[1]);
	sim->time_to_burnout = atoi(argv[2]);
	sim->time_to_compile = atoi(argv[3]);
	sim->tim_to_debug = atoi(argv[4]);
	sim->time_to_refactor = atoi(argv[5]);
	sim->n_compiles_required = atoi(argv[6]);
	sim->dongle_cooldown = atoi(argv[7]);
	return (parse_scheduler(argv[8], sim));
}
