/**
 * \file coindemo.c
 * \author Aftersol
 * \date 2026-05-18
 * \brief A simple 2D coin collecting game example for libdragon.
 * 
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <https://unlicense.org>
 * 
 * Credits:
 *  - hassekf - Tower Defense - Grass Background
 *    https://opengameart.org/content/tower-defense-grass-background
 *  - Luke.RUSTLTD - 8-bit Coin Sound
 *    https://opengameart.org/content/10-8bit-coin-sounds
 */

#include <libdragon.h>

#include <stdlib.h>

#define MAX_COINS 10
const float speed = 2.0f;

// Returns a uniform float in [0, 1)
double uniform_rand() {
    return (double)rand() / ((double)RAND_MAX + 1.0);
}

// Returns a uniform float in [min, max)
double uniform_rand_range(double min, double max) {
    return min + (max - min) * uniform_rand();
}

int main() {
    float player_x, player_y;
    float coin_x[MAX_COINS], coin_y[MAX_COINS];
    unsigned int coin_collected = 0;
    int seed; // For random number generator
    resolution_t display_res = RESOLUTION_640x480; // Set display resolution

    debug_init_emulog();
    debug_init_usblog();

    display_init(
        display_res,
        DEPTH_16_BPP,
        2,
        GAMMA_NONE,
        FILTERS_DISABLED
    );

    dfs_init(DFS_DEFAULT_LOCATION);
    joypad_init();
    rdpq_init();
    audio_init(22050, 3);
    mixer_init(4);

    // Random number generator initialization
    getentropy(&seed, sizeof(seed));
    srand(seed);
    register_VI_handler((void(*)(void))rand);

    sprite_t* background = sprite_load("rom:/background.sprite");
    sprite_t* player = sprite_load("rom:/player.sprite");
    sprite_t* coin = sprite_load("rom:/coin.sprite");

    wav64_t coin_sound;
    wav64_open(&coin_sound, "rom:/coin.wav64");

    rdpq_font_t *font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO);
    rdpq_text_register_font(1, font);

    // Center player to center of screen
    player_x = (display_res.width/2.0f)-player->width/2.0f;
    player_y = (display_res.height/2.0f)-player->height/2.0f;

    // Scatter coins throughout the screen
    for (int i = 0; i < MAX_COINS; i++) {
        coin_x[i] = uniform_rand_range(0, display_res.width - coin->width);
        coin_y[i] = uniform_rand_range(0, display_res.height - coin->height);
    }

    while (1) {
        surface_t* disp;

        joypad_inputs_t joypad_port_1 = joypad_get_inputs(JOYPAD_PORT_1);
        joypad_buttons_t button_port_1 = joypad_get_buttons_held(JOYPAD_PORT_1);

        float speed_x = 0.0f, speed_y = 0.0f;

        while(!(disp = display_try_get())) {;}

        joypad_poll();
        mixer_try_play(); // Required for playing sound

        rdpq_blitparms_t background_blit_params = {
            .cx = 0,
            .cy = 0, 
            .scale_x = background->width / (background->width / disp->width),
            .scale_y = background->height / (background->height / disp->height)
        };
        
        // 85.0f was picked because it is the practical maximum number when 
        // the joystick is shifted all the way in one direction

        speed_x += (joypad_port_1.stick_x / 85.0f) * speed;
        speed_y -= (joypad_port_1.stick_y / 85.0f) * speed;

        if (button_port_1.d_up || button_port_1.c_up) speed_y -= speed;
        if (button_port_1.d_down || button_port_1.c_down) speed_y += speed;
        if (button_port_1.d_left || button_port_1.c_left) speed_x -= speed;
        if (button_port_1.d_right || button_port_1.c_right) speed_x += speed;

        // Clamp speed to make sure player don't go too fast
        if (speed_x > speed) speed_x = speed;
        if (speed_x < -speed) speed_x = -speed;
        if (speed_y > speed) speed_y = speed;
        if (speed_y < -speed) speed_y = -speed;

        player_x += speed_x;
        player_y += speed_y;

        // Clamp player position to prevent player from leaving screen
        if (player_x < 0.0f)
            player_x = 0.0f;

        if (player_x > (float)disp->width - (float)player->width)
            player_x = (float)disp->width - (float)player->width;

        if (player_y < 0.0f)
            player_y = 0.0f;

        if (player_y > (float)disp->height - (float)player->height)
            player_y = (float)disp->height - (float)player->height;

        // Iterate through coins to check if player touches them
        for (int i = 0; i < MAX_COINS; i++) {
            // Player-coin 2D box collision code
            if (!(coin_x[i] > player_x + player->width ||
                coin_x[i] + coin->width < player_x ||
                coin_y[i] > player_y + player->height ||
                coin_y[i] + coin->height < player_y 
            )) {
                // Teleports coin to different position
                coin_x[i] = uniform_rand_range(0, disp->width - coin->width);
                coin_y[i] = uniform_rand_range(0, disp->height - coin->height);

                // Give player a coin anyways
                coin_collected++;

                // Play a ding sound for feedback
                wav64_play(&coin_sound, 0);
            }
        }

        rdpq_attach(disp, NULL);
        rdpq_set_mode_copy(true);
        rdpq_sprite_blit(background, 0, 0, &background_blit_params);
        // Crude instancing draw coins code
        for (int i = 0; i < MAX_COINS; i++) {
            rdpq_sprite_blit(coin, coin_x[i], coin_y[i], NULL);
        }
        rdpq_sprite_blit(player, player_x, player_y, NULL);
        rdpq_set_mode_standard();
                    rdpq_text_printf(&(rdpq_textparms_t) {
                    .width = disp->width-32,
                    .align = ALIGN_LEFT,
                    .wrap = WRAP_WORD,
        }, 1, 32, 32, "Coins: %u", coin_collected);

        rdpq_detach_show(); // Send the final result to screen to be displayed

    }
}
