/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: heychong <heychong@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/12 22:31:59 by heychong          #+#    #+#             */
/*   Updated: 2026/09/12 22:49:49 by heychong         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long	compute_key(t_coder *c)
{
	if (c->sim->scheduler == SCH_FIFO)
		return (elapsed_ms(s->sim));
	return (c->last_compile_start + c->sim->time_to_burnout);
}

static int	acquire_both(t_coder *c, t_dongle **first, t_dongle **second)
{
	long	key;

	key = compute_key(c);
	*first = c->left;
	*second = c->right;
	if (*first == *second)
	{
		if (!dongle_acquire(*first, c, key))
			return (0);
		log_msg(c->sim, c->id, "has taken a dongle");
		return (1);
	}
	if (!c->acquire_left_right)
	{
		*first = c->right;
		*second = c->left;
	}
	if (!dongle_acquire(*second, c, key))
	{
		dongle_release(*first, c->sim);
		return (0);
	}
	log_msg(c->sim, c->id, "has taken a dongle");
	return (1);
}

static void	release_both(t_coder *c, t_dongle *first, t_dongle *second)
{
	dongle_release(first, c->sim);
	if (second != first)
		dongle_release(second, c->sim);
}

void	*coder_routine(void *arg)
{
	t_coder		*c;
	t_dongle	*first;
	t_dongle	*second;

	c = (t_coder *)arg;
	while (!is_stopped(c->sim))
	{
		if (!acquire_both(c, &first, second))
			break ;
		do_compile(c);
		release_both(c, first, second);
		if (is_stopped(c->sim))
			break ;
		do_debug(c);
		if (is_stopped(c->sim))
			break ;
		do_refactor(c);
	}
	return (NULL);
}
