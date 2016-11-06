/*

Code to convert map_init into w_init and w,
 after generation in g_world.map.


*/

#include <allegro5/allegro.h>

#include <stdio.h>
#include <math.h>

#include "m_config.h"
#include "m_globvars.h"

#include "g_header.h"

#include "g_misc.h"

#include "m_maths.h"
#include "i_background.h"
#include "g_world_back.h"
#include "g_world_map.h"
#include "g_world_map_2.h"

extern struct world_init_struct w_init; // this is the world_init generated by world setup menus - declared in s_menu.c
extern struct map_init_struct map_init;





static void add_data_well(int block_x, int block_y);
//static void circle_around_data_well(int centre_block_x, int centre_block_y, int clear_size, int inner_edge_width, int middle_edge_width, int outer_edge_width);
//static void init_hex_block_nodes(int x, int y);
static void clear_hex_block_nodes(int x, int y);
static void base_background_uniform(void);
static void raise_background(int centre_x, int centre_y, int main_size, int edge_size);
static void block_node_line(int start_x, int start_y, int end_x, int end_y, int line_main_size, int line_edge_size);
static void block_node_circle(int centre_block_x,
																														int centre_block_y,
																														int centre_size,
																														int inner_edge_width,
																														int middle_edge_width,
																														int outer_edge_width,
																														int centre_type,
																														int inner_edge_type,
																														int middle_edge_type,
																														int outer_edge_type);

void block_node_system(int centre_block_x, int centre_block_y, int system_size);

// finalises world background and data wells from map_init()
// the map generation code may exit before this e.g. if called from s_menu during custom game creation
void generate_map_from_map_init(void)
{

 w.data_wells = 0;

 int i;//, j;
/*
  for (i = 0; i < map_init.map_size_blocks; i ++)
  {
   for (j = 0; j < map_init.map_size_blocks; j ++)
   {
    init_hex_block_nodes(i, j);
   }
  }
*/
map_init.background_size_random_freq = 0;
map_init.background_depth_random_freq = 0;
 base_background_uniform();

 for (i = 0; i < w.players; i ++)
	{
		w.player[i].spawn_position.x = block_to_fixed(map_init.spawn_position[i].x) + BLOCK_SIZE_FIXED / 2;
		w.player[i].spawn_position.y = block_to_fixed(map_init.spawn_position[i].y) + BLOCK_SIZE_FIXED / 2;
		w.player[i].spawn_angle = map_init.spawn_angle[i];
	}

	for (i = 0; i < MDETAILS; i ++)
	{
		if (map_init.mdetail[i].type == MDETAIL_NONE)
			break;

		switch(map_init.mdetail[i].type)
		{
		 case MDETAIL_LINE:
		 	block_node_line(map_init.mdetail[i].block_position.x,
																				map_init.mdetail[i].block_position.y,
																				map_init.mdetail[i].block_position2.x,
																				map_init.mdetail[i].block_position2.y,
																				map_init.mdetail[i].dsize,
																				map_init.mdetail[i].dsize * 2);
				break;
			case MDETAIL_RING:
 block_node_circle(map_init.mdetail[i].block_position.x,
																			map_init.mdetail[i].block_position.y,
																			map_init.mdetail[i].dsize, 4, 1, 5,
																			BNC_CENTRE_EMPTY,
																			BNC_INNER_SCATTERED_GRADIENT,
																			BNC_MIDDLE_RAISED,
																			BNC_OUTER_RANDOM_GRADIENT);
				break;

			case MDETAIL_SYSTEM_CLEAR:
    clear_background_circle(map_init.mdetail[i].block_position.x,
																			         map_init.mdetail[i].block_position.y,
																			         map_init.mdetail[i].dsize + 2, 2);
// fall-through...
			case MDETAIL_SYSTEM:
 block_node_system(map_init.mdetail[i].block_position.x,
																			map_init.mdetail[i].block_position.y,
																			map_init.mdetail[i].dsize);
				break;




		}

	}


// data wells come last and overwrite everything else (they need to, to make sure the special backblock values they use are retained)

	for (i = 0; i < map_init.data_wells; i ++)
	{
  add_data_well(map_init.data_well_position[i].x, map_init.data_well_position[i].y);
	}

}




void add_data_well(int block_x, int block_y)
{

#ifdef SANITY_CHECK
 if (w.data_wells >= DATA_WELLS)
	{
		fpr("\nError: g_world.c: add_data_well(): too many data wells (map_init.data_wells %i).", map_init.data_wells);
		error_call();
	}
#endif

	w.data_well[w.data_wells].active = 1;
	w.data_well[w.data_wells].block_position.x = block_x;
	w.data_well[w.data_wells].block_position.y = block_y;
	w.data_well[w.data_wells].position.x = al_itofix((block_x * BLOCK_SIZE_PIXELS) + BLOCK_SIZE_PIXELS / 2);
	w.data_well[w.data_wells].position.y = al_itofix((block_y * BLOCK_SIZE_PIXELS) + BLOCK_SIZE_PIXELS / 2);
	w.data_well[w.data_wells].data_max = 192;
	w.data_well[w.data_wells].data = w.data_well[w.data_wells].data_max;
	w.data_well[w.data_wells].last_harvested = 0;
	w.data_well[w.data_wells].last_transferred = 0;

	w.data_well[w.data_wells].reserve_data [0] = map_init.data_well_reserve_data [w.data_wells] [0];//2000;//1000 + grand(1000);
	w.data_well[w.data_wells].reserve_data [1] = map_init.data_well_reserve_data [w.data_wells] [1];
	w.data_well[w.data_wells].reserve_squares = map_init.data_well_reserve_squares [w.data_wells];
	w.data_well[w.data_wells].spin_rate = map_init.data_well_spin_rate [w.data_wells];

	w.data_well[w.data_wells].static_build_exclusion = al_itofix(450); // this is the default but it may be changed later

// clear_background_square(block_x - 4, block_y - 4, block_x + 4, block_y + 4);
// clear_background_circle(block_x, block_y, 4, 100);
// clear_background_circle(block_x, block_y, 3, 100);

// circle_around_data_well(block_x, block_y, 3, 4, 1, 5);

// block_node_circle(block_x, block_y, 3, 4, 1, 5,
 block_node_circle(block_x, block_y, 2, 3, 1, 3,
																			BNC_CENTRE_EMPTY,
																			BNC_INNER_SCATTERED_GRADIENT,
																			BNC_MIDDLE_RAISED,
																			BNC_OUTER_GRADIENT);
//static void circle_around_data_well(int centre_block_x, int centre_block_y, int clear_size, int inner_edge_width, int middle_edge_width, int outer_edge_width)


 w.backblock[block_x][block_y].backblock_type = BACKBLOCK_DATA_WELL;
 w.backblock[block_x][block_y].backblock_value = w.data_wells;


#define WELL_EDGE_SPACING 2
// surround it with data_well_edge blocks that tell the display code to draw it even when it's off-screen
 w.backblock[block_x + WELL_EDGE_SPACING][block_y + WELL_EDGE_SPACING].backblock_type = BACKBLOCK_DATA_WELL_EDGE;
 w.backblock[block_x + WELL_EDGE_SPACING][block_y + WELL_EDGE_SPACING].backblock_value = w.data_wells;
 clear_hex_block_nodes(block_x + WELL_EDGE_SPACING, block_y + WELL_EDGE_SPACING);
 w.backblock[block_x - WELL_EDGE_SPACING][block_y + WELL_EDGE_SPACING].backblock_type = BACKBLOCK_DATA_WELL_EDGE;
 w.backblock[block_x - WELL_EDGE_SPACING][block_y + WELL_EDGE_SPACING].backblock_value = w.data_wells;
 clear_hex_block_nodes(block_x - WELL_EDGE_SPACING, block_y + WELL_EDGE_SPACING);
 w.backblock[block_x - WELL_EDGE_SPACING][block_y - WELL_EDGE_SPACING].backblock_type = BACKBLOCK_DATA_WELL_EDGE;
 w.backblock[block_x - WELL_EDGE_SPACING][block_y - WELL_EDGE_SPACING].backblock_value = w.data_wells;
 clear_hex_block_nodes(block_x - WELL_EDGE_SPACING, block_y - WELL_EDGE_SPACING);
 w.backblock[block_x + WELL_EDGE_SPACING][block_y - WELL_EDGE_SPACING].backblock_type = BACKBLOCK_DATA_WELL_EDGE;
 w.backblock[block_x + WELL_EDGE_SPACING][block_y - WELL_EDGE_SPACING].backblock_value = w.data_wells;
 clear_hex_block_nodes(block_x + WELL_EDGE_SPACING, block_y - WELL_EDGE_SPACING);
	w.data_well[w.data_wells].last_drawn = 0;

 w.data_wells ++;

}

static void block_node_circle(int centre_block_x,
																														int centre_block_y,
																														int centre_size,
																														int inner_edge_width,
																														int middle_edge_width,
																														int outer_edge_width,
																														int centre_type,
																														int inner_edge_type,
																														int middle_edge_type,
																														int outer_edge_type)
{

#ifdef SANITY_CHECK
 if (inner_edge_width == 0
		|| outer_edge_width == 0)
	{
		fpr("\n Error: g_world_back.c: circle_around_data_well(): inner_edge_width and outer_edge_width must not be 0.");
// ... because we need to divide by it later
		error_call();
	}

#endif

	int total_size = centre_size + inner_edge_width + middle_edge_width + outer_edge_width;
	al_fixed centre_size_fixed = al_itofix(centre_size * BLOCK_SIZE_PIXELS);
	al_fixed inner_size_fixed = centre_size_fixed + al_itofix(inner_edge_width * BLOCK_SIZE_PIXELS);
	al_fixed middle_size_fixed = inner_size_fixed + al_itofix(middle_edge_width * BLOCK_SIZE_PIXELS);
	al_fixed outer_size_fixed = middle_size_fixed + al_itofix(outer_edge_width * BLOCK_SIZE_PIXELS);

// fpr("\n dw cl %i i %i m %i o %i ts %i", al_fixtoi(centre_size_fixed), al_fixtoi(inner_size_fixed), al_fixtoi(middle_size_fixed), al_fixtoi(outer_size_fixed), total_size * BLOCK_SIZE_PIXELS);

 int x1 = centre_block_x - total_size;
 int y1 = centre_block_y - total_size;
 int x2 = centre_block_x + total_size + 1;
 int y2 = centre_block_y + total_size + 1;

	int i, j, k;
//	int size_pixels = total_size * BLOCK_SIZE_PIXELS;
//	al_fixed size_fixed = al_itofix(size_pixels);
	al_fixed dist;
	al_fixed centre_fixed_x = block_to_fixed(centre_block_x) + BLOCK_SIZE_FIXED / 2;
	al_fixed centre_fixed_y = block_to_fixed(centre_block_y) + BLOCK_SIZE_FIXED / 2;

//fpr("\n xy %i,%i to %i,%i total_size %i", x1, y1, x2, y2, total_size);

	for (i = x1; i < x2; i ++)
	{
		for (j = y1; j < y2; j ++)
		{
// bounds-check:
			if (i < 3
				|| i >= w.blocks.x - 4
				|| j < 3
				|| j >= w.blocks.y - 4)
				 continue;
   if (w.backblock[i][j].backblock_type == BACKBLOCK_DATA_WELL
				|| w.backblock[i][j].backblock_type == BACKBLOCK_DATA_WELL_EDGE)
					continue;
// first eliminate blocks that are either empty, or ignored because they're outside all circles:
			dist = distance(al_itofix(centre_block_y - j), al_itofix(centre_block_x - i)); // dist is in blocks (but fixed)
			if (dist > al_itofix(total_size + 1))
			 continue;
			if (centre_type == BNC_CENTRE_EMPTY
				&& dist < al_itofix(centre_size - 1))
			{
			 if (w.backblock[i][j].backblock_type == BACKBLOCK_BASIC_HEX
				 || w.backblock[i][j].backblock_type == BACKBLOCK_BASIC_HEX_NO_NODES)
				{
			  w.backblock[i][j].backblock_type = BACKBLOCK_BASIC_HEX_NO_NODES; //
     clear_hex_block_nodes(i,j);
				}
			 continue;
			}
/*
			if (dist < al_itofix(centre_size - 1))
			{
			 w.backblock[i][j].backblock_type = BACKBLOCK_BASIC_HEX_NO_NODES; //
    clear_hex_block_nodes(i,j);
			 continue;
			}*/

			if (w.backblock[i][j].backblock_type == BACKBLOCK_BASIC_HEX
				|| w.backblock[i][j].backblock_type == BACKBLOCK_BASIC_HEX_NO_NODES
				|| w.backblock[i][j].backblock_type == BACKBLOCK_EMPTY)
			{
				int nodes_left = 0; // this will be added to and subtracted from below

			 for (k = 0; k < BLOCK_NODES; k ++)
			 {
				 dist = distance(block_to_fixed(j) + al_itofix(w.backblock[i][j].node_y [k]) - centre_fixed_y, block_to_fixed(i) + al_itofix(w.backblock[i][j].node_x [k]) - centre_fixed_x);

				 if (w.backblock[i][j].node_exists [k])
						nodes_left ++;


     if (dist < centre_size_fixed)
					{
						switch(centre_type)
						{
							case BNC_CENTRE_EMPTY:
						  w.backblock[i][j].node_exists [k] = 0;
						  nodes_left--;
						  continue;
						 case BNC_CENTRE_LEAVE:
								continue;
						}
						continue;
					}

					if (dist < inner_size_fixed)
					{
						switch(inner_edge_type)
						{
						 case BNC_INNER_SCATTERED_GRADIENT:
							 {
						   int remove_chance = al_fixtoi(al_fixdiv((dist - centre_size_fixed), (inner_size_fixed - centre_size_fixed)) * 100);
						   if (mrand(80) > remove_chance)
						   {
						    w.backblock[i][j].node_exists [k] = 0;
						    nodes_left--;
						    continue;
						   }
				     if (!w.backblock[i][j].node_exists [k])
						    nodes_left ++;
					    w.backblock[i][j].node_exists [k] = 1;
 			     w.backblock[i][j].node_size [k] = 8 + remove_chance / 10 + mrand(4);
// 			  w.backblock[i][j].node_size [k] = 7 + remove_chance / 7;
			      w.backblock[i][j].node_depth [k] = 0;
			      int depth_chance = remove_chance + mrand(30);
 			     if (depth_chance < 50)
							   w.backblock[i][j].node_depth [k] = 3;
						     else
								   {
 			        if (depth_chance < 70)
							      w.backblock[i][j].node_depth [k] = 2;
  								    else
		  						    {
 			           if (depth_chance < 90)
					 		        w.backblock[i][j].node_depth [k] = 1;
						 		     }
								   }
							 } // end case BNC_INNER_SCATTERED_GRADIENT
						  continue;
						} // end switch(inner_edge_type)
					}

					if (dist < middle_size_fixed)
					{
// only BNC_MIDDLE_RAISED implemented for now
						w.backblock[i][j].node_size [k] = 18 + mrand(6);
//						if (mrand(3) == 0)
//						w.backblock[i][j].node_size [k] = 28;
				  w.backblock[i][j].node_depth [k] = 0;
	     if (!w.backblock[i][j].node_exists [k])
				    nodes_left ++;
		    w.backblock[i][j].node_exists [k] = 1;
						continue;
					}




					if (dist < outer_size_fixed)
					{
						switch(outer_edge_type)
						{
						 case BNC_OUTER_GRADIENT:
						 	{
					    int distance_proportion = al_fixtoi(al_fixdiv((dist - middle_size_fixed), (outer_size_fixed - middle_size_fixed)) * 70) + mrand(30);
						   int new_node_size = w.backblock[i][j].node_size [k];
						   int new_node_depth = w.backblock[i][j].node_depth [k];
						   new_node_size = 21 - distance_proportion / 6;
						   new_node_depth = distance_proportion / 26;
/*						   if (distance_proportion < 20)
						   {
										new_node_depth = 0;
										new_node_size = 3;
						   }
						    else
										{
						     if (distance_proportion < 20)
						     {
										  new_node_depth = 0;
										  new_node_size = 3;
						     }

										}
*/

					    if (w.backblock[i][j].node_size [k] < new_node_size)
							   w.backblock[i][j].node_size [k] = new_node_size;
							  if (w.backblock[i][j].node_depth [k] > new_node_depth)
							   w.backblock[i][j].node_depth [k] = new_node_depth;

  				   if (!w.backblock[i][j].node_exists [k])
		  				  nodes_left ++;
  					  w.backblock[i][j].node_exists [k] = 1;

						 	}
							 break;

						 case BNC_OUTER_RANDOM_GRADIENT:
							 {
						   int change_chance = al_fixtoi(al_fixdiv((dist - middle_size_fixed), (outer_size_fixed - middle_size_fixed)) * 100);
						   int new_node_size = w.backblock[i][j].node_size [k];
						   int new_node_depth = w.backblock[i][j].node_depth [k];
						if (mrand(100) > change_chance)
//						if (change_chance < 50)
						   {
						    new_node_size = 20;
				      new_node_depth = 0;

				      change_chance = 100 - change_chance;

 			      new_node_size = 8 + change_chance / 10 + mrand(4);
// 			  new_node_size = 7 + change_chance / 7;
			       new_node_depth = 0;
			       int depth_chance = change_chance;// + mrand(30);
 			      if (depth_chance < 30)
							    new_node_depth = 3;
						      else
								    {
 			         if (depth_chance < 50)
							       new_node_depth = 2;
  								     else
		  						     {
 			            if (depth_chance < 70)
					 		         new_node_depth = 1;
						 		      }
								    }

							    if (w.backblock[i][j].node_size [k] < new_node_size)
							     w.backblock[i][j].node_size [k] = new_node_size;
							    if (w.backblock[i][j].node_depth [k] > new_node_depth)
							     w.backblock[i][j].node_depth [k] = new_node_depth;

  				     if (!w.backblock[i][j].node_exists [k])
		  				    nodes_left ++;
  					    w.backblock[i][j].node_exists [k] = 1;
						   }
							 }
						  continue; // end case BNC_OUTER_RANDOM_GRADIENT


						} // end switch(outer_edge_type)
						continue;
					}

			} // end for k (node loop)
		 if (nodes_left <= 0)
				w.backblock[i][j].backblock_type = BACKBLOCK_BASIC_HEX_NO_NODES;
				 else
  				w.backblock[i][j].backblock_type = BACKBLOCK_BASIC_HEX;
		 } // end if BASIC_HEX

		} // end for j
	} // end for i

}







/*

static void circle_around_data_well(int centre_block_x, int centre_block_y, int clear_size, int inner_edge_width, int middle_edge_width, int outer_edge_width)
{

#ifdef SANITY_CHECK
 if (inner_edge_width == 0
		|| outer_edge_width == 0)
	{
		fpr("\n Error: g_world_back.c: circle_around_data_well(): inner_edge_width and outer_edge_width must not be 0.");
// ... because we need to divide by it later
		error_call();
	}

#endif

	int total_size = clear_size + inner_edge_width + middle_edge_width + outer_edge_width;
	al_fixed clear_size_fixed = al_itofix(clear_size * BLOCK_SIZE_PIXELS);
	al_fixed inner_size_fixed = clear_size_fixed + al_itofix(inner_edge_width * BLOCK_SIZE_PIXELS);
	al_fixed middle_size_fixed = inner_size_fixed + al_itofix(middle_edge_width * BLOCK_SIZE_PIXELS);
	al_fixed outer_size_fixed = middle_size_fixed + al_itofix(outer_edge_width * BLOCK_SIZE_PIXELS);

// fpr("\n dw cl %i i %i m %i o %i ts %i", al_fixtoi(clear_size_fixed), al_fixtoi(inner_size_fixed), al_fixtoi(middle_size_fixed), al_fixtoi(outer_size_fixed), total_size * BLOCK_SIZE_PIXELS);

 int x1 = centre_block_x - total_size;
 int y1 = centre_block_y - total_size;
 int x2 = centre_block_x + total_size + 1;
 int y2 = centre_block_y + total_size + 1;

	int i, j, k;
//	int size_pixels = total_size * BLOCK_SIZE_PIXELS;
//	al_fixed size_fixed = al_itofix(size_pixels);
	al_fixed dist;
	al_fixed centre_fixed_x = block_to_fixed(centre_block_x) + BLOCK_SIZE_FIXED / 2;
	al_fixed centre_fixed_y = block_to_fixed(centre_block_y) + BLOCK_SIZE_FIXED / 2;

//fpr("\n xy %i,%i to %i,%i total_size %i", x1, y1, x2, y2, total_size);

	for (i = x1; i < x2; i ++)
	{
		for (j = y1; j < y2; j ++)
		{
// bounds-check:
			if (i < 3
				|| i >= w.blocks.x - 4
				|| j < 3
				|| j >= w.blocks.y - 4)
				 continue;
   if (w.backblock[i][j].backblock_type == BACKBLOCK_DATA_WELL
				|| w.backblock[i][j].backblock_type == BACKBLOCK_DATA_WELL_EDGE)
					continue;
// first eliminate blocks that are either empty, or ignored because they're outside all circles:
			dist = distance(al_itofix(centre_block_y - j), al_itofix(centre_block_x - i)); // dist is in blocks (but fixed)
			if (dist > al_itofix(total_size + 1))
			 continue;
			if (dist < al_itofix(clear_size - 1))
			{
			 w.backblock[i][j].backblock_type = BACKBLOCK_EMPTY; //
    clear_hex_block_nodes(i,j);
			 continue;
			}
			if (dist < al_itofix(clear_size - 1))
			{
			 w.backblock[i][j].backblock_type = BACKBLOCK_BASIC_HEX_NO_NODES; //
    clear_hex_block_nodes(i,j);
			 continue;
			}

			if (w.backblock[i][j].backblock_type == BACKBLOCK_BASIC_HEX)
			{
				int nodes_cleared = 0;

			 for (k = 0; k < BLOCK_NODES; k ++)
			 {
				 dist = distance(block_to_fixed(j) + al_itofix(w.backblock[i][j].node_y [k]) - centre_fixed_y, block_to_fixed(i) + al_itofix(w.backblock[i][j].node_x [k]) - centre_fixed_x);
// can this (and the distance call above) be made distance_oct to avoid the slow fixed sqrt function?

     if (dist < clear_size_fixed)
					{
						w.backblock[i][j].node_exists [k] = 0;
						nodes_cleared++;
						continue;
					}

					if (dist < inner_size_fixed)
					{
						int remove_chance = al_fixtoi(al_fixdiv((dist - clear_size_fixed), (inner_size_fixed - clear_size_fixed)) * 100);
						if (mrand(80) > remove_chance)
						{
						 w.backblock[i][j].node_exists [k] = 0;
						 nodes_cleared++;
						}
 			  w.backblock[i][j].node_size [k] = 8 + remove_chance / 10 + mrand(4);
// 			  w.backblock[i][j].node_size [k] = 7 + remove_chance / 7;
			   w.backblock[i][j].node_depth [k] = 0;
			   int depth_chance = remove_chance + mrand(30);
 			  if (depth_chance < 50)
							w.backblock[i][j].node_depth [k] = 3;
						  else
								{
 			     if (depth_chance < 70)
							   w.backblock[i][j].node_depth [k] = 2;
  								 else
		  						 {
 			        if (depth_chance < 90)
					 		     w.backblock[i][j].node_depth [k] = 1;
						 		  }
								}

						continue;
					}

					if (dist < middle_size_fixed)
					{
						w.backblock[i][j].node_size [k] = 18 + mrand(6);
//						if (mrand(3) == 0)
//						w.backblock[i][j].node_size [k] = 28;
				  w.backblock[i][j].node_depth [k] = 0;
						continue;
					}




					if (dist < outer_size_fixed)
					{

						int change_chance = al_fixtoi(al_fixdiv((dist - middle_size_fixed), (outer_size_fixed - middle_size_fixed)) * 100);
						int new_node_size = w.backblock[i][j].node_size [k];
						int new_node_depth = w.backblock[i][j].node_depth [k];
//						if (mrand(100) > change_chance)
//						if (change_chance < 50)
						{
						 new_node_size = 20;
				   new_node_depth = 0;

				  change_chance = 100 - change_chance;

 			  new_node_size = 8 + change_chance / 10 + mrand(4);
// 			  new_node_size = 7 + change_chance / 7;
			   new_node_depth = 0;
			   int depth_chance = change_chance;// + mrand(30);
 			  if (depth_chance < 30)
							new_node_depth = 3;
						  else
								{
 			     if (depth_chance < 50)
							   new_node_depth = 2;
  								 else
		  						 {
 			        if (depth_chance < 70)
					 		     new_node_depth = 1;
						 		  }
								}

							if (w.backblock[i][j].node_size [k] < new_node_size)
							 w.backblock[i][j].node_size [k] = new_node_size;
							if (w.backblock[i][j].node_depth [k] > new_node_depth)
							 w.backblock[i][j].node_depth [k] = new_node_depth;


						}
						continue;
					}

			} // end for k (node loop)
		 if (nodes_cleared == BLOCK_NODES)
				w.backblock[i][j].backblock_type = BACKBLOCK_BASIC_HEX_NO_NODES;
		 } // end if BASIC_HEX

		} // end for j
	} // end for i

}
*/

static void base_background_uniform(void)
{

 int i, j;
 int k, l, nd;
 struct backblock_struct* backbl;
// int space_size = (NODE_SPACING / 6) * 5;

 nd = 0;

  for (i = 0; i < map_init.map_size_blocks; i ++)
  {
   for (j = 0; j < map_init.map_size_blocks; j ++)
   {

    backbl = &w.backblock [i] [j];

    for (k = 0; k < 3; k ++)
    {
     for (l = 0; l < 3; l ++)
     {

      nd = l + (k * 3);

      backbl->node_exists [nd] = 1;
      backbl->node_size [nd] = map_init.base_background_size;
      backbl->node_depth [nd] = map_init.base_background_depth;

      if (mrand(100) < map_init.background_size_random_freq)
						{
							backbl->node_size [nd] += mrand(map_init.background_size_random_add + 1);
							backbl->node_size [nd] -= mrand(map_init.background_size_random_sub + 1);
						}

      if (mrand(100) < map_init.background_depth_random_freq)
						{
							backbl->node_depth [nd] += mrand(map_init.background_depth_random_add + 1);
							backbl->node_depth [nd] -= mrand(map_init.background_depth_random_sub + 1);
						}

						backbl->node_size [nd] -= backbl->node_depth [nd];

      backbl->node_x [nd] = (k * NODE_SPACING) + NODE_SPACING / 2;// + grand(18) - grand(18);
      backbl->node_y [nd] = (l * NODE_SPACING) + NODE_SPACING / 2;// + grand(18) - grand(18);
      backbl->node_x_base [nd] = backbl->node_x [nd];
      backbl->node_y_base [nd] = backbl->node_y [nd];

      if ((j + l) % 2 == 0)
       backbl->node_x [nd] += NODE_SPACING / 2;

      backbl->node_team_col [nd] = 0;
      backbl->node_new_colour [nd] = 0;
      backbl->node_col_saturation [nd] = 0;
      backbl->node_new_saturation [nd] = 0;

      backbl->node_disrupt_timestamp [nd] = 0;
      backbl->node_pending_explosion_timestamp [nd] = 0;
      backbl->node_colour_change_timestamp [nd] = 0;

     }
    }
   }
  }


}

/*
static void init_hex_block_nodes(int x, int y)
{

 int k, l, nd;
 struct backblock_struct* backbl;
// int spacing = (BLOCK_SIZE_PIXELS + 3) / 3;
 int space_size = (NODE_SPACING / 6) * 5;
// int space_min = spacing / 12;

    backbl = &w.backblock [x] [y];
    nd = 0;

    for (k = 0; k < 3; k ++)
    {
     for (l = 0; l < 3; l ++)
     {


      nd = l + (k * 3);

      backbl->node_exists [nd] = 1;

      backbl->node_size [nd] = ((space_size) / 2) - 2 + 2;
      if (mrand(4) == 0)
       backbl->node_size [nd] += 2;
      if (mrand(7) == 0)
       backbl->node_size [nd] += 2;
      if (mrand(7) == 0)
       backbl->node_size [nd] -= 2;
      if (mrand(7) == 0)
       backbl->node_size [nd] -= 4;
      backbl->node_x [nd] = (k * NODE_SPACING) + NODE_SPACING / 2;// + grand(18) - grand(18);
      backbl->node_y [nd] = (l * NODE_SPACING) + NODE_SPACING / 2;// + grand(18) - grand(18);

      if ((y + l) % 2 == 0)
       backbl->node_x [nd] += NODE_SPACING / 2;

//      bl->node_size [nd] = fixed_size;//(bl->node_size [nd]);// * 2);// / 3;
      backbl->node_size [nd] = 20;//grand(7);

//      if (backblock_level == BACKBLOCK_LEVEL_LOWER)
//							backbl->node_size [nd] *= 0.8;

						backbl->node_depth [nd] = mrand(BACKBLOCK_LAYERS);//1.01 + (mrand(10) * 0.006);
						backbl->node_size [nd] -= backbl->node_depth [nd] * 2;
						backbl->node_size [nd] = 12;
						backbl->node_depth [nd] = 3;//mrand(BACKBLOCK_LAYERS);//1.01 + (mrand(10) * 0.006);

						if (mrand(6) == 0)
						{
						backbl->node_size [nd] = 14;
						backbl->node_depth [nd] = 2;//mrand(BACKBLOCK_LAYERS);//1.01 + (mrand(10) * 0.006);

						}

      backbl->node_x_base [nd] = backbl->node_x [nd];
      backbl->node_y_base [nd] = backbl->node_y [nd];

      backbl->node_team_col [nd] = 0;
      backbl->node_new_colour [nd] = 0;
      backbl->node_col_saturation [nd] = 0;
      backbl->node_new_saturation [nd] = 0;

      backbl->node_disrupt_timestamp [nd] = 0;
      backbl->node_pending_explosion_timestamp [nd] = 0;
      backbl->node_colour_change_timestamp [nd] = 0;

     }
    }



}

*/


static void clear_hex_block_nodes(int x, int y)
{

 int nd;
 struct backblock_struct* backbl;
    backbl = &w.backblock [x] [y];
    nd = 0;

    for (nd = 0; nd < BLOCK_NODES; nd ++)
    {
    	backbl->node_exists [nd] = 0;
    	backbl->node_depth [nd] = BACKBLOCK_LAYERS-1;
    }

}


static void block_node_line(int start_x, int start_y, int end_x, int end_y, int line_main_size, int line_edge_size)
{

 int start_x_pixel = start_x * BLOCK_SIZE_PIXELS;
 int start_y_pixel = start_y * BLOCK_SIZE_PIXELS;
 int end_x_pixel = end_x * BLOCK_SIZE_PIXELS;
 int end_y_pixel = end_y * BLOCK_SIZE_PIXELS;

 int diff_x = end_x_pixel - start_x_pixel;
 int diff_y = end_y_pixel - start_y_pixel;

 if (diff_x == 0
	 || diff_y == 0)
			return; // maybe should draw a single thing here?

 int step_x, step_y;
 int steps;

 if (diff_x > diff_y)
	{
		step_x = 100;

		step_y = (diff_y * 100) / diff_x;

		steps = (diff_x / 100) + 1;

	}
	 else
		{
		 step_y = 100;

		 step_x = (diff_x * 100) / diff_y;

 		steps = (diff_y / 100) + 1;

		}

 int i;

 int line_x = start_x_pixel;
 int line_y = start_y_pixel;



 for (i = 0; i < steps + 1; i ++)
	{
		raise_background(line_x, line_y, line_main_size, line_edge_size);

		line_x += step_x;
		line_y += step_y;

	}

}


static void raise_background(int centre_x, int centre_y, int main_size, int edge_size)
{

//fpr("\n rb %i,%i,%i", centre_x, centre_y,	rsize);
	int centre_block_x = centre_x / BLOCK_SIZE_PIXELS;
	int centre_block_y = centre_y / BLOCK_SIZE_PIXELS;

	al_fixed main_size_fixed = al_itofix(main_size);
	al_fixed edge_size_fixed = al_itofix(edge_size);
	al_fixed total_size_fixed = main_size_fixed + edge_size_fixed;
	int raise_size_block = ((main_size + edge_size) / BLOCK_SIZE_PIXELS) + 1;

	int x1 = centre_block_x - raise_size_block;
 int y1 = centre_block_y - raise_size_block;
 int x2 = centre_block_x + raise_size_block + 1;
 int y2 = centre_block_y + raise_size_block + 1;

	int i, j, k;
//	int size_pixels = main_size * BLOCK_SIZE_PIXELS;
//	al_fixed size_fixed = al_itofix(size_pixels);
	al_fixed dist;
	al_fixed centre_fixed_x = al_itofix(centre_x);
	al_fixed centre_fixed_y = al_itofix(centre_y);

	for (i = x1; i < x2; i ++)
	{
		for (j = y1; j < y2; j ++)
		{
			if (i < 3
				|| i >= w.blocks.x - 4
				|| j < 3
				|| j >= w.blocks.y - 4)
				continue;
			if (w.backblock[i][j].backblock_type == BACKBLOCK_BASIC_HEX)
			{

			 for (k = 0; k < BLOCK_NODES; k ++)
			 {
			 	if (!w.backblock[i][j].node_exists [k])
						continue;
				 dist = distance(block_to_fixed(j) + al_itofix(w.backblock[i][j].node_y [k]) - centre_fixed_y,
																					block_to_fixed(i) + al_itofix(w.backblock[i][j].node_x [k]) - centre_fixed_x);
//					int dist_proportion = al_fixdiv(dist, raise_size_fixed);
					int new_node_size = 0;// = al_fixtoi(dist_proportion * 18);
					int new_node_depth = 3;
					if (dist < main_size_fixed)
					{
						new_node_size = 18;
						new_node_depth = 0;
					new_node_size += mrand(5);
					}
						 else
							{
								if (edge_size > 0
									&&	dist < total_size_fixed)
								{
//									dist -= main_size_fixed;
					    int dist_proportion = 100 - al_fixtoi(al_fixdiv(dist - main_size_fixed, edge_size_fixed) * 100);
					    new_node_size = 5 + (dist_proportion / 8);
					new_node_size += mrand(5);
									if (new_node_size < 12)
										new_node_depth = 2;
									  else
											{
									   if (new_node_size < 15)
										   new_node_depth = 1;
										    else
															new_node_depth = 0;
											}
									}
							}

/*					if (dist < raise_size_fixed)
					{
						new_node_size = 18 + mrand(5);
						new_node_depth = 0;
					}*/
					if (new_node_size > w.backblock[i][j].node_size [k])
					 w.backblock[i][j].node_size [k] = new_node_size;
					if (new_node_depth < w.backblock[i][j].node_depth [k])
					 w.backblock[i][j].node_depth [k] = new_node_depth;
			 }
			}

		}
	}

}









void block_node_system(int centre_block_x, int centre_block_y, int system_size)
{

	int i, j, k;
	int centre_x_pixels = centre_block_x * BLOCK_SIZE_PIXELS + BLOCK_SIZE_PIXELS / 2;
	int centre_y_pixels = centre_block_y * BLOCK_SIZE_PIXELS + BLOCK_SIZE_PIXELS / 2;

	int left_block = centre_block_x - system_size;
	if (left_block < 2)
		left_block = 2;
	int right_block = centre_block_x + system_size + 1;
	if (right_block > w.blocks.x - 3)
		right_block = w.blocks.x - 3;
	int top_block = centre_block_y - system_size;
	if (top_block < 2)
		top_block = 2;
	int bottom_block = centre_block_y + system_size + 1;
	if (bottom_block > w.blocks.y - 3)
		bottom_block = w.blocks.y - 3;

	int system_size_pixels = system_size * BLOCK_SIZE_PIXELS;

	struct backblock_struct* backbl;
	int node_dist_from_centre;
	int node_world_x, node_world_y;
//	int node_effect_time;
 int nodes_added;

	for (i = left_block; i < right_block; i++)
	{
		for (j = top_block; j < bottom_block; j ++)
		{
			if (w.backblock[i][j].backblock_type != BACKBLOCK_BASIC_HEX
				&& w.backblock[i][j].backblock_type != BACKBLOCK_BASIC_HEX_NO_NODES) // new nodes can be added
				continue;
			backbl = &w.backblock[i][j];
			nodes_added = 0;
			for (k = 0; k < BLOCK_NODES; k ++)
			{

				node_world_x = (i * BLOCK_SIZE_PIXELS) + backbl->node_x [k];
				node_world_y = (j * BLOCK_SIZE_PIXELS) + backbl->node_y [k];
				node_dist_from_centre = distance_oct_int(node_world_y - centre_x_pixels, node_world_x - centre_y_pixels);
//fpr("[nw(%i,%i)d(%i)", node_world_x, node_world_y, node_dist_from_centre);
				int approaching_edge = system_size_pixels - node_dist_from_centre;
    approaching_edge += mrand(80);

				if (approaching_edge < 100
				 && mrand(100) > approaching_edge)
						continue;

				if (mrand(3) != 0)
					continue;

				if (node_dist_from_centre < 140 + mrand(80))
					continue;

				if (node_dist_from_centre < system_size_pixels)
				{
     backbl->node_exists [k] = 1;
     int new_size = 20;
     int new_depth = 0;//2 + mrand(3);

     if (approaching_edge < 180)
					{
						new_size = 10 + (approaching_edge / 18);
						new_depth = 3 - (approaching_edge / 50) - mrand(3);
						if (new_depth < 0)
							new_depth = 0;
					}

     if (backbl->node_size [k] < new_size)
      backbl->node_size [k] = new_size;
//     if (backbl->node_depth [k] > new_depth)
      backbl->node_depth [k] = new_depth;

      backbl->node_colour_change_timestamp [k] = 0;
      backbl->node_new_saturation [k] = 3 - backbl->node_depth [k];//1 + mrand(3);// + mrand(3);
      backbl->node_new_colour [k] = 1;// + mrand(3);

     nodes_added ++;
				}

			}

			if (w.backblock[i][j].backblock_type == BACKBLOCK_BASIC_HEX_NO_NODES
				&& nodes_added > 0)
					w.backblock[i][j].backblock_type = BACKBLOCK_BASIC_HEX;
		}
	}


// finally add the large central node:

  i = centre_block_x;
  j = centre_block_y;
  k = 5;

			if (w.backblock[i][j].backblock_type == BACKBLOCK_BASIC_HEX
				|| w.backblock[i][j].backblock_type == BACKBLOCK_BASIC_HEX_NO_NODES)
			{
  			backbl = &w.backblock[i][j];
     backbl->node_exists [k] = 1;
     backbl->node_size [k] = 80 + mrand(32);
     backbl->node_depth [k] = 2;
     backbl->node_team_col [k] = 1;

      backbl->node_colour_change_timestamp [k] = 0;
      backbl->node_new_saturation [k] = 0;// + mrand(3);
      backbl->node_new_colour [k] = 0;// + mrand(3);

//      backbl->node_col_saturation [k] = 0;// + mrand(3);
//      backbl->node_team_col [k] = 0;// + mrand(3);

					w.backblock[i][j].backblock_type = BACKBLOCK_BASIC_HEX;

			}



}




void clear_background_circle(int centre_block_x, int centre_block_y, int clear_size, int edge_thickness)
{
 int x1 = centre_block_x - clear_size;
 int y1 = centre_block_y - clear_size;
 int x2 = centre_block_x + clear_size + 1;
 int y2 = centre_block_y + clear_size + 1;

	int i, j, k;
	int size_pixels = clear_size * BLOCK_SIZE_PIXELS;
	al_fixed size_fixed = al_itofix(size_pixels);
	al_fixed dist;
	al_fixed centre_fixed_x = block_to_fixed(centre_block_x) + BLOCK_SIZE_FIXED / 2;
	al_fixed centre_fixed_y = block_to_fixed(centre_block_y) + BLOCK_SIZE_FIXED / 2;

	for (i = x1; i < x2; i ++)
	{
		for (j = y1; j < y2; j ++)
		{
			if (i < 3
				|| i >= w.blocks.x - 4
				|| j < 3
				|| j >= w.blocks.y - 4)
				continue;
			dist = distance(al_itofix(centre_block_y - j), al_itofix(centre_block_x - i));
			if (dist < al_itofix(clear_size - edge_thickness - 1))
			{
			 w.backblock[i][j].backblock_type = BACKBLOCK_BASIC_HEX_NO_NODES;
			 continue;
			}
			if (dist > al_itofix(clear_size + 1))
			{
//			 w.block[i][j].backblock_type = BACKBLOCK_EMPTY; - do nothing
			 continue;
			}
			if (w.backblock[i][j].backblock_type == BACKBLOCK_BASIC_HEX)
			{
				int nodes_cleared = 0;

			 for (k = 0; k < BLOCK_NODES; k ++)
			 {
				 dist = distance(block_to_fixed(j) + al_itofix(w.backblock[i][j].node_y [k]) - centre_fixed_y, block_to_fixed(i) + al_itofix(w.backblock[i][j].node_x [k]) - centre_fixed_x);
				 if (dist < size_fixed - al_itofix(edge_thickness))
					{
						w.backblock[i][j].node_exists [k] = 0;
						nodes_cleared++;
					}
					 else
				   if (dist < size_fixed)
					  {
					  	int size_percent = (al_fixtoi(size_fixed - dist) * 100) / edge_thickness;

//					  	if (grand(100) > size_percent)
//								if (size_percent < 99)
//									w.block[i][j].node_exists [k] = grand(2);

						  w.backblock[i][j].node_size [k] *= 100 - size_percent;//(dist - size_fixed);//edge_thickness + 60 + al_fixtoi(dist - size_fixed);
						  w.backblock[i][j].node_size [k] /= 100;//edge_thickness;



//						  w.block[i][j].node_size [k] *= edge_thickness + 60 + al_fixtoi(dist - size_fixed);
//						  w.block[i][j].node_size [k] /= edge_thickness + 160;
					  }
			 }
			 if (nodes_cleared == BLOCK_NODES)
					w.backblock[i][j].backblock_type = BACKBLOCK_BASIC_HEX_NO_NODES;
			}

		}
	}

}


