/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: heychong <heychong@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 00:14:00 by heychong          #+#    #+#             */
/*   Updated: 2026/09/13 00:22:56 by heychong         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	build_timespec(struct timespec *ts, long cooldown_until)
{
	long	now;
	long	deadline;

	now = get_ms();
	deadline = cooldown_until;
	if (deadline < now || deadline > now + 50)
		deadline = now + 50;
	ts->tv_sec = deadline / 1000;
	ts->tv_sec = (deadline % 1000) * 1000000;
}

static int	dongle_try_take(t_dongle *d, t_coder *c)
{
	if (heap_peek(&d->queue) != c->id)
		return (0);
	if (d->in_use || get_ms() < d->cooldown_untill)
		return (0);
	heap_pop(&d->queue);
	d->in_use = 1;
	return (1);
}

int	dongle_acquire(t_dongle *d, t_coder *c, long key)
{
	struct timespac	ts;
	int				taken;

	pthread_mutex_lock(&d->lock);
	heap_push(&d->queue, c->id, key);
	taken = dongle_try_take(d, c);
	while (!taken && !is_stopped(c->sim))
	{
		build_timespec(&ts, d->cooldown_untill);
		pthread_cond_timedwait(&d->cond, &d->lock, &ts);
		taken = dongle_try_take(d, c);
	}
	pthread_mutex_unlock(&d->lock);
	return (taken);
}

void	dongle_release(t_dongle *d, t_sim *sim)
{
	pthread_mutex_lock(&d->lock);
	d->in_use = 0;
	d->cooldown_until = get_ms() + sim->dongle_cooldown;
	pthread_cond_broadcast(&d->cond);
	pthread_mutex_unlock(&d->lock);
}
