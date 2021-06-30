#include "Engine.h"
#include <stdlib.h>
#include <memory.h>

//
//  IDEAS:
//  Vector for all objects, projecting their possition on the buffer
//  Each object is a class derieved from the parent-object class with the sizes
//  For bullets DTL (Distants to live)
//  Collision detection (if asteroids - change direction [probably don't implement it], if player - take the life and don't change the direction)
//  Different asteroid types (difficulties), different colors and speed (maybe diffirent recurse types)
//  Create default BG
//  From creation position at the start at radius R no asteroids are spawned
//  If player dies in N first seconds after respawn - no penalties
//  When asteroid breaks into 2 part - tangential speed remains the same and normal speed are create for purpose of 30 degrees for initial derection
//  Few levels with increasing number of faster asteroids (probably max level = all M-type asteroids)
//  LEADERBOARD stored in file as binary data
//

//
//  EXPLANATIONS:
//  BGRA32 - blue, green, red, alpha - 8 bit for each
//  VK_LEFT/VK_RIGHT - rotation, VK_UP - acceleration, VK_SPACE - shooting
//  is_key_pressed(int button_vk_code) - check if a key is pressed,
//                                       use keycodes (VK_SPACE, VK_RIGHT, VK_LEFT, VK_UP, VK_DOWN, 'A', 'B')
//
//  get_cursor_x(), get_cursor_y() - get mouse cursor position                                                 - WE DON'T NEED IT
//  is_mouse_button_pressed(int button) - check if mouse button is pressed (0 - left button, 1 - right button) - WE DON'T NEED IT
//  clear_buffer() - set all pixels in buffer to 'black'
//  is_window_active() - returns true if window is active
//  schedule_quit_game() - quit game after act()
//

// initialize game data in this function
void initialize()
{
}

// this function is called to update game data,
// dt - time elapsed since the previous update (in seconds)
void act(float dt)
{
  if (is_key_pressed(VK_ESCAPE))
    schedule_quit_game();
}

// fill buffer in this function
// uint32_t buffer[SCREEN_HEIGHT][SCREEN_WIDTH] - is an array of 32-bit colors (8 bits per R, G, B)
void draw()
{
  // clear backbuffer
  memset(buffer, 0, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));

}

// free game data in this function
void finalize()
{
}

