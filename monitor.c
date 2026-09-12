/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: heychong <heychong@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/12 23:21:11 by heychong          #+#    #+#             */
/*   Updated: 2026/09/12 23:31:41 by heychong         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	check_burnout(t_sim *sim, t_coder *c, long now)
{
	if (now - c->last_compile_start > sim->time_to_burnout)
	{
		pthread_mutex_lock(&sim->state_lock);
		if (!sim->stop_flag)
		{
			sim->stop_flag = 1;
			sim->burnout_id = c->id;
			pthread_mutex_unlock(&sim->state_lock);
			log_msg(sim, c->id, "burned out");
			return (1);
		}
		pthread_mutex_unlock(&sim->state_lock);
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	int		i;
	long	now;

	sim = (t_sim *)arg;
	while (!is_stopped(sim))
	{
		now = elapsed_ms(sim);
		i = 0;
		while (i < sim->n_coders && !is_stopped(sim))
		{
			if (check_burnout(sim, *sim->coders[i], now))
				break ;
			i++;
		}
		usleep(1000);
	}
	return (NULL);
}
