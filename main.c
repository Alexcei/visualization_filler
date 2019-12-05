#include "filler_visual.h"

static int		fdf_close(void *data)
{
	(void)data;
	exit(EXIT_SUCCESS);
}

static void		free_all_fil(t_fil *fil)
{
	if (fil->plat)
		ft_free_char_arr(&fil->plat);
	if (fil->pie)
		ft_free_char_arr(&fil->pie);
	if (fil->map)
		ft_free_int_arr(&fil->map, fil->h_plat);
}

static char		*get_player(t_fil *fil)
{
	char 	**arr;
	char 	*player;
	int 	i;

	i = 0;
	arr = ft_double_split(fil->line, '/', '.');
	if (!arr)
		print_error(fil, "error_read_player");
	while (arr[i])
		i++;
	if (i < 2)
		print_error(fil, "error_read_player");
	player = ft_strdup(arr[i - 2]);
	ft_free_char_arr(&arr);
	return (player);
}

static void		read_player(t_fil *fil)
{
	while (!fil->play_two)
	{
		get_next_line(fil->fd, &fil->line);
		if (!fil->line)
			print_error(fil, "error_read_player");
		if (ft_strnequ(fil->line, "$$$ exec p1", 11))
			fil->play_one = get_player(fil);
		else if (ft_strnequ(fil->line, "$$$ exec p2", 11))
			fil->play_two = get_player(fil);
		ft_strdel(&fil->line);
	}
	fil->n_play = 1;
}

static int		fdf_init(t_data *data, t_camera *camera, t_fil *fil)
{
	ft_bzero(camera, sizeof(t_camera));
	ft_bzero(data, sizeof(t_data));
	ft_bzero(fil, sizeof(t_fil));
	data->camera = camera;
	data->fil = fil;
	if (!(data->mlx = mlx_init()) ||
		!(data->win = mlx_new_window(data->mlx, WIDTH, HEIGHT, "FDF")) ||
		!(data->img = mlx_new_image(data->mlx, WIDTH, HEIGHT)))
		return (0);
	data->camera->x_offset = WIDTH / 2;
	data->camera->y_offset = HEIGHT / 2;
	camera->view_selection = 0;
	data->camera->color_selection = 2;
	return (1);
}

void		print_final(t_data *data)
{
	char	*str;
	int 	len;

	len = ft_strlen(data->fil->play_one);
	str = "IS WIN";
	mlx_string_put(data->mlx, data->win, (WIDTH / 4) - 10, (HEIGHT / 2) - 10,
				   0x0FFFFFF, data->fil->play_one);
	mlx_string_put(data->mlx, data->win, (WIDTH / 4) - 10 + 5 + len, (HEIGHT / 2) - 10,
				   0x0FFFFFF, str);
}

//static void		render_background(t_data *data)
//{
//	int 		i;
//
//	i = 0;
//	ft_bzero(data->data_addr, sizeof(char) * SIZE);
//	while (i < SIZE)
//		((int*)(data->data_addr))[i++] = BACKGROUND;
//}



void			ft_creat_fdf(t_data *data)
{
	int 	height;
	int 	wigth;

	data->width = data->fil->w_plat;
	data->height = data->fil->h_plat;
	if (!data->camera->zoom)
		data->camera->zoom = FT_MIN(WIDTH / data->width / 2, HEIGHT / data->height / 2);
	data->size = data->width * data->height;
	data->dot = (t_dot*)ft_memalloc(sizeof(t_dot) * data->size);
	height = 0;
	while (height < data->height)
	{
		wigth = 0;
		while(wigth < data->width)
		{
			data->dot[height * data->width + wigth].z = - data->fil->map[height][wigth];    // * (-1) + data->max_i;
			data->dot[height * data->width + wigth].x = (double)wigth;
			data->dot[height * data->width + wigth].y = (double)height;
			if (data->fil->plat[height][wigth] == 'x' || data->fil->plat[height][wigth] == 'X')
				data->dot[height * data->width + wigth].color = RED;
			else if (data->fil->plat[height][wigth] == 'o' || data->fil->plat[height][wigth] == 'O')
				data->dot[height * data->width + wigth].color = BLUE;
			else
				data->dot[height * data->width + wigth].color = 0;
			wigth++;
		}
		height++;
	}
}

void			ft_creat_image(t_data *data)
{
	int 	i;

	i = 1;
	if (!(ft_creat_heat_map(data->fil)))
		print_error(data->fil, "error_memmory_head_map");
	ft_put_players_on_heat_map(data->fil);
	while (ft_fill_heat_map(data->fil, i))
		i++;
	data->max_i = i;
	ft_creat_fdf(data);
	//render_background(data);
	fdf_render(data);
	mlx_put_image_to_window(data->mlx, data->win, data->img, 0, 0);


}

static int	loop_key_hook(t_data *data)
{
	char	*str;

	if (data->pause == 0)
	{
		data->data_addr = mlx_get_data_addr(data->img, &data->bits_per_pixel,
											&data->size_line, &data->endian);
		if (!data->fil->read)
		{
			while (!data->fil->read && !ft_read_fil(data->fil))
				;
			if (data->fil->plat)
			{
				ft_creat_image(data);
				free_all_fil(data->fil);
			}
		}
		mlx_put_image_to_window(data->mlx, data->win, data->img, 0, 0);
		mlx_destroy_image(data->mlx, data->img);
		data->img = mlx_new_image(data->mlx, WIDTH, HEIGHT);
		//render_menu(data);
		if (data->fil->read)
			print_final(data);
	}
	if (data->pause == 1)
	{
		str = "PAUSE";
		mlx_string_put(data->mlx, data->win, (WIDTH / 4) - 10, (HEIGHT / 2) - 10,
					   0x0FFFFFF, str);
	}
	return (0);
}

int		main(void)
{
	t_camera	camera;
	t_data		data;
	t_fil	fil;

	if (!fdf_init(&data, &camera, &fil))
		print_error(&fil, "error: initialization");
	data.fil->fd = open("t2", O_RDONLY);
	read_player(&fil);
	mlx_key_hook(data.win, fdf_hook_keydown, &data);
	mlx_hook(data.win, 17, 0, fdf_close, &data);
	loop_key_hook(&data);
	mlx_loop_hook(data.mlx, loop_key_hook, &data);
	mlx_loop(&data.mlx);
	return 0;
}