/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: heychong <heychong@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/12 23:53:56 by heychong          #+#    #+#             */
/*   Updated: 2026/09/13 00:25:13 by heychong         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	get_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000L + tv.tv_usec / 1000L);
}

long	elapsed_ms(t_sim *sim)
{
	return (get_ms() - sim->start_ts);
}

int	is_stopped(t_sim *sim)
{
	int	val;

	pthread_mutex_lock(&sim->state_lock);
	val = sim->stop_flag;
	pthread_mutex_unlock(&sim->state_lock);
	return (val);
}

void	log_msg(t_sim *sim, int id, const char *msg)
{
	pthread_mutex_lock(&sim->log_lock);
	printf("%ld %d %s\n", elapsed_ms(sim), id, msg);
	pthread_mutex_unlock(&sim->log_lock);
}
