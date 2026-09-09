/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: heychong <heychong@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/08 14:45:09 by heychong          #+#    #+#             */
/*   Updated: 2026/09/09 18:04:46 by heychong         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(void)
{
	int	n_coder;
	int	i;

	n_coder = 1;
	i = 0;
	while (i < n_coder)
	{
		i++;
		printf("%d has taken a dongle\n", i);
		printf("%d is compiling\n", i);
		printf("%d is debugging\n", i);
		printf("%d is refactoring\n", i);
	}
}
