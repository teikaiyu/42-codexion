/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: heychong <heychong@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/13 00:25:37 by heychong          #+#    #+#             */
/*   Updated: 2026/09/13 00:36:02 by heychong         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	swap_node(t_hnode *a, t_hnode *b)
{
	t_hnode	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

static int	has_priority(t_hnode a, t_hnode b)
{
	if (a.key != b.key)
		return (a.key < b.key);
	return (a.coder_id < b.coder_id);
}

void	heap_push(t_heap *h, int coder_id, long key)
{
	int	idx;
	int	parent;

	h->arr[h->size].coder_id = coder_id;
	h->arr[h->size].key = key;
	idx = h->size;
	h->size++;
	while (idx > 0)
	{
		parent = (idx - 1) / 2;
		if (!has_priority(h->arr[idx], h->arr[parent]))
			break ;
		swap_node(&h->arr[idx], &h->arr[parent]);
		idx = parent;
	}
}

int	heap_peek(t_heap *h)
{
	if (h->size == 0)
		return (-1);
	return (h->arr[0].coder_id);
}

int	heap_pop(t_heap *h)
{
	int	id;
	int	idx;
	int	left;
	int	right;
	int	smallest;

	if (h->size == 0)
		return (-1);
	id = h->arr[0].coder_id;
	h->size--;
	h->arr[0] = h->arr[h->size];
	idx = 0;
	while (1)
	{
		left = 2 * idx + 1;
		right = 2 * idx + 2;
		smallest = idx;
		if (left < h->size && has_priority(h->arr[left], h->arr[smallest]))
			smallest = left;
		if (right < h->size && has_priority(h->arr[right], h->arr[smallest]))
			smallest = right;
		if (smallest == idx)
			break ;
		swap_node(&h->arr[idx], &h->arr[smallest]);
		idx = smallest;
	}
	return (id);
}
