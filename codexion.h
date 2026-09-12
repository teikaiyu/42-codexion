/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: heychong <heychong@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/08 14:23:27 by heychong          #+#    #+#             */
/*   Updated: 2026/09/12 22:48:47 by heychong         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <time.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>

/* scheduler type */
typedef enum e_sched
{
	SCH_FIFO,
	SCH_EDF
}	t_sched;

/* coder state */
typedef enum e_state
{
	WAITING,
	COMPILING,
	DEBUGGING,
	REFACTORING,
	DEAD
}	t_state;

/* heap node (fifo/edf) */
typedef struct s_hnode
{
	int		coder_id;
	long	key;
}	t_hnode;

typedef struct s_heap
{
	t_hnode	*arr;
	int		size;
}	t_heap;

typedef struct s_dongle
{
	int				id;
	int				in_use;
	long			cooldown_until;
	t_heap			queue;
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
}	t_dongle;

typedef struct s_sim	t_sim;

typedef struct s_coder
{
	int			id;
	pthread_t	thread;
	t_state		state;
	long		last_compile_start; // burnout criteria & edf deadline calc
	int			compile_count;
	t_dongle	*left;
	t_dongle	*right;
	int			acquire_left_first;
	t_sim		*sim;
}	t_coder;

/* simulation main structure */
struct s_sim
{
	int				n_coders;
	long			time_to_burnout;
	long			time_to_compile;
	long			time_to_debug;
	long			time_to_refactor;
	int				n_compiles_required;
	long			dongle_cooldown;
	t_sched			scheduler;

	t_coder			*coders;
	t_dongle		*dongles;

	long			start_ts;
	int				stop_flag;
	int				burnout_id;

	p_thread_t		monitor;
	pthread_mutex_t	log_lock;
	pthread_mutex_t	state_lock;
};

/* parsing */
int		parse_args(int argc, char **argv, t_sim *sim);

/* init */
int		init_sim(t_sim *sim);
void	init_coder(t_sim *sim, int i);
int		init_dongle(t_sim *sim, int i);

/* cleanup */
void	destroy_sim(t_sim *sim);

/* heap */
void	heap_push(t_heap *h, int coder_id, long key);
int		heap_pop(t_heap *h);
int		heap_peek(t_heap *h);

/* dongle */
int		dongle_acquire(t_dongle *d, t_coder *c, long key);
void	dongle_release(t_dongle *d, t_sim *sim);

/* coder */
void	*coder_routine(void *arg);

/* coder_phases */
void	do_compile(t_coder *c);
void	do_debug(t_coder *c);
void	do_refactor(t_coder *c);
void	sleep_ms(t_sim *sim, long ms);

/* monitor */
void	*monitor_routine(void *arg);

/* utils */
long	get_ms(void);
long	elapsed_ms(t_sim *sim);
int		is_stopped(t_sim *sim);
void	log_msg(t_sim *sim, int id, const cha *msg);

#endif