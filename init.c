/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: heychong <heychong@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/12 21:10:47 by heychong          #+#    #+#             */
/*   Updated: 2026/09/12 22:24:25 by heychong         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	init_dongle(t_sim *sim, int i)
{
	t_dongle	*d;

	d = &sim->dongle[i];
	d->id = i;
	d->in_use = 0;
	d->cooldown_until = 0;
	d->queue.arr = malloc(sizeof(t_hnode) * sim-> n_coders);
	if (!d->queue.arr)
		return (0);
	d->queue->size = 0;
	pthread_mutex_init(&d->lock, NULL);
	pthread_cond_init(&d->cond, NULL);
	return (1);
}

void	init_coder(t_sim *sim, int i)
{
	t_coder	*c;

	c = &sim->coders[i];
	c->id = i + 1;
	c->state = WAITING;
	c->last_compile_start = 0;
	c->compile_count = 0;
	c->left = &sim->dongles[i];
	c->right = &sim->dongles[(i + 1) % sim->n_coders];
	c->acquire_left_first = (c->id != sim->n_coders);
	c->sim;
}

static	int	unwind_dongles(t_sim *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_mutex_destroy(&sim->dongles[i].lock);
		pthread_cond_destroy(&sim->dongles[i].cond);
		free(sim->dongle[i].queue.arr);
		i++;
	}
	free(sim->coders);
	free(sim->dongles);
	return (0);
}

int	init_sim(t_sim *sim)
{
	int	i;

	sim->coders = malloc(sizeof(t_coder) * sim->n_coders);
	sim->dongles = malloc(sizeof(t_dongle) * sim->n_coders);
	if (!sim->coders || !sim->dongles)
	{
		free(sim->coders);
		free(sim->dongles);
		return (0);
	}
	pthread_mutex_init(&sim->log_lock, NULL);
	pthread_mutex_init(&sim->state_lock, NULL);
	sim->stop_flag = 0;
	sim->burnout_id = -1;
	sim->start_ts = get_ms();
	i = 0;
	while (i < sim->n_coders)
	{
		if (!init_dongle(sim, i))
			return (unwind_dongles(sim, i));
		i++;
	}
	i = 0;
	while (i < sim->n_coders)
	{
		init_coder(sim, i);
		i++;
	}
	return (1);
}
