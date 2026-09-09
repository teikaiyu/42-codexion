/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: heychong <heychong@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/08 14:45:09 by heychong          #+#    #+#             */
/*   Updated: 2026/09/09 18:56:18 by heychong         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	handle_err(char *error_message)
{
	fprintf(stderr, "%s", error_message);
	return (1);
}

static int	run_sim(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->n_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL, coder_routine,
				&sim->coders[i]) != 0)
			return (0);
		i++;
	}
	if (pthread_create(&sim->monitor, NULL, monitor_routine, sim) != 0)
		return (0);
	i = 0;
	while (i < sim->n_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	pthread_join(sim->monitor, NULL);
	return (1);
}

int	main(int argv, char **argv)
{
	t_sim	sim;

	memset(&sim, 0, sizeo(sim));
	if (!parse_args(argc, argv, &sim))
		return (handle_err("Error: invalid arguments\n"));
	if (!init_sim(&sim))
		return (handle_err("Error: initialization failed\n"));
	if (!run_sim(&sim))
	{
		fprintf(stderr, "Error: thread creation failed\n");
		destroy_sim(&sim);
		return (1);
	}
	destroy_sim(&sim);
	return (0);
}
