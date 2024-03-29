/** @file    gameboard.c
 *  @authors Joshua Byoungsoo Kim (bki42), Jay Brydon (jbr268)
 *  @date    10 Oct 2023
 *  @brief   it shows current game board status on LED matrix
*/


#include "system.h"
#include "pacer.h"
#include "navswitch.h"
#include "gameboard.h"
#include "tinygl.h"
#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>

/* Flag to turn killer position LED ON and OFF. */
static uint8_t killer_blink_flag = 0;

/** Initialize navswitch*/
void nav_init(void) 
{
    navswitch_init ();
}

/** Update navswitch*/
void nav_update(void) 
{
    navswitch_update ();
}

/** Make killer point blinking
 * @param killer killer's location (x,y coordinate)
*/
void killer_blink(killer_t killer) 
{
    if (killer_blink_flag == 0) {
        tinygl_draw_point(killer.pos, 1);
        killer_blink_flag = 1;
    } else {
        tinygl_draw_point(killer.pos, 0);
        killer_blink_flag = 0;
    }
}

/** Check whether the given spot is filled by a bug
 * return index number of array of bugs if there is bug
 * return -1 if there is no bug
 * @param dots the pointer for the array of the each bug position(x,y coordinates)
 * @param total_bug_num the total number of bugs in the stage
 * @param pos the coordinates(x,y) that want to be check
*/
int8_t bug_check(bugs_t *dots, uint8_t total_bug_num, tinygl_point_t pos)
{
    for (uint8_t i = 0; i < total_bug_num; i++) {
        if (dots[i].pos.x == pos.x && dots[i].pos.y == pos.y && dots[i].status) {
            return i;
        }
    }
    return -1;
}

/** Change x,y coordinates of killer
 * @param dots the pointer for the array of the each bug position(x,y coordinates)
 * @param killer the pointer for the player's location as x,y coordinates
 * @param dx x coordinate that player wants to move
 * @param dy y coordinate that player wants to move
 */
void killer_move(bugs_t *dots, killer_t *killer, int8_t dx, int8_t dy)
{
    tinygl_point_t n_pos;

    n_pos = tinygl_point (killer->pos.x + dx, killer->pos.y + dy);
    /* check whether there was bug on the location before the player move in */
    if ( bug_check (dots, TOTAL_SPOTS, killer->pos) != -1) {
        tinygl_draw_point (killer->pos,1);
    } else {
        tinygl_draw_point (killer->pos,0);
    } 
    killer->pos = n_pos;
    tinygl_draw_point (killer->pos, 1);
}

/** 
 * Move killer position according to the navswitch control when navswitch is pushed, 
 * it will kill the bug if there is bug on that spot
 * @param bugs the pointer for the array of the each bug position(x,y coordinates)
 * @param killer the pointer for the player's location as x,y coordinates
 * @param total_bug_num the total bugs number that the player start with in the stage
 */
uint8_t killer_control (bugs_t *bugs, killer_t *killer, uint8_t total_bug_num) {

    if (navswitch_push_event_p (NAVSWITCH_NORTH)) 
        killer_move(bugs, killer, 0,-1);
    if (navswitch_push_event_p (NAVSWITCH_SOUTH)) 
        killer_move(bugs, killer, 0,1);
    if (navswitch_push_event_p (NAVSWITCH_WEST))
        killer_move(bugs, killer, -1,0);
    if (navswitch_push_event_p (NAVSWITCH_EAST)) 
        killer_move(bugs, killer, 1,0);
    if (navswitch_push_event_p (NAVSWITCH_PUSH)) 
    {
        int8_t i = bug_check (bugs, total_bug_num, killer->pos);
        if ( i != -1 && bugs[i].status) {
            tinygl_draw_point (bugs[i].pos, 0);
            bugs[i].status = false;
            return 1;
        }
    }
    return 0;
}

/** create bugs on the LED matrix based on different stages 
 * @param bugs the pointer for the array of the each bug position(x,y coordinates)
 * @param total_bug_num the total bugs number that the player start with in the stage
*/
void bugs_create (bugs_t *bugs, uint8_t total_bug_num)
{   
    for (uint8_t i = 0; i < TOTAL_SPOTS; i++) {
        for (uint8_t dx = 0; dx < TINYGL_WIDTH; dx++) {
            for (uint8_t dy = 0; dy < TINYGL_HEIGHT; dy++) {
                bugs[i].pos.x = dx;
                bugs[i].pos.y = dy;
                bugs[i].status = false;
            }
        }
    }

    for (uint8_t i = 0; i < total_bug_num; i++)
    {
        uint8_t x;
        uint8_t y;

        /* This will plot the points/bugs on the LED screen for each stage. It utilises the
            random number generator rand(), and TCNT1, as the seed of the random
            number generator, to plot them randomly. */
        do {
            srand(TCNT1);
            x = rand() % TINYGL_WIDTH;
            srand(TCNT1);
            y = rand() % TINYGL_HEIGHT;
        } while (bug_check (bugs, i, tinygl_point (x, y)) != -1);
        
        bugs[i].pos.x = x;
        bugs[i].pos.y = y;
        bugs[i].status = true;

        tinygl_draw_point (bugs[i].pos, 1);
    }
}
