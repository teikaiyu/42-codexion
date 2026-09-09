/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: heychong <heychong@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/08 14:23:27 by heychong          #+#    #+#             */
/*   Updated: 2026/09/09 18:04:08 by heychong         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

/* --- dongle --- */
typedef struct s_dongle
{
	int				id;
	int				in_use;
	long			free_since_ms; // release time, cooldown
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
}	t_dongle;

/* --- heap node (fifo/edf) --- */
typedef struct s_hnode
{
	int		coder_id;
	long	key;
}	t_hnode;

typedef struct s_heap
{
	t_hnode			*arr;
	int				size;
	int				capacity;
	pthread_mutex_t	lock;
}	t_heap;

/* --- coder state --- */
typedef enum e_state
{
	COMPILING,
	DEBUGGING,
	REFACTORING,
	WAITING,
	DEAD
}	t_state;

typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	t_state			state;
	long			last_compile_start; // burnout criteria & edf deadline calc
	int				compile_count;
	t_dongle		*left;
	t_dongle		*right;
	struct s_sim	*sim;
}	t_coder;

/* --- scheduler type --- */
typedef enum e_sched
{
	SCHED_FIFO,
	SCHED_EDF
}	t_sched;

/* --- simulation main structure --- */
typedef struct s_sim
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
	t_heap			fifo_queue;
	t_heap			edf_queue;

	long			start_ts;
	int				stop_flag;
	int				burnout_coder_id;

	pthread_mutex_t	log_lock;
	pthread_mutex_t	state_lock;
}	t_sim;