/**
 * \file coindemo.c
 * \author Aftersol
 * \date 2026-05-11
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

#define MAX_COINS 10

int main() {
    float player_x, player_y;
    float coin_x[MAX_COINS], coin_y[MAX_COINS];
    unsigned int coin_collected = 0;
    int seed;

    debug_init_emulog();
    debug_init_usblog();

    dfs_init(DFS_DEFAULT_LOCATION);
    display_init(
        RESOLUTION_320x240,
        DEPTH_16_BPP,
        2,
        GAMMA_NONE,
        FILTERS_DISABLED
    );

    joypad_init();
    rdpq_init();
    audio_init(48000, 3);
    mixer_init(32);
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
    player_x = (320.0f/2.0f)-player->width/2.0f;
    player_y = (240.0f/2.0f)-player->height/2.0f;

    for (int i = 0; i < MAX_COINS; i++) {
        coin_x[i] = rand() % (320-coin->width);
        coin_y[i] = rand() % (240-coin->height);
    }

    while (1) {
        surface_t* disp;

        joypad_inputs_t joypad_port_1 = joypad_get_inputs(JOYPAD_PORT_1);
        joypad_buttons_t button_port_1 = joypad_get_buttons_held(JOYPAD_PORT_1);

        float speed_x = 0.0f, speed_y = 0.0f;

        joypad_poll();
        mixer_try_play();
        
        speed_x += (joypad_port_1.stick_x / 85.0f) * 2.0f;
        speed_y -= (joypad_port_1.stick_y / 85.0f) * 2.0f;

        if (button_port_1.d_up || button_port_1.c_up) speed_y -= 2.0f;
        if (button_port_1.d_down || button_port_1.c_down) speed_y += 2.0f;
        if (button_port_1.d_left || button_port_1.c_left) speed_x -= 2.0f;
        if (button_port_1.d_right || button_port_1.c_right) speed_x += 2.0f;

        if (speed_x > 2.0f) speed_x = 2.0f;
        if (speed_x < -2.0f) speed_x = -2.0f;
        if (speed_y > 2.0f) speed_y = 2.0f;
        if (speed_y < -2.0f) speed_y = -2.0f;

        player_x += speed_x;
        player_y += speed_y;

        if (player_x < 0.0f) player_x = 0.0f;
        if (player_x > 320.0f - (float)player->width) player_x = 320.0f - (float)player->width;
        if (player_y < 0.0f) player_y = 0.0f;
        if (player_y > 240.0f - (float)player->height) player_y = 240.0f - (float)player->height;

        for (int i = 0; i < MAX_COINS; i++) {
            if (coin_x[i] != -1 && coin_y[i] != -1) {
                if (!(coin_x[i] > player_x + player->width ||
                    coin_x[i] + coin->width < player_x ||
                    coin_y[i] > player_y + player->height ||
                    coin_y[i] + coin->height < player_y 
                )) {
                    coin_x[i] = rand() % (320 - coin->width);
                    coin_y[i] = rand() % (240 - coin->height);
                    coin_collected++;
                    wav64_play(&coin_sound, 0);
                }
            }
        }

        while(!(disp = display_try_get())) {;}

        rdpq_attach(disp, NULL);
        rdpq_set_mode_copy(true);
        rdpq_sprite_blit(background, 0, 0, NULL);
        for (int i = 0; i < MAX_COINS; i++) {
            rdpq_sprite_blit(coin, coin_x[i], coin_y[i], NULL);
        }
        rdpq_sprite_blit(player, player_x, player_y, NULL);
        rdpq_set_mode_standard();
                    rdpq_text_printf(&(rdpq_textparms_t) {
                    .width = 320-32,
                    .align = ALIGN_LEFT,
                    .wrap = WRAP_WORD,
        }, 1, 32, 32, "Coins: %u", coin_collected);

        rdpq_detach_show();

    }
}
