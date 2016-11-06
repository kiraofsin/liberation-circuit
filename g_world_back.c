/*

World background generation etc code

*/
#include <allegro5/allegro.h>

#include <stdio.h>
#include <math.h>


#include "m_config.h"
#include "m_globvars.h"

#include "g_header.h"

#include "g_misc.h"

#include "m_maths.h"
#include "i_disp_in.h"
#include "i_background.h"
#include "s_menu.h"
#include "g_world_back.h"
#include "g_world_map_2.h"

static void init_vision_block(int x, int y);
//static void block_node_pattern(void);

extern struct world_init_struct w_init; // this is the world_init generated by world setup menus - declared in s_menu.c


// This function creates a world background in the block structures
// it assumes that the block structures have otherwise been set up
void init_world_background(void)
{

	int i, j;

	w.backblock_parallax [0] = 1;
//	w.backblock_parallax [1] = 0.985;
//	w.backblock_parallax [2] = 0.97;
//	w.backblock_parallax [3] = 0.955;
	w.backblock_parallax [1] = 0.99;
	w.backblock_parallax [2] = 0.98;
	w.backblock_parallax [3] = 0.97;

// init the whole backblock array
  for (i = 0; i < w.blocks.x; i ++)
  {
   for (j = 0; j < w.blocks.y; j ++)
   {
    w.backblock [i] [j].backblock_type = BACKBLOCK_BASIC_HEX;
//    init_hex_block_nodes(i, j);
   }
  }

  for (i = 0; i < w.blocks.x; i ++)
  {
   for (j = 0; j < w.blocks.y; j ++)
   {
    init_vision_block(i, j);
   }
  }


// now the edges:
// for (k = 0; k < BACKBLOCK_LEVELS; k ++)
	{
  for (i = 0; i < w.blocks.x; i ++)
  {

   w.backblock [i] [0].backblock_type = BACKBLOCK_OUTER;
   w.backblock [i] [1].backblock_type = BACKBLOCK_EDGE_UP;
   w.backblock [i] [w.blocks.y - 1].backblock_type = BACKBLOCK_OUTER;
   w.backblock [i] [w.blocks.y - 2].backblock_type = BACKBLOCK_EDGE_DOWN;
  }
  for (i = 0; i < w.blocks.y; i ++)
  {
   w.backblock [0] [i].backblock_type = BACKBLOCK_OUTER;
   if (i>0&&i<(w.blocks.y-1))
 			w.backblock [1] [i].backblock_type = BACKBLOCK_EDGE_LEFT;
   w.backblock [w.blocks.x - 1] [i].backblock_type = BACKBLOCK_OUTER;
   if (i>0&&i<(w.blocks.y-1))
    w.backblock [w.blocks.x - 2] [i].backblock_type = BACKBLOCK_EDGE_RIGHT;
  }

  w.backblock [1] [1].backblock_type = BACKBLOCK_EDGE_UP_LEFT;
  w.backblock [1] [w.blocks.y - 2].backblock_type = BACKBLOCK_EDGE_DOWN_LEFT;
  w.backblock [w.blocks.x - 2] [1].backblock_type = BACKBLOCK_EDGE_UP_RIGHT;
  w.backblock [w.blocks.x - 2] [w.blocks.y - 2].backblock_type = BACKBLOCK_EDGE_DOWN_RIGHT;
	} // end for k

/*
// now let's clear a space:
 for (i = 8; i < 20; i++)
	{
		for (j = 8; j < 16; j ++)
		{
   w.block [i] [j].backblock_type = BACKBLOCK_EMPTY;
// shouldn't need to worry about hex nodes as they just won't be displayed
		}
	}*/

// block_node_pattern();


}




/*
static void block_node_pattern(void)
{

	int i;

 int blotches = 5;//20 + mrand(100);

 for (i = 0; i < blotches; i ++)
	{
		int centre_x = 10 + mrand(w.blocks.x - 20);
		int centre_y = 10 + mrand(w.blocks.y - 20);
		clear_background_circle(centre_x, centre_y, 4 + mrand(5), 300 + mrand(300));


	}

}
*/





/*
void clear_background_square(int x1, int y1, int x2, int y2)
{

	if (x1 < 3)
		x1 = 3;
 if (y1 < 3)
		y1 = 3;
	if (x2 > w.blocks.x - 4)
		x2 = w.blocks.x - 4;
	if (y2 > w.blocks.y - 4)
		y2 = w.blocks.y - 4;

	int i, j;

	for (i = x1; i < x2 + 1; i ++)
	{
		for (j = y1; j < y2 + 1; j ++)
		{
			w.backblock[i][j].backblock_type = BACKBLOCK_EMPTY;
		}
	}

}
*/



/*

void clear_background_circle(int centre_block_x, int centre_block_y, int clear_size)
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

	al_fixed fixed_clear_size = al_itofix(clear_size + 1);

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
			if (dist < al_itofix(clear_size - 1))
			{
			 w.block[i][j].backblock_type = BACKBLOCK_EMPTY;
			 continue;
			}
			if (dist > al_itofix(clear_size + 1))
			{
//			 w.block[i][j].backblock_type = BACKBLOCK_EMPTY; - do nothing
			 continue;
			}
			if (w.block[i][j].backblock_type == BACKBLOCK_BASIC_HEX)
			{
				int nodes_cleared = 0;

			 for (k = 0; k < BLOCK_NODES; k ++)
			 {
				 dist = distance(block_to_fixed(j) + al_itofix(w.block[i][j].node_y [k]) - centre_fixed_y, block_to_fixed(i) + al_itofix(w.block[i][j].node_x [k]) - centre_fixed_x);
				 if (dist < size_fixed)
					{
						w.block[i][j].node_exists [k] = 0;
						nodes_cleared++;
					}
					 else
				   if (dist < size_fixed + al_itofix(100))
					  {
						  w.block[i][j].node_size [k] *= 160 + al_fixtoi(dist - size_fixed);
						  w.block[i][j].node_size [k] /= 260;
					  }
			 }
			 if (nodes_cleared == BLOCK_NODES)
					w.block[i][j].backblock_type = BACKBLOCK_EMPTY;
			}

		}
	}

}

*/

void explosion_affects_block_nodes(al_fixed explosion_x, al_fixed explosion_y, int explosion_size, int player_index)
{

	int i, j, k;
	int centre_block_x = fixed_to_block(explosion_x);
	int centre_block_y = fixed_to_block(explosion_y);
	int explosion_x_pixels = al_fixtoi(explosion_x);
	int explosion_y_pixels = al_fixtoi(explosion_y);

// test values:
	int explosion_size_pixels = explosion_size;
	int explosion_size_blocks = (explosion_size / BLOCK_SIZE_PIXELS) + 1; // this is a radius

	int left_block = centre_block_x - explosion_size_blocks;
	if (left_block < 2)
		left_block = 2;
	int right_block = centre_block_x + explosion_size_blocks + 1;
	if (right_block > w.blocks.x - 3)
		right_block = w.blocks.x - 3;
	int top_block = centre_block_y - explosion_size_blocks;
	if (top_block < 2)
		top_block = 2;
	int bottom_block = centre_block_y + explosion_size_blocks + 1;
	if (bottom_block > w.blocks.y - 3)
		bottom_block = w.blocks.y - 3;

	struct backblock_struct* backbl;
	int node_dist_from_centre;
	int node_world_x, node_world_y;
	int node_explosion_time;

	for (i = left_block; i < right_block; i++)
	{
		for (j = top_block; j < bottom_block; j ++)
		{
			if (w.backblock[i][j].backblock_type != BACKBLOCK_BASIC_HEX)
				continue;
			backbl = &w.backblock[i][j];
			for (k = 0; k < BLOCK_NODES; k ++)
			{
				if (w.backblock[i][j].node_exists [k] == 0)
					continue;
				node_world_x = (i * BLOCK_SIZE_PIXELS) + backbl->node_x [k];
				node_world_y = (j * BLOCK_SIZE_PIXELS) + backbl->node_y [k];
				node_dist_from_centre = distance_oct_int(node_world_y - explosion_y_pixels, node_world_x - explosion_x_pixels);
				int approaching_edge = explosion_size_pixels - node_dist_from_centre;
				if (approaching_edge < 80
				 && grand(80) > approaching_edge)
						continue;
				if (node_dist_from_centre < explosion_size_pixels)
				{
					node_explosion_time = 34 - (node_dist_from_centre / 10);// + grand(5);
					if (node_explosion_time > 32)
						node_explosion_time = 32;
     backbl->node_pending_explosion_timestamp [k] = w.world_time + 32 + (32 - node_explosion_time);
//     blk->node_pending_explosion_player [k] = player_index;
     backbl->node_pending_explosion_strength [k] = 1;//explosion_strength_at_node;
     backbl->node_colour_change_timestamp [k] = backbl->node_pending_explosion_timestamp [k] - 32;
     backbl->node_team_col [k] = backbl->node_new_colour [k]; // shouldn't do this if replacing another pending explosion!
     backbl->node_new_colour [k] = player_index;
     backbl->node_col_saturation [k] = backbl->node_new_saturation [k]; // shouldn't do this if replacing another pending explosion!
     if (backbl->node_new_saturation [k] < BACK_COL_SATURATIONS - 1)
      backbl->node_new_saturation [k] ++;


				}
			}
		}
	}




}


void static_build_affects_block_nodes(al_fixed build_x, al_fixed build_y, int effect_size, int player_index)
{

	int i, j, k;
	int centre_block_x = fixed_to_block(build_x);
	int centre_block_y = fixed_to_block(build_y);
	int effect_x_pixels = al_fixtoi(build_x);
	int effect_y_pixels = al_fixtoi(build_y);

	int effect_size_pixels = effect_size;// * 5;
	int effect_size_blocks = (effect_size / BLOCK_SIZE_PIXELS) + 1; // this is a radius

	int left_block = centre_block_x - effect_size_blocks;
	if (left_block < 2)
		left_block = 2;
	int right_block = centre_block_x + effect_size_blocks + 1;
	if (right_block > w.blocks.x - 3)
		right_block = w.blocks.x - 3;
	int top_block = centre_block_y - effect_size_blocks;
	if (top_block < 2)
		top_block = 2;
	int bottom_block = centre_block_y + effect_size_blocks + 1;
	if (bottom_block > w.blocks.y - 3)
		bottom_block = w.blocks.y - 3;

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
//			fpr("\n %i,%i type %i ", i, j, w.backblock[i][j].backblock_type);
			for (k = 0; k < BLOCK_NODES; k ++)
			{
//				if (w.backblock[i][j].node_exists [k] == 0)
//					continue;
//    backbl->node_x [k] = ((k / 3) * NODE_SPACING) + NODE_SPACING / 2;// + grand(18) - grand(18);
//    backbl->node_y [k] = ((k % 3) * NODE_SPACING) + NODE_SPACING / 2;// + grand(18) - grand(18);

				node_world_x = (i * BLOCK_SIZE_PIXELS) + backbl->node_x [k];
				node_world_y = (j * BLOCK_SIZE_PIXELS) + backbl->node_y [k];
				node_dist_from_centre = distance_oct_int(node_world_y - effect_y_pixels, node_world_x - effect_x_pixels);
//fpr("[nw(%i,%i)d(%i)", node_world_x, node_world_y, node_dist_from_centre);
				int approaching_edge = effect_size_pixels - node_dist_from_centre;
    approaching_edge += grand(80);
				if (approaching_edge < 100
				 && grand(100) > approaching_edge)
						continue;
				if (node_dist_from_centre < effect_size_pixels)
				{
//					fpr("A");
//					node_explosion_time = 34 - (node_dist_from_centre / 10);// + grand(5);
//					if (node_explosion_time > 32)
//						node_explosion_time = 32;
     backbl->node_exists [k] = 1;
     int new_size = 20;
     int new_depth = 0;
     if (approaching_edge < 120)
					{
						new_size = 10 + (approaching_edge / 12);
						new_depth = 3 - (approaching_edge / 40);
						if (new_depth < 0)
							new_depth = 0;
					}
     if (backbl->node_size [k] < new_size)
      backbl->node_size [k] = new_size;
     if (backbl->node_depth [k] > new_depth)
      backbl->node_depth [k] = new_depth;
     backbl->node_pending_explosion_timestamp [k] = w.world_time + 32;// + (32 - node_explosion_time);
//     blk->node_pending_explosion_player [k] = player_index;
     backbl->node_pending_explosion_strength [k] = 1;//explosion_strength_at_node;
     backbl->node_colour_change_timestamp [k] = backbl->node_pending_explosion_timestamp [k] - 32;
     backbl->node_team_col [k] = backbl->node_new_colour [k]; // shouldn't do this if replacing another pending explosion!
     backbl->node_new_colour [k] = player_index;
     backbl->node_col_saturation [k] = backbl->node_new_saturation [k]; // shouldn't do this if replacing another pending explosion!
     if (backbl->node_new_saturation [k] < BACK_COL_SATURATIONS - 1)
      backbl->node_new_saturation [k] ++;
     nodes_added ++;

//      backbl->node_x_base [k] = backbl->node_x [k];
//      backbl->node_y_base [k] = backbl->node_y [k];
				}
//					fpr("]");

			}
//					fpr("*na(%i)", nodes_added);

			if (w.backblock[i][j].backblock_type == BACKBLOCK_BASIC_HEX_NO_NODES
				&& nodes_added > 0)
					w.backblock[i][j].backblock_type = BACKBLOCK_BASIC_HEX;
		}
	}




}



void pulse_block_node(al_fixed pulse_x, al_fixed pulse_y)
{

//	int i, j, k;
	int block_x = fixed_to_block(pulse_x);
	int block_y = fixed_to_block(pulse_y);

	struct backblock_struct* backbl = &w.backblock[block_x][block_y];

	if (backbl->backblock_type != BACKBLOCK_BASIC_HEX)
		return;

	int node_index = ((al_fixtoi(pulse_y) & (BLOCK_SIZE_PIXELS-1)) / NODE_SPACING) + ((al_fixtoi(pulse_x) & (BLOCK_SIZE_PIXELS-1)) / NODE_SPACING) * 3;

#ifdef SANITY_CHECK
 if (node_index < 0
		|| node_index > 8)
	{
		fpr("\nError in g_world.c: pulse_block_node(): node_index out of bounds (%i)", node_index);
		error_call();
	}
#endif

 backbl->node_pending_explosion_timestamp [node_index] = w.world_time + 32;

// fpr("\n pulse at %i,%i block %i,%i node %i", al_fixtoi(pulse_x), al_fixtoi(pulse_y), block_x, block_y, node_index);

}



static void init_vision_block(int x, int y)
{

 w.vision_block[x][y].clear_time = 0;
 w.vision_block[x][y].proximity = 0;
 w.vision_block[x][y].proximity_time = 0;

}



unsigned int mrand_seed;

void seed_mrand(unsigned int new_mrand_seed)
{
	mrand_seed = new_mrand_seed;
}

unsigned int mrand(unsigned int rand_max)
{

 mrand_seed = mrand_seed * 1103515245 + 12345;
 return (unsigned int)(mrand_seed / 65536) % rand_max;

}



// should be called whenever w_init.size_setting is updated
void fix_w_init_size(void)
{
	switch(w_init.size_setting)
	{
		default: // should never be default
		case 0: w_init.map_size_blocks = MAP_SIZE_0; break;
		case 1: w_init.map_size_blocks = MAP_SIZE_1; break;
		case 2: w_init.map_size_blocks = MAP_SIZE_2; break;
		case 3: w_init.map_size_blocks = MAP_SIZE_3; break;
	}
}
