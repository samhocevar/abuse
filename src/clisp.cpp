/*
 *  Abuse — dark 2D side-scrolling platform game
 *  Copyright © 1995 Crack dot Com
 *  Copyright © 2005—2018 Sam Hocevar <sam@hocevar.net>
 *
 *  This software was released into the Public Domain. As with most public
 *  domain software, no warranty is made or implied by Crack dot Com, by
 *  Jonathan Clark, or by Sam Hocevar.
 */

#if HAVE_CONFIG_H
#   include "config.h"
#endif

#include <string.h>

#if defined HAVE_UNISTD_H
#   include <unistd.h> /* for getcwd */
#endif

#include "common.h"

#include "lisp/lisp.h"
#include "lisp/gc.h"
#include "lisp/symbols.h"

#include "imlib/pcxread.h"
#include "imlib/dprint.h"
#include "imlib/jdir.h"

#include "sdlport/joy.h"

#include "ant.h"
#include "game.h"
#include "dev.h"
#include "menu.h"
#include "clisp.h"
#include "chars.h"
#include "cop.h"
#include "loadgame.h"
#include "nfserver.h"
#include "demo.h"
#include "chat.h"
#include "netcfg.h"

#define ENGINE_MAJOR 1
#define ENGINE_MINOR 20

extern int has_joystick;

// the following are references to lisp symbols
LSymbol *l_chat_input, *l_post_render;

LSymbol *l_difficulty, *l_easy, *l_hard, *l_medium, *l_extreme,
        *l_max_hp, *l_max_power, *l_empty_cache;

// FIXME: port these to LSymbol
LSymbol *l_main_menu, *l_logo, *l_state_art, *l_abilities, *l_state_sfx,
        *l_song_list, *l_filename, *l_sfx_directory, *l_default_font,
        *l_morph, *l_default_abilities, *l_default_ai_function,
        *l_tile_files, *l_range, *l_hurt_all, *l_death_handler,
        *l_title_screen, *l_console_font, *l_fields, *l_dist, *l_pushx, *l_pushy,
        *l_object, *l_tile, *l_fire_object, *l_FIRE, *l_cop_dead_parts, *l_restart_player,
        *l_help_screens, *l_player_draw, *l_sneaky_draw, *l_health_image, *l_fly_image,
        *l_sneaky_image, *l_draw_fast, *l_player_tints, *l_save_order, *l_next_song,
        *l_level_load_start,
        *l_level_load_end,
        *l_cdc_logo,
        *l_keep_backup,
        *l_switch_to_powerful,
        *l_mouse_can_switch,
        *l_ask_save_slot,
        *l_get_local_input,
        *l_player_text_color,
        *l_level_loaded;        // called when a new level is loaded


char game_name[50];
LObject *sensor_ai();

// variables for the status bar
LSymbol *l_statbar_ammo_x, *l_statbar_ammo_y,
        *l_statbar_ammo_w, *l_statbar_ammo_h,
        *l_statbar_ammo_bg_color,
        *l_statbar_health_x, *l_statbar_health_y,
        *l_statbar_health_w, *l_statbar_health_h,
        *l_statbar_health_bg_color,
        *l_statbar_logo_x, *l_statbar_logo_y;

uint8_t chatting_enabled = 0;

extern void show_end();

static view *lget_view(LObject *arg, char const *msg)
{
  GameObject *o = (GameObject *)lpointer_value(arg);
  view *c = o->m_controller;
  if (!c)
  {
    dprintf("%s: object does not have a view\n", msg);
    lbreak("");
    exit(0);
  }
  return c;
}

extern int get_option(char const *name);
extern void set_login(char const *name);

void clisp_init() // called by lisp_init, defines symbols and functions
                  // to interface with c
{
    l_easy = lisp::make_sym("easy");
    l_medium = lisp::make_sym("medium");
    l_hard = lisp::make_sym("hard");
    l_extreme = lisp::make_sym("extreme");

    l_logo = lisp::make_sym("logo");
    l_morph = lisp::make_sym("morph");

    l_pushx = lisp::make_sym("pushx");
    l_pushy = lisp::make_sym("pushy");

    l_dist = lisp::make_sym("dist");
    l_state_art = lisp::make_sym("state_art");
    l_abilities = lisp::make_sym("abilities");
    l_default_abilities = lisp::make_sym("default_abilities");
    l_state_sfx = lisp::make_sym("state_sfx");
    l_filename = lisp::make_sym("filename");
    l_sfx_directory = lisp::make_sym("sfx_directory");
    l_default_font = lisp::make_sym("default_font");
    l_console_font = lisp::make_sym("console_font");
    l_default_ai_function = lisp::make_sym("default_ai");
    l_tile_files = lisp::make_sym("tile_files");
    l_empty_cache = lisp::make_sym("empty_cache");
    l_range = lisp::make_sym("range");
    l_difficulty = lisp::make_sym("difficulty");
    l_death_handler = lisp::make_sym("death_handler");
    l_title_screen = lisp::make_sym("title_screen");
    l_fields = lisp::make_sym("fields");
    l_FIRE = lisp::make_sym("FIRE");
    l_fire_object = lisp::make_sym("fire_object");
    l_cop_dead_parts = lisp::make_sym("cop_dead_parts");
    l_difficulty->SetValue(l_hard);
    l_restart_player = lisp::make_sym("restart_player");
    l_help_screens = lisp::make_sym("help_screens");
    l_save_order = lisp::make_sym("save_order");
    l_next_song = lisp::make_sym("next_song");
    l_player_draw = lisp::make_sym("player_draw");
    l_sneaky_draw = lisp::make_sym("sneaky_draw");
    l_keep_backup = lisp::make_sym("keep_backup");
    l_level_loaded = lisp::make_sym("level_loaded");

    l_draw_fast = lisp::make_sym("draw_fast");
    l_player_tints = lisp::make_sym("player_tints");

    l_max_hp = lisp::make_sym("max_hp");
    l_max_hp->SetNumber(200);
    l_max_power = lisp::make_sym("max_power");
    l_main_menu = lisp::make_sym("main_menu");
    l_max_power->SetNumber(999);

    lisp::make_sym("run_state")->SetNumber(RUN_STATE);
    lisp::make_sym("pause_state")->SetNumber(PAUSE_STATE);
    lisp::make_sym("menu_state")->SetNumber(MENU_STATE);
    lisp::make_sym("scene_state")->SetNumber(SCENE_STATE);

    l_statbar_ammo_x = lisp::make_sym("statbar_ammo_x");
    l_statbar_ammo_y = lisp::make_sym("statbar_ammo_y");
    l_statbar_ammo_w = lisp::make_sym("statbar_ammo_w");
    l_statbar_ammo_h = lisp::make_sym("statbar_ammo_h");
    l_statbar_ammo_bg_color = lisp::make_sym("statbar_ammo_bg_color");

    l_statbar_health_x = lisp::make_sym("statbar_health_x");
    l_statbar_health_y = lisp::make_sym("statbar_health_y");
    l_statbar_health_w = lisp::make_sym("statbar_health_w");
    l_statbar_health_h = lisp::make_sym("statbar_health_h");
    l_statbar_health_bg_color = lisp::make_sym("statbar_health_bg_color");

    l_statbar_logo_x = lisp::make_sym("statbar_logo_x");
    l_statbar_logo_y = lisp::make_sym("statbar_logo_y");
    l_object = lisp::make_sym("object");
    l_tile = lisp::make_sym("tile");
    l_cdc_logo = lisp::make_sym("logo");

    l_switch_to_powerful = lisp::make_sym("switch_to_powerful");
    l_mouse_can_switch = lisp::make_sym("mouse_can_switch");
    l_ask_save_slot = lisp::make_sym("ask_save_slot");

    l_level_load_start = lisp::make_sym("level_load_start");
    l_level_load_end = lisp::make_sym("level_load_end");
    l_get_local_input = lisp::make_sym("get_local_input");
    l_chat_input = lisp::make_sym("chat_input");
    l_player_text_color = lisp::make_sym("player_text_color");

    for (int i = 0; i < MAX_STATE; ++i)
        lisp::make_sym(state_names[i])->SetNumber(i);
    for (int i = 0; i < TOTAL_ABILITIES; ++i)
        lisp::make_sym(ability_names[i])->SetNumber(i);
    for (int i = 0; i < TOTAL_CFLAGS; ++i)
        lisp::make_sym(cflag_names[i])->SetNumber(i);

    l_song_list = lisp::make_sym("song_list");
    l_post_render = lisp::make_sym("post_render");

    add_c_function("distx", 0, 0,                   1);
    add_c_function("disty", 0, 0,                   2);
    add_c_bool_fun("key_pressed", 1, 1,             3);
    add_c_bool_fun("local_key_pressed", 1, 1,       4);

    add_c_function("bg_state", 0, 0,                5);
    add_c_function("aitype", 0, 0,                  6);
    add_c_function("aistate", 0, 0,                 7);
    add_c_function("set_aistate", 1, 1,             8);
    add_c_function("random", 1, 1,                  9);
    add_c_function("state_time", 0, 0,             10);
    add_c_function("state", 0, 0,                  11);
    add_c_function("toward", 0, 0,                 12);
    add_c_function("move", 3, 3,                   13);
    add_c_function("facing", 0, 0,                 14);
    add_c_function("otype", 0, 0,                  15);
    add_c_bool_fun("next_picture", 0, 0,           16);
    add_c_bool_fun("set_fade_dir", 1, 1,           17);
    add_c_function("mover", 3, 3,                  18);
    add_c_bool_fun("set_fade_count", 1, 1,         19);
    add_c_function("fade_count", 0, 0,             20);
    add_c_function("fade_dir", 0, 0,               21);
    add_c_bool_fun("touching_bg", 0, 0,            22);
    add_c_function("add_power", 1, 1,              23);
    add_c_function("add_hp", 1, 1,                 24);

    add_c_bool_fun("draw", 0, 0,                   27);
    add_c_bool_fun("edit_mode", 0, 0,              28);
    add_c_bool_fun("draw_above", 0, 0,             29);
    add_c_function("x", 0, 0,                      30);
    add_c_function("y", 0, 0,                      31);
    add_c_bool_fun("set_x", 1, 1,                  32);
    add_c_bool_fun("set_y", 1, 1,                  33);
    add_c_bool_fun("push_characters", 2, 2,        34);



    add_c_bool_fun("set_state", 1, 1,              37);
    add_c_function("bg_x", 0, 0,                   38);
    add_c_function("bg_y", 0, 0,                   39);
    add_c_bool_fun("set_aitype", 1, 1,             40);

    add_c_function("xvel", 0, 0,                   42);
    add_c_function("yvel", 0, 0,                   43);
    add_c_bool_fun("set_xvel", 1, 1,               44);
    add_c_bool_fun("set_yvel", 1, 1,               45);
    add_c_function("away", 0, 0,                   46);
    add_c_bool_fun("blocked_left", 1, 1,           47);
    add_c_bool_fun("blocked_right", 1, 1,          48);

    add_c_function("add_palette", 1, -1,           50);    // name, w, h, x, y, scale, tiles
    add_c_bool_fun("screen_shot", 1, 1,            51);    // filename

    add_c_bool_fun("set_zoom", 1, 1,               52);
    add_c_function("show_help", 1, 1,              55);    // type, x, y
    add_c_function("direction", 0, 0,              56);
    add_c_function("set_direction", 1, 1,          57);

    add_c_bool_fun("freeze_player", 1, 1,          58);   // freeze time

    add_c_function("menu", 1, -1,                  59);
    add_c_bool_fun("do_command", 1, 1,             60);   // command string
    add_c_bool_fun("set_game_state", 1, 1,         61);


// scene control functions, game must first be set to scene mode.
    add_c_bool_fun("scene:set_text_region", 4, 4,  62);
    add_c_bool_fun("scene:set_frame_speed", 1, 1,  63);
    add_c_bool_fun("scene:set_scroll_speed", 1, 1, 64);
    add_c_bool_fun("scene:set_pan_speed", 1, 1,    65);
    add_c_bool_fun("scene:scroll_text", 1, 1,      66);
    add_c_bool_fun("scene:pan", 3, 3,              67);
    add_c_bool_fun("scene:wait", 1, 1,             68);

    add_c_bool_fun("level:new", 3, 3,              74);    // width, height, name

    add_c_bool_fun("do_damage", 2, 4,              75);    // amount, to_object, [pushx pushy]
    add_c_function("hp", 0, 0,                     76);
    add_c_bool_fun("set_shift_down", 2, 2,         77);
    add_c_bool_fun("set_shift_right", 2, 2,        78);
    add_c_bool_fun("set_gravity", 1, 1,            79);
    add_c_function("tick", 0, 0,                   80);

    add_c_bool_fun("set_xacel", 1, 1,              81);
    add_c_bool_fun("set_yacel", 1, 1,              82);
    add_c_bool_fun("set_local_players", 1, 1,      84);   // set # of players on this machine, unsupported?
    add_c_function("local_players", 0, 0,          85);

    add_c_bool_fun("set_light_detail", 1, 1,       86);
    add_c_function("light_detail", 0, 0,           87);
    add_c_bool_fun("set_morph_detail", 1, 1,       88);
    add_c_function("morph_detail", 0, 0,           89);
    add_c_bool_fun("morph_into", 3, 3,             90);       // type aneal frames
    add_c_bool_fun("link_object", 1, 1,            91);

    add_c_bool_fun("draw_line", 5, 5,              92);
    add_c_function("dark_color", 0, 0,             93);
    add_c_function("medium_color", 0, 0,           94);
    add_c_function("bright_color", 0, 0,           95);

    add_c_bool_fun("remove_object", 1, 1,          99);
    add_c_bool_fun("link_light", 1, 1,            100);
    add_c_bool_fun("remove_light", 1, 1,          101);
    add_c_function("total_objects", 0, 0,         102);
    add_c_function("total_lights", 0, 0,          103);

    add_c_bool_fun("set_light_r1", 2, 2,          104);
    add_c_bool_fun("set_light_r2", 2, 2,          105);
    add_c_bool_fun("set_light_x", 2, 2,           106);
    add_c_bool_fun("set_light_y", 2, 2,           107);
    add_c_bool_fun("set_light_xshift", 2, 2,      108);
    add_c_bool_fun("set_light_yshift", 2, 2,      109);

    add_c_function("light_r1", 1, 1,              110);
    add_c_function("light_r2", 1, 1,              111);
    add_c_function("light_x", 1, 1,               112);
    add_c_function("light_y", 1, 1,               113);
    add_c_function("light_xshift", 1, 1,          114);
    add_c_function("light_yshift", 1, 1,          115);

    add_c_function("xacel", 0, 0,                 116);
    add_c_function("yacel", 0, 0,                 117);
    add_c_bool_fun("delete_light", 1, 1,          118);

    add_c_bool_fun("set_fx", 1, 1,                119);
    add_c_bool_fun("set_fy", 1, 1,                120);
    add_c_bool_fun("set_fxvel", 1, 1,             121);
    add_c_bool_fun("set_fyvel", 1, 1,             122);
    add_c_bool_fun("set_fxacel", 1, 1,            123);
    add_c_bool_fun("set_fyacel", 1, 1,            124);
    add_c_function("picture_width", 0, 0,         125);
    add_c_function("picture_height", 0, 0,        126);
    add_c_bool_fun("trap", 0, 0,                  127);
    add_c_bool_fun("platform_push", 2, 2,         128);

    add_c_function("def_sound", 1, 2,             133);  // symbol, filename [ or just filenmae]
    add_c_bool_fun("play_sound", 1, 4,            134);

    add_c_function("def_particle", 2, 2,          137);  // symbol, filename
    add_c_function("add_panim", 4, 4,             138);  // id, x, y, dir

    add_c_function("weapon_to_type", 1, 1,        142);  // returns total for type weapon
    add_c_bool_fun("hurt_radius", 6, 6,           143);  // x y radius max_damage exclude_object max_push

    add_c_bool_fun("add_ammo", 2, 2,              144);  // weapon_type, amount
    add_c_function("ammo_total", 1, 1,            145);  // returns total for type weapon
    add_c_function("current_weapon", 0, 0,        146);  // weapon_type, amount
    add_c_function("current_weapon_type", 0, 0,   147);  // returns total for type weapon

    add_c_bool_fun("blocked_up", 1, 1,            148);
    add_c_bool_fun("blocked_down", 1, 1,          149);
    add_c_bool_fun("give_weapon", 1, 1,           150);  // type
    add_c_function("get_ability", 1, 1,           151);
    add_c_bool_fun("reset_player", 0, 0,          152);
    add_c_function("site_angle", 1, 1,            153);
    add_c_bool_fun("set_course", 2, 2,            154);  // angle, magnitude
    add_c_bool_fun("set_frame_angle", 3, 3,       155);  // ang1, ang2, ang
    add_c_bool_fun("jump_state", 1, 1,            156);  // don't reset current_frame

    add_c_bool_fun("morphing", 0, 0,              168);
    add_c_bool_fun("damage_fun", 6, 6,            169);
    add_c_bool_fun("gravity", 0, 0,               170);
    add_c_bool_fun("make_view_solid", 1, 1,       171);
    add_c_function("find_rgb", 3, 3,              172);

    add_c_function("player_x_suggest", 0, 0,      173);  // return player "joystick" x
    add_c_function("player_y_suggest", 0, 0,      174);
    add_c_function("player_b1_suggest", 0, 0,     175);
    add_c_function("player_b2_suggest", 0, 0,     176);
    add_c_function("player_b3_suggest", 0, 0,     177);

    add_c_bool_fun("set_bg_scroll", 4, 4,         178);  // xmul xdiv ymul ydiv
    add_c_bool_fun("set_ambient_light", 2, 2,     179);  // player, 0..63 (out of bounds ignored)
    add_c_function("ambient_light", 1, 1,         180);  // player
    add_c_bool_fun("has_object", 1, 1,            181);  // true if linked with object x
    add_c_bool_fun("set_otype", 1, 1,             182);  // otype

    add_c_function("current_frame", 0, 0,         184);
    add_c_function("fx", 0, 0,                    185);
    add_c_function("fy", 0, 0,                    186);
    add_c_function("fxvel", 0, 0,                 187);
    add_c_function("fyvel", 0, 0,                 188);
    add_c_function("fxacel", 0, 0,                189);
    add_c_function("fyacel", 0, 0,                190);
    add_c_bool_fun("set_stat_bar", 2, 2,          191);  // filename, object
    add_c_bool_fun("set_fg_tile", 3, 3,           192);  // x, y, tile #
    add_c_function("fg_tile", 2, 2,               193);  // x, y
    add_c_bool_fun("set_bg_tile", 3, 3,           194);  // x, y, tile #
    add_c_function("bg_tile", 2, 2,               195);  // x, y
    add_c_bool_fun("load_tiles", 1, -1,           196);  // filename1, filename2...
    add_c_bool_fun("load_palette", 1, 1,          197);  // filename
    add_c_bool_fun("load_color_filter", 1, 1,     198);  // filename
    add_c_bool_fun("create_players", 1, 1,        199);  // player type, returns true if game is networked
    add_c_bool_fun("try_move", 2, 3,              200);  // xv yv (check_top=t) -> returns T if not blocked
    add_c_function("sequence_length", 1, 1,       201);  // sequence number
    add_c_bool_fun("can_see", 5, 5,               202);  // x1, y1, x2, y2, chars_block
    add_c_function("load_big_font", 2, 2,         203);  // filename, name
    add_c_function("load_small_font", 2, 2,       204);  // filename, name
    add_c_function("load_console_font", 2, 2,     205);  // filename, name
    add_c_function("set_current_frame", 1, 1,     206);

    add_c_bool_fun("draw_transparent", 2, 2,      208);  // count, max
    add_c_bool_fun("draw_tint", 1, 1,             209);  // tint id number
    add_c_bool_fun("draw_predator", 0, 0,         210);  // tint_number

    add_c_function("shift_down", 1, 1,            211);  // player
    add_c_function("shift_right", 1, 1,           212);  // player
    add_c_bool_fun("set_no_scroll_range", 5, 5,   213);  // player, x1, y1, x2, y2

    add_c_function("def_image", 2, 2,             215);  // filename, name
    add_c_bool_fun("put_image", 3, 3,             216);  // x, y, id
    add_c_function("view_x1", 0, 0,               217);
    add_c_function("view_y1", 0, 0,               218);
    add_c_function("view_x2", 0, 0,               219);
    add_c_function("view_y2", 0, 0,               220);
    add_c_function("view_push_down", 1, 1,        221);
    add_c_bool_fun("local_player", 0, 0,          222);
    add_c_bool_fun("save_game", 1, 1,             223);  // filename
    add_c_bool_fun("set_hp", 1, 1,                224);
    add_c_bool_fun("request_level_load", 1, 1,    225);  // filename
    add_c_bool_fun("set_first_level", 1, 1,       226);  // filename
    add_c_function("def_tint", 1, 1,              227);  // filename
    add_c_function("tint_palette", 3, 3,          228);  // radd, gadd, badd
    add_c_function("player_number", 0, 0,         229);
    add_c_bool_fun("set_current_weapon", 1, 1,    230);  // type
    add_c_bool_fun("has_weapon", 1, 1,            231);  // type
    add_c_bool_fun("ambient_ramp", 1, 1,          232);
    add_c_function("total_players", 0, 0,         233);
    add_c_bool_fun("scatter_line", 6, 6,          234);  // x1, y1, x2, y2, color, scatter value
    add_c_function("game_tick", 0, 0,             235);
    add_c_bool_fun("isa_player", 0, 0,            236);
    add_c_bool_fun("shift_rand_table", 1, 1,      237);  // amount
    add_c_function("total_frames", 0, 0,          238);
    add_c_function("raise", 0, 0,                 239);  // call only from reload constructor!
    add_c_function("lower", 0, 0,                 240);  // call only from reload constructor!

    add_c_function("player_pointer_x", 0, 0,      241);
    add_c_function("player_pointer_y", 0, 0,      242);
    add_c_bool_fun("frame_panic", 0, 0,           243);
    add_c_bool_fun("ascatter_line", 7, 7,         244);  // x1, y1, x2, y2, color1, color2, scatter value
    add_c_function("rand_on", 0, 0,               245);
    add_c_function("set_rand_on", 1, 1,           246);
    add_c_function("bar", 5, 5,                   247);
    add_c_function("argc", 0, 0,                  248);
    add_c_bool_fun("play_song", 1, 1,             249);  // filename
    add_c_bool_fun("stop_song", 0, 0,             250);
    add_c_bool_fun("targetable", 0, 0,            251);
    add_c_bool_fun("set_targetable", 1, 1,        252);  // T or nil
    add_c_bool_fun("show_stats", 0, 0,            253);

    add_c_function("kills", 0, 0,                 254);
    add_c_function("tkills", 0, 0,                255);
    add_c_function("secrets", 0, 0,               256);
    add_c_function("tsecrets", 0, 0,              257);

    add_c_bool_fun("set_kills", 1, 1,             258);
    add_c_bool_fun("set_tkills", 1, 1,            259);
    add_c_bool_fun("set_secrets", 1, 1,           260);
    add_c_bool_fun("set_tsecrets", 1, 1,          261);
    add_c_bool_fun("request_end_game", 0, 0,      262);
    add_c_function("get_save_slot", 0, 0,         263);
    add_c_bool_fun("mem_report", 0, 0,            264);
    add_c_function("major_version", 0, 0,         265);
    add_c_function("minor_version", 0, 0,         266);
    add_c_bool_fun("draw_double_tint", 2, 2,      267);  // tint1 id number, tint 2 id number
    add_c_function("image_width", 1, 1,           268);  // image number
    add_c_function("image_height", 1, 1,          269);  // image number
    add_c_function("foreground_width", 0, 0,      270);
    add_c_function("foreground_height", 0, 0,     271);
    add_c_function("background_width", 0, 0,      272);
    add_c_function("background_height", 0, 0,     273);
    add_c_function("get_key_code", 1, 1,          274);  // name of key, returns code that can be used with keypressed
    add_c_bool_fun("set_cursor_shape", 3, 3,      275);  // image id, x hot, y hot
    add_c_bool_fun("start_server", 0, 0,          276);
    add_c_bool_fun("put_string", 4, 5,            277);  // font, x, y, string, [color]
    add_c_function("font_width", 1, 1,            278);  // font
    add_c_function("font_height", 1, 1,           279);  // font
    add_c_bool_fun("chat_print", 1, 1,            280);  // chat string
    add_c_bool_fun("set_player_name", 1, 1,       281);  // name
    add_c_bool_fun("draw_bar", 5, 5,              282);  // x1, y1, x2, y2, color
    add_c_bool_fun("draw_rect", 5, 5,             283);  // x1, y1, x2, y2, color
    add_c_bool_fun("get_option", 1, 1,            284);
    add_c_bool_fun("set_delay_on", 1, 1,          288);  // T or nil
    add_c_bool_fun("set_login", 1, 1,             289);  // name
    add_c_bool_fun("enable_chatting", 0, 0,       290);
    add_c_bool_fun("demo_break_enable", 0, 0,     291);
    add_c_bool_fun("am_a_client", 0, 0,           292);
    add_c_bool_fun("time_for_next_level", 0, 0,   293);
    add_c_bool_fun("reset_kills", 0, 0,           294);
    add_c_bool_fun("set_game_name", 1, 1,         295);  // server game name
    add_c_bool_fun("set_net_min_players", 1, 1,   296);

    add_c_bool_fun("set_object_tint", 1, 1,    1001);  // set_object_tint
    add_c_function("get_object_tint", 0, 0,    1002);  // get_object_tint
    add_c_bool_fun("set_object_team", 1, 1,    1003);  // set_object_team
    add_c_function("get_object_team", 0, 0,    1004);  // get_object_tint


    add_lisp_function("go_state", 1, 1,              0);
    add_lisp_function("with_object", 2, -1,          1);
    add_lisp_function("bmove", 0, 1,                 2);   // returns true=unblocked, nil=block, or object
    add_lisp_function("me", 0, 0,                    3);
    add_lisp_function("bg", 0, 0,                    4);
    add_lisp_function("find_closest", 1, 1,          5);
    add_lisp_function("find_xclosest", 1, 1,         6);
    add_lisp_function("find_xrange", 2, 2,           7);
    add_lisp_function("add_object", 3, 4,            8);    // type, x, y (type)
    add_lisp_function("first_focus", 0, 0,           9);
    add_lisp_function("next_focus", 1, 1,           10);
    add_lisp_function("get_object", 1, 1,           11);
    add_lisp_function("get_light", 1, 1,            12);
    add_lisp_function("with_objects", 1, 1,         13);
    add_lisp_function("add_light", 7, 7,            14);   // type, x, y, r1, r2, xshift, yshift
    add_lisp_function("find_enemy", 1, 1,           15);   // exclude
    add_lisp_function("user_fun", 0, -1,            16);   // calls anobject's user function
    add_lisp_function("time", 2, 2,                 17);   // returns a fixed point (times and operation)
    add_lisp_function("name", 0, 0,                 18);   // returns current object's name (for debugin)
    add_lisp_function("float_tick", 0, 0,           19);
    add_lisp_function("find_object_in_area", 5, 5,  20);  // x1, y1, x2, y2  type_list
    add_lisp_function("find_object_in_angle", 3, 3, 21);  // start_angle end_angle type_list
    add_lisp_function("add_object_after", 3, 4,     22);  // type, x, y (type)
    add_lisp_function("def_char", 2, -1,            23);  // needs at least 2 parms
    add_lisp_function("see_dist", 4, 4,             24);  // returns longest unblocked path from x1, y1, x2, y2
    add_lisp_function("platform", 0, 0,             25);
    add_lisp_function("level_name", 0, 0,           26);
    add_lisp_function("ant_ai", 0, 0,               27);
    add_lisp_function("sensor_ai", 0, 0,            28);
    add_lisp_function("dev_draw", 0, 0,             29);
    add_lisp_function("top_ai", 0, 0,               30);
    add_lisp_function("laser_ufun", 2, 2,           31);
    add_lisp_function("top_ufun", 2, 2,             32);

    add_lisp_function("player_rocket_ufun", 2, 2,   34);

    add_lisp_function("plaser_ufun", 2, 2,          33);
    add_lisp_function("lsaber_ufun", 2, 2,          35);

    add_lisp_function("cop_mover", 3, 3,            36);
    add_lisp_function("latter_ai", 0, 0,            37);
    add_lisp_function("with_obj0", -1, -1,          38);
    add_lisp_function("activated", 0, 0,            39);
    add_lisp_function("top_draw", 0, 0,             40);
    add_lisp_function("bottom_draw", 0, 0,          41);
    add_lisp_function("mover_ai", 0, 0,             42);
    add_lisp_function("sgun_ai", 0, 0,              43);
    add_lisp_function("last_savegame_name", 0, 0,   44);
    add_lisp_function("next_savegame_name", 0, 0,   45);
    add_lisp_function("argv", 1, 1,                 46);
    add_lisp_function("joy_stat", 0, 0,             47);  // xm ym b1 b2 b3
    add_lisp_function("mouse_stat", 0, 0,           48);  // mx my b1 b2 b3
    add_lisp_function("mouse_to_game", 2, 2,        49);  // pass in x, y -> x, y
    add_lisp_function("game_to_mouse", 2, 2,        50);  // pass in x, y -> x, y
    add_lisp_function("get_main_font", 0, 0,        51);
    add_lisp_function("player_name", 0, 0,          52);
    add_lisp_function("get_cwd", 0, 0,              54);
    add_lisp_function("system", 1, 1,               55);
    add_lisp_function("convert_slashes", 2, 2,      56);
    add_lisp_function("get_directory", 1, 1,        58);  // path
    add_lisp_function("respawn_ai", 0, 0,           60);

    add_lisp_function("score_draw", 0, 0,           61);
    add_lisp_function("show_kills", 0, 0,           62);
    add_lisp_function("mkptr", 1, 1,                63);
    add_lisp_function("seq", 3, 3,                  64);
}


// Note: args for l_caller have not been evaluated yet!
LObject *l_caller(long number, LObject *args)
{
    // Shortcut to evaluate nth argument
    auto get_nth = [=](int n) { return lisp::nth(n, args); };
    auto eval_nth = [=](int n) { return lisp::eval(lisp::nth(n, args)); };

    PtrRef r1(args);
    switch (number)
    {
        case 0: {
            current_object->set_aistate(lnumber_value(eval_nth(0)));
            current_object->set_aistate_time(0);
            LObject *ai = figures[current_object->otype]->get_fun(OFUN_AI);
            if (!ai)
            {
                lbreak("hrump... call to go_state, and no ai function defined?\n"
                      "Are you calling from move function (not mover)?\n");
                exit(0);
            }
            return ((LSymbol *)ai)->EvalFunction(nullptr);
        }

        case 1: {
            GameObject *old_cur = current_object;
            current_object = (GameObject *)lpointer_value(lisp::eval(lisp::car(args)));
            LObject *ret = eval_block(lisp::cdr(args));
            current_object = old_cur;
            return ret;
        }

        case 2: {
            int whit;
            GameObject *o = (GameObject *)lpointer_value(lisp::eval(lisp::car(args)));
            GameObject *hit = current_object->bmove(whit, o ? o : current_object);
            if (hit)
                return lisp::make_ptr(hit);
            return whit ? nullptr : lisp::sym::true_;
        }

        case 3:
            return lisp::make_ptr(current_object);

        case 4:
            if (player_list->next)
                return lisp::make_ptr(g_current_level->attacker(current_object));
            return lisp::make_ptr(player_list->m_focus);

        case 5:
            return lisp::make_ptr(g_current_level->find_closest(current_object->m_pos.x,
                                                                  current_object->m_pos.y,
                                                                  lnumber_value(eval_nth(0)),
                                                                  current_object));

        case 6:
            return lisp::make_ptr(g_current_level->find_xclosest(current_object->m_pos.x,
                                                                   current_object->m_pos.y,
                                                                   lnumber_value(eval_nth(0)),
                                                                   current_object));

        case 7:
            return lisp::make_ptr(g_current_level->find_xrange(current_object->m_pos.x,
                                                                 current_object->m_pos.y,
                                                                 lnumber_value(eval_nth(0)),
                                                                 lnumber_value(eval_nth(1))));

        case 8:
        case 22: {
            int aitype = get_nth(3) ? lnumber_value(eval_nth(3)) : 0;
            GameObject *o = create(lnumber_value(eval_nth(0)),
                                   lnumber_value(eval_nth(1)),
                                   lnumber_value(eval_nth(2)),
                                   0, aitype);
            if (g_current_level)
            {
                if (number == 8)
                    g_current_level->add_object(o);
                else // 22
                    g_current_level->add_object_after(o, current_object);
            }
            return lisp::make_ptr(o);
        }

        case 9:
            return lisp::make_ptr(the_game->first_view->m_focus);

        case 10: {
            view *v = ((GameObject *)lpointer_value(eval_nth(0)))->m_controller->next;
            return v ? lisp::make_ptr(v->m_focus) : nullptr;
        }

        case 11:
            return lisp::make_ptr
                ((void *)current_object->get_object(lnumber_value(eval_nth(0))));

        case 12:
            return lisp::make_ptr
                ((void *)current_object->get_light(lnumber_value(eval_nth(0))));

        case 13: {
            GameObject *old_cur = current_object;
            LObject *ret = nullptr;
            for (int i = 0; i < old_cur->total_objects(); ++i)
            {
                current_object = old_cur->get_object(i);
                ret = eval_nth(0);
            }
            current_object = old_cur;
            return ret;
        }

        case 14: {
            int t = lnumber_value(eval_nth(0));
            ivec2 pos(lnumber_value(eval_nth(1)), lnumber_value(eval_nth(2)));
            int r2 = lnumber_value(eval_nth(3));
            int r3 = lnumber_value(eval_nth(4));
            ivec2 shift(lnumber_value(eval_nth(5)), lnumber_value(eval_nth(6)));
            return lisp::make_ptr(AddLightSource(t, pos, r2, r3, shift));
        }

        case 15:
//          return current_lev shit
            break;

        case 16:
        {
            LObject *f = figures[current_object->otype]->get_fun(OFUN_USER_FUN);
            if (!f) return NULL;
            return ((LSymbol *)f)->EvalFunction(args);
        } break;
        case 17:
        {
            long trials = lnumber_value(lisp::eval(lisp::car(args)));
            args = lisp::cdr(args);
            timer t;
            for (int x = 0; x < trials; x++)
            {
                LSpace::Tmp.Clear();
                lisp::eval(lisp::car(args));
            }
            return lisp::make_fp((long)(t.get() * (1 << 16)));
        } break;
        case 18:
        { return lisp::make_str(object_names[current_object->otype]); } break;
        case 19:
        { return current_object->float_tick(); } break;
        case 20:
        {
            long x1 = lnumber_value(lisp::eval(lisp::car(args))); args = lisp::cdr(args);
            long y1 = lnumber_value(lisp::eval(lisp::car(args))); args = lisp::cdr(args);
            long x2 = lnumber_value(lisp::eval(lisp::car(args))); args = lisp::cdr(args);
            long y2 = lnumber_value(lisp::eval(lisp::car(args))); args = lisp::cdr(args);

            LObject *list = lisp::eval(lisp::car(args));
            GameObject *find = g_current_level->find_object_in_area(current_object->m_pos.x,
                                                    current_object->m_pos.y,
                                                    x1, y1, x2, y2, list, current_object);
            return find ? lisp::make_ptr(find) : nullptr;
        } break;

        case 21:
        {
            long a1 = lnumber_value(lisp::eval(lisp::car(args))); args = lisp::cdr(args);
            long a2 = lnumber_value(lisp::eval(lisp::car(args))); args = lisp::cdr(args);

            LObject *list = lisp::eval(lisp::car(args));
            PtrRef r2(list);
            GameObject *find = g_current_level->find_object_in_angle(current_object->m_pos.x,
                                                        current_object->m_pos.y,
                                                        a1, a2, list, current_object);
            return find ? lisp::make_ptr(find) : nullptr;
        } break;
        case 23:         // def_character
        {
            LSymbol *sym = (LSymbol *)lisp::car(args);
            if (item_type(sym)!=L_SYMBOL)
            {
                lbreak("expecting first arg to def-character to be a symbol!\n");
                exit(0);
            }
            LSpace *sp = LSpace::Current;
            LSpace::Current = &LSpace::Perm;
            sym->SetNumber(total_objects);   // set the symbol value to the object number
            LSpace::Current = sp;
            if (!total_objects)
            {
                object_names = (char **)malloc(sizeof(char *)*(total_objects+1));
        figures = (CharacterType **)malloc(sizeof(CharacterType *)*(total_objects+1));
            }
            else
            {
                object_names = (char **)realloc(object_names, sizeof(char *)*(total_objects+1));
        figures = (CharacterType **)realloc(figures, sizeof(CharacterType *)*(total_objects+1));
            }

            object_names[total_objects] = strdup(lstring_value(sym->GetName()));
            figures[total_objects]=new CharacterType((LList *)lisp::cdr(args), sym);
            total_objects++;
            return lisp::make_number(total_objects-1);
        } break;
        case 24:
        {
            int32_t x1 = lnumber_value(lisp::eval(lisp::car(args))); args = lisp::cdr(args);
            int32_t y1 = lnumber_value(lisp::eval(lisp::car(args))); args = lisp::cdr(args);
            int32_t x2 = lnumber_value(lisp::eval(lisp::car(args))); args = lisp::cdr(args);
            int32_t y2 = lnumber_value(lisp::eval(lisp::car(args)));
            g_current_level->foreground_intersect(x1, y1, x2, y2);
            LList *ret = nullptr;
            push_onto_list(lisp::make_number(y2), ret);
            push_onto_list(lisp::make_number(x2), ret);
            return ret;
        } break;
        case 25:
        {
#ifdef __linux__
            return lisp::make_sym("LINUX");
#endif
#ifdef __sgi
            return lisp::make_sym("IRIX");
#endif
#ifdef __WIN32
            return lisp::make_sym("WIN32");
#endif
        } break;
        case 26:
        {
            return lisp::make_str(g_current_level->GetName().c_str());
        } break;
        case 27: return ant_ai(); break;
        case 28: return sensor_ai(); break;
        case 29: if (dev&EDIT_MODE) current_object->drawer(); break;
        case 30: return top_ai(); break;
        case 31: return laser_ufun(args); break;
        case 32: return top_ufun(args); break;
        case 33: return plaser_ufun(args); break;
        case 34: return player_rocket_ufun(args); break;
        case 35: return lsaber_ufun(args); break;
        case 36:
        {

            int32_t xm, ym, but;
            xm = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            ym = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            but = lnumber_value(lisp::car(args));
            return cop_mover(xm, ym, but);
        } break;
        case 37: return ladder_ai(); break;
        case 38:
        {
            GameObject *old_cur = current_object;
            current_object = current_object->get_object(0);
            LObject *ret = eval_block(args);
            current_object = old_cur;
            return ret;
        }  break;
        case 39:
        {
            if (current_object->total_objects()==0)
                return lisp::sym::true_;
            else if (current_object->get_object(0)->aistate())
                return lisp::sym::true_;
            else return NULL;
        } break;
        case 40: top_draw(); break;
        case 41: bottom_draw(); break;
        case 42: return mover_ai(); break;
        case 43: return sgun_ai();
        case 44:
        {
            char nm[50];
            last_savegame_name(nm);
            return lisp::make_str(nm);
        } break;
        case 45:
        {
            char nm[50];
            sprintf(nm, "save%04d.pcx", load_game(1, symbol_str("LOAD")));
//      get_savegame_name(nm);
            the_game->reset_keymap();
            return lisp::make_str(nm);
        } break;
        case 46:
        {
            return lisp::make_str(start_argv[lnumber_value(lisp::eval(lisp::car(args)))]);
        } break;
        case 47:
        {
            int xv, yv, b1, b2, b3;
            if (has_joystick)
                joy_status(b1, b2, b3, xv, yv);
            else b1 = b2 = b3 = xv = yv = 0;

            LList *ret = nullptr;
            PtrRef r2(ret);
            push_onto_list(lisp::make_number(b3), ret);
            push_onto_list(lisp::make_number(b2), ret);
            push_onto_list(lisp::make_number(b1), ret);
            push_onto_list(lisp::make_number(yv), ret);
            push_onto_list(lisp::make_number(xv), ret);
            return ret;
        } break;
        case 48:
        {
            LList *ret = nullptr;
            {
        PtrRef r2(ret);
        push_onto_list(lisp::make_number((last_demo_mbut&4)==4), ret);
        push_onto_list(lisp::make_number((last_demo_mbut&2)==2), ret);
        push_onto_list(lisp::make_number((last_demo_mbut&1)==1), ret);
        push_onto_list(lisp::make_number(last_demo_mpos.y), ret);
        push_onto_list(lisp::make_number(last_demo_mpos.x), ret);
            }
            return ret;
        } break;
        case 49:
        {
            ivec2 pos;
            pos.x = lnumber_value(lisp::eval(lisp::car(args))); args = lisp::cdr(args);
            pos.y = lnumber_value(lisp::eval(lisp::car(args))); args = lisp::cdr(args);
            pos = the_game->MouseToGame(pos);
            LList *ret = nullptr;
            {
                    PtrRef r2(ret);
                    push_onto_list(lisp::make_number(pos.y), ret);
                    push_onto_list(lisp::make_number(pos.x), ret);
            }
            return ret;
        } break;
        case 50:
        {
            ivec2 pos;
            pos.x = lnumber_value(lisp::eval(lisp::car(args))); args = lisp::cdr(args);
            pos.y = lnumber_value(lisp::eval(lisp::car(args))); args = lisp::cdr(args);
            pos = the_game->GameToMouse(pos, current_view);
            LList *ret = nullptr;
            {
                PtrRef r2(ret);
                push_onto_list(lisp::make_number(pos.y), ret);
                push_onto_list(lisp::make_number(pos.x), ret);
            }
            return ret;
        } break;
        case 51:   return lisp::make_ptr(wm->font()); break;
        case 52:
        {
            view *c = current_object->m_controller;
            if (!c)
                lbreak("object is not a player, cannot return name");
            else
                return lisp::make_str(c->name);
        } break;
        case 54:
        {
#if defined HAVE_UNISTD_H
            char cd[150];
            getcwd(cd, 100);
#else
            /* FIXME: maybe retrieve the PS3 account name etc.? */
            char const *cd = "Player";
#endif
            return lisp::make_str(cd);
        } break;
        case 55:
#if !defined __CELLOS_LV2__
            /* FIXME: this looks rather dangerous */
            system(lstring_value(lisp::eval(lisp::car(args))));
#endif
            break;
        case 56:
        {
            LObject *fn = lisp::eval(lisp::car(args)); args = lisp::cdr(args);
            char tmp[200];
            {
                PtrRef r2(fn);
                char *slash = lstring_value(lisp::eval(lisp::car(args)));
                char *filename = lstring_value(fn);

                char *s = filename, *tp;

                for (tp = tmp; *s; s++, tp++)
                {
                    if (*s=='/' || *s=='\\')
                    *tp=*slash;
                    else *tp=*s;
                }
                *tp = 0;
            }
            return lisp::make_str(tmp);
        } break;
        case 58:
        {
            char **files, **dirs;
            int tfiles, tdirs, i;

            get_directory(lstring_value(lisp::eval(lisp::car(args))), files, tfiles, dirs, tdirs);
            LList *fl = nullptr, *dl = nullptr, *rl = nullptr;
            {
                PtrRef r2(fl), r3(dl);

                for (i = tfiles-1; i>=0; i--) { push_onto_list(lisp::make_str(files[i]), fl); free(files[i]); }
                free(files);

                for (i = tdirs-1; i>=0; i--) { push_onto_list(lisp::make_str(dirs[i]), dl); free(dirs[i]); }
                free(dirs);

                push_onto_list(dl, rl);
                push_onto_list(fl, rl);
            }

            return rl;
        } break;
        case 60: return respawn_ai(); break;
        case 61: return score_draw();  break;
        case 62: return show_kills(); break;
        case 63:
        {
                long x;
                sscanf(lstring_value(lisp::eval(lisp::car(args))), "%lx", &x);
                return lisp::make_ptr((void *)(intptr_t)x);
        } break;
        case 64:
        {
            char name[256], name2[256];
            strcpy(name, lstring_value(lisp::eval(lisp::car(args))));  args = lisp::cdr(args);
            long first = lnumber_value(lisp::eval(lisp::car(args)));  args = lisp::cdr(args);
            long last = lnumber_value(lisp::eval(lisp::car(args)));
            LList *ret = nullptr;
            PtrRef r2(ret);

            if (last>=first)
            {
                for (long i = last; i >= first; --i)
                {
                    sprintf(name2, "%s%04ld.pcx", name, i);
                    push_onto_list(lisp::make_str(name2), ret);
                }
            } else
            {
                for (long i = last; i <= first; ++i)
                {
                    sprintf(name2, "%s%04ld.pcx", name, i);
                    push_onto_list(lisp::make_str(name2), ret);
                }
            }
            return ret;
        }
    }
    return NULL;
}

//extern bFILE *rcheck, *rcheck_lp;

// arguments have already been evaled..
long c_caller(long number, LObject *args)
{
    PtrRef r1(args);
    switch (number)
    {
        case 1:
        {
            return lol::abs(current_object->m_pos.x-g_current_level->attacker(current_object)->m_pos.x);
        } break;
        case 2:
        {
            return lol::abs(current_object->m_pos.y-g_current_level->attacker(current_object)->m_pos.y);
        } break;
        case 3:
        {
            if( !current_object->m_controller )
            {
                lbreak("object is not a player, cannot determine keypresses");
            }
            else
            {
                return current_object->m_controller->key_down(lnumber_value(lisp::car(args)));
            }
        } break;
        case 4:
        {
            return the_game->key_down(lnumber_value(lisp::car(args)));
        } break;
        case 5:
        {
            return g_current_level->attacker(current_object)->state;
        } break;
        case 6:
        {
            return current_object->aitype();
        } break;
        case 7:
        {
            if (!current_object->keep_ai_info())
                current_object->set_aistate(0);
            return current_object->aistate();
        } break;
        case 8:
        {
            int ns = lnumber_value(lisp::car(args));
            current_object->set_aistate_time(0);
            current_object->set_aistate(ns); return 1;
        } break;
        case 9:
        {
/*      if (rcheck_lp)
      {
    char str[100];
    sprintf(str, "\n\nTick %d, Rand_on %d\n", g_current_level->tick_counter(), rand_on);
    rcheck_lp->write(str, strlen(str)+1);
    current_print_file = rcheck_lp;
    print_trace_stack(6);
    current_print_file=NULL;
      }*/

            return rand(lnumber_value(lisp::car(args)));
        } break;
        case 10: return current_object->aistate_time(); break;
        case 11: return current_object->state; break;
        case 12:
        {
            if (g_current_level->attacker(current_object)->m_pos.x>current_object->m_pos.x)
                return 1;
            else
                return -1;
        } break;
        case 13:
        {
            return current_object->move(lnumber_value(lisp::car(args)), lnumber_value(lisp::cadr(args)), lnumber_value(lisp::caddr(args)));
        } break;
        case 14:
        {
            if (current_object->direction>0)
                return 1;
            else
                return -1;
        } break;
        case 15: return current_object->otype; break;
        case 16: return current_object->next_picture(); break;
        case 17: current_object->set_fade_dir(lnumber_value(lisp::car(args))); return 1; break;
        case 18:
        {
            int cx = lnumber_value(lisp::car(args));
            args = lisp::cdr(args);
            int cy = lnumber_value(lisp::car(args));
            args = lisp::cdr(args);
            int but = lnumber_value(lisp::car(args));
            return current_object->mover(cx, cy, but);
        } break;
        case 19: current_object->set_fade_count(lnumber_value(lisp::car(args))); return 1; break;
        case 20: return current_object->fade_count(); break;
        case 21: return current_object->fade_dir(); break;
        case 22:
        {
            int32_t x1, y1, x2, y2, xp1, yp1, xp2, yp2;
            g_current_level->attacker(current_object)->picture_space(x1, y1, x2, y2);
            current_object->picture_space(xp1, yp1, xp2, yp2);
            if (xp1>x2 || xp2<x1 || yp1>y2 || yp2<y1) return 0;
            else return 1;
        } break;
        case 23: current_object->add_power(lnumber_value(lisp::car(args))); break;
        case 24: current_object->add_hp(lnumber_value(lisp::car(args))); break;

        case 27:
        { current_object->drawer(); return 1; } break;
        case 28:
        { return (dev & EDIT_MODE); } break;
        case 29:
        { current_object->draw_above(current_view); return 1; } break;
        case 30: return current_object->m_pos.x; break;
        case 31: return current_object->m_pos.y; break;
        case 32:
        { int32_t v = lnumber_value(lisp::car(args));
            current_object->m_pos.x = v;
//      current_object->last_x = v;
            return 1;
        } break;
        case 33:
        { int32_t v = lnumber_value(lisp::car(args));
            current_object->m_pos.y = v;
//      current_object->last_y = v;
            return 1;
        } break;

        case 34: { return g_current_level->push_characters(current_object, lnumber_value(lisp::car(args)),
                                                lnumber_value(lisp::cadr(args)));
                    } break;

        case 37:
        {
            int32_t s = lnumber_value(lisp::car(args));
            current_object->set_state((character_state)s);
            return (s==current_object->state);
        } break;

        case 38: return g_current_level->attacker(current_object)->m_pos.x; break;
        case 39: return g_current_level->attacker(current_object)->m_pos.y; break;
        case 40: current_object->change_aitype(lnumber_value(lisp::car(args))); return 1; break;

        case 42: return current_object->m_vel.x; break;
        case 43: return current_object->m_vel.y; break;
        case 44: current_object->m_vel.x = lnumber_value(lisp::car(args)); return 1; break;
        case 45: current_object->m_vel.y = lnumber_value(lisp::car(args)); return 1; break;
        case 46: if (g_current_level->attacker(current_object)->m_pos.x>current_object->m_pos.x) return -1;
                            else return 1; break;
        case 47: return lnumber_value(lisp::car(args))&BLOCKED_LEFT; break;
        case 48: return lnumber_value(lisp::car(args))&BLOCKED_RIGHT; break;

        case 50: dev_cont->add_palette(args); break;
        case 51: write_PCX(main_screen, g_palette, lstring_value(lisp::car(args))); break;

        case 52: the_game->zoom = lnumber_value(lisp::car(args)); the_game->draw(); break;
        case 55: the_game->show_help(lstring_value(lisp::car(args))); break;

        case 56: return current_object->direction; break;
        case 57: current_object->direction = lnumber_value(lisp::car(args)); break;
        case 58:
        {
            int x1 = lnumber_value(lisp::car(args));
            if (!current_object->m_controller)
            { lbreak("set_freeze_time: object is not a focus\n"); }
            else current_object->m_controller->freeze_time = x1; return 1;
        } break;
        case 59: return menu(args, big_font); break;
        case 60:
        { Event ev; dev_cont->do_command(lstring_value(lisp::car(args)), ev); return 1; } break;
        case 61: the_game->set_state(lnumber_value(lisp::car(args))); break;

        case 62:
        {
            int x1 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int y1 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int x2 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int y2 = lnumber_value(lisp::car(args));
            scene_director.set_text_region(x1, y1, x2, y2);
        } break;
        case 63: scene_director.set_frame_speed(lnumber_value(lisp::car(args))); break;
        case 64: scene_director.set_scroll_speed(lnumber_value(lisp::car(args))); break;
        case 65: scene_director.set_pan_speed(lnumber_value(lisp::car(args))); break;
        case 66: scene_director.scroll_text(lstring_value(lisp::car(args))); break;
        case 67: scene_director.set_pan(lnumber_value(lisp::car(args)),
                                  lnumber_value(lisp::cadr(args)),
                                  lnumber_value(lisp::caddr(args))); break;
        case 68: scene_director.wait(lisp::car(args)); break;


        case 73: the_game->set_level(new Level(lnumber_value(lisp::car(args)),
                                                lnumber_value(lisp::cadr(args)),
                                                lstring_value(lisp::caddr(args)))); break;
        case 74:
        { if (g_current_level) delete g_current_level;
            g_current_level = new Level(100, 100, "new_level");
        } break;
        case 75:
        {
            int amount = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            GameObject *o = ((GameObject *)lpointer_value(lisp::car(args))); args = lisp::cdr(args);
            int xv = 0, yv = 0;
            if (args)
            {
        xv = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
        yv = lnumber_value(lisp::car(args));
            }
            o->do_damage(amount, current_object, current_object->m_pos.x, current_object->m_pos.y, xv, yv);
        } break;
        case 76: return current_object->hp(); break;
        case 77:
        {
            GameObject *o = (GameObject *)lpointer_value(lisp::car(args));
            if (!o->m_controller) printf("set shift: object is not a focus\n");
            else o->m_controller->m_shift.y = lnumber_value(lisp::cadr(args)); return 1;
        } break;
        case 78:
        {
            GameObject *o = (GameObject *)lpointer_value(lisp::car(args));
            if (!o->m_controller) printf("set shift: object is not a focus\n");
            else o->m_controller->m_shift.x = lnumber_value(lisp::cadr(args)); return 1;
        } break;
        case 79: current_object->set_gravity(lnumber_value(lisp::car(args))); return 1; break;
        case 80: return current_object->tick(); break;
        case 81: current_object->m_accel.x = lnumber_value(lisp::car(args)); return 1; break;
        case 82: current_object->m_accel.y = lnumber_value(lisp::car(args)); return 1; break;
        case 84: set_local_players(lnumber_value(lisp::car(args))); return 1; break;
        case 85: return total_local_players(); break;
        case 86: light_detail = lnumber_value(lisp::car(args)); return 1; break;
        case 87: return light_detail; break;
        case 88: morph_detail = lnumber_value(lisp::car(args)); return 1; break;
        case 89: return morph_detail; break;
        case 90: current_object->morph_into(lnumber_value(lisp::car(args)), NULL,
                                          lnumber_value(lisp::cadr(args)),
                                          lnumber_value(lisp::caddr(args))); return 1; break;
        case 91: current_object->add_object((GameObject *)lpointer_value(lisp::car(args))); return 1; break;
        case 92:
        {
            int32_t x1 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t y1 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t x2 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t y2 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t c = lnumber_value(lisp::car(args));
            ivec2 pos1 = the_game->GameToMouse(ivec2(x1, y1), current_view);
            ivec2 pos2 = the_game->GameToMouse(ivec2(x2, y2), current_view);
            main_screen->Line(pos1, pos2, c);
            return 1;
        } break;
        case 93: return wm->dark_color(); break;
        case 94: return wm->medium_color(); break;
        case 95: return wm->bright_color(); break;

        case 99: current_object->remove_object((GameObject *)lpointer_value(lisp::car(args))); return 1; break;
        case 100: current_object->add_light((LightSource *)lpointer_value(lisp::car(args))); return 1; break;
        case 101: current_object->remove_light((LightSource *)lpointer_value(lisp::car(args))); return 1; break;
        case 102: return current_object->total_objects(); break;
        case 103: return current_object->total_lights(); break;

        case 104:
        {
            LightSource *l = (LightSource *)lpointer_value(lisp::car(args));
            int32_t x = lnumber_value(lisp::cadr(args));
            if (x >= 1)
                l->m_inner_radius = x;
            l->CalcRange();
            return 1;
        } break;
        case 105:
        {
            LightSource *l = (LightSource *)lpointer_value(lisp::car(args));
            int32_t x = lnumber_value(lisp::cadr(args));
            if (x > l->m_inner_radius)
                l->m_outer_radius = x;
            l->CalcRange();
            return 1;
        } break;
        case 106:
        {
            LightSource *l = (LightSource *)lpointer_value(lisp::car(args));
            l->m_pos.x = lnumber_value(lisp::cadr(args));
            l->CalcRange();
            return 1;
        } break;
        case 107:
        {
            LightSource *l = (LightSource *)lpointer_value(lisp::car(args));
            l->m_pos.y = lnumber_value(lisp::cadr(args));
            l->CalcRange();
            return 1;
        } break;
        case 108:
        {
            LightSource *l = (LightSource *)lpointer_value(lisp::car(args));
            l->m_shift.x = lnumber_value(lisp::cadr(args));
            l->CalcRange();
            return 1;
        } break;
        case 109:
        {
            LightSource *l = (LightSource *)lpointer_value(lisp::car(args));
            l->m_shift.y = lnumber_value(lisp::cadr(args));
            l->CalcRange();
            return 1;
        } break;
        case 110: return ((LightSource *)lpointer_value(lisp::car(args)))->m_inner_radius; break;
        case 111: return ((LightSource *)lpointer_value(lisp::car(args)))->m_outer_radius; break;
        case 112: return ((LightSource *)lpointer_value(lisp::car(args)))->m_pos.x; break;
        case 113: return ((LightSource *)lpointer_value(lisp::car(args)))->m_pos.y; break;
        case 114: return ((LightSource *)lpointer_value(lisp::car(args)))->m_shift.x; break;
        case 115: return ((LightSource *)lpointer_value(lisp::car(args)))->m_shift.y; break;
        case 116: return current_object->m_accel.x; break;
        case 117: return current_object->m_accel.y; break;
        case 118: g_current_level->remove_light((LightSource *)lpointer_value(lisp::car(args))); break;
        case 119: current_object->set_fx(lnumber_value(lisp::car(args))); break;
        case 120: current_object->set_fy(lnumber_value(lisp::car(args))); break;
        case 121: current_object->set_fxvel(lnumber_value(lisp::car(args))); break;
        case 122: current_object->set_fyvel(lnumber_value(lisp::car(args))); break;
        case 123: current_object->set_fxacel(lnumber_value(lisp::car(args))); break;
        case 124: current_object->set_fyacel(lnumber_value(lisp::car(args))); break;
        case 125: return current_object->picture()->Size().x; break;
        case 126: return current_object->picture()->Size().y; break;
        case 127: { dprintf("trap\n"); } break;   // I use this to set gdb break points
        case 128: { return g_current_level->platform_push(current_object, lnumber_value(lisp::car(args)),
                                                lnumber_value(lisp::cadr(args)));
                                                } break;
        case 133:  // def_sound
        {
            LSymbol *sym=NULL;
            if (lisp::cdr(args))
            {
                sym = (LSymbol *)lisp::car(args);
                if (item_type(sym)!=L_SYMBOL)
                {
                    lbreak("expecting first arg to def-character to be a symbol!\n");
                    exit(0);
                }
                args = lisp::cdr(args);
            }

            LSpace *sp = LSpace::Current;
            LSpace::Current = &LSpace::Perm;
            int id = cache.reg(lstring_value(lisp::car(args)), NULL, SPEC_EXTERN_SFX, 1);
            if (sym)
                sym->SetNumber(id);    // set the symbol value to sfx id
            LSpace::Current = sp;
            return id;
        } break;
        case 134:  // play_sound
        {
            LObject *a = args;
            PtrRef r2(a);
            int id = lnumber_value(lisp::car(a));
            if (id<0) return 0;
            a = lisp::cdr(a);
            if (!a)
                cache.sfx(id)->play(127);
            else
            {
                int vol = lnumber_value(lisp::car(a)); a = lisp::cdr(a);
                if (a)
                {
                    int32_t x = lnumber_value(lisp::car(a)); a = lisp::cdr(a);
                    if (!a)
                    {
                        lisp::print(args);
                        lbreak("expecting y after x in play_sound\n");
                        exit(1);
                    }
                    int32_t y = lnumber_value(lisp::car(a));
                    the_game->play_sound(id, vol, x, y);
                } else cache.sfx(id)->play(vol);
            }

        } break;

        case 137: return defun_pseq(args); break;
        case 138:
        { int id = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t x = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t y = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t dir = lnumber_value(lisp::car(args));
            add_panim(id, x, y, dir);
        } break;
        case 142:
        {
            int32_t x = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            if (x<0 || x>=total_weapons)
            {
                lbreak("weapon out of range (%d)\n", x);
                exit(0);
            }
            return weapon_types[x];
        } break;
        case 143:
        {
            int32_t x = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t y = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t r = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t m = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            GameObject *o = (GameObject *)lpointer_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t mp = lnumber_value(lisp::car(args));
            g_current_level->hurt_radius(x, y, r, m, current_object, o, mp);
        } break;

        case 144:
        {
            view *v = current_object->m_controller;
            if (!v) dprintf("Can't add weapons for non-players\n");
            else
            {
                int32_t x = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
                int32_t y = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
                if (x<0 || x>=total_weapons)
                { lbreak("weapon out of range (%d)\n", x); exit(0); }
                v->add_ammo(x, y);
            }
        } break;
        case 145:
        {
            view *v = current_object->m_controller;
            if (!v) return 0;
            else return v->weapon_total(lnumber_value(lisp::car(args)));
        } break;
        case 146:
        {
            view *v = current_object->m_controller;
            if (!v) return 0; else return v->current_weapon;
        } break;
        case 147:
        {
            view *v = current_object->m_controller;
            if (!v) { lbreak("current_weapon_type: object cannot hold weapons\n");
                return 0; }
            else return v->current_weapon;
        } break;
        case 148: return lnumber_value(lisp::car(args))&BLOCKED_UP; break;
        case 149: return lnumber_value(lisp::car(args))&BLOCKED_DOWN; break;
        case 150:
        {
            view *v = current_object->m_controller;
            int x = lnumber_value(lisp::car(args));
            if (x<0 || x>=total_weapons)
            { lbreak("weapon out of range (%d)\n", x); exit(0); }
            if (v) v->give_weapon(x);
        } break;
        case 151:
        {
            int a = lnumber_value(lisp::car(args));
            if (a<0 || a>=TOTAL_ABILITIES)
            {
                lisp::print(args);
                lbreak("bad ability number for get_ability, should be 0..%d, not %d\n",
                        TOTAL_ABILITIES, a);
                exit(0);
            }
            return get_ability(current_object->otype, (ability)a);
        } break;
        case 152:
        {
            view *v = current_object->m_controller;
            if (!v) dprintf("Can't use reset_player on non-players\n");
            else
                v->reset_player();
        } break;
        case 153:
        {
            GameObject *o = (GameObject *)lpointer_value(lisp::car(args));
            int32_t x = o->m_pos.x-current_object->m_pos.x,
                y=-(o->m_pos.y-o->picture()->Size().y/2-(current_object->m_pos.y-(current_object->picture()->Size().y/2)));
            return lisp::atan2(y, x);
        } break;
        case 154:
        {
            int32_t ang = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t mag = lfixed_point_value(lisp::car(args));
            int32_t xvel = (lisp::cos(ang)>>8)*(mag>>8);
            int32_t yvel=-(lisp::sin(ang)>>8)*(mag>>8);
            current_object->m_vel = ivec2(xvel >> 16, yvel >> 16);
            current_object->set_fxvel((xvel&0xffff)>>8);
            current_object->set_fyvel((yvel&0xffff)>>8);
        } break;
        case 155:
        {
            int tframes = current_object->total_frames(), f;

            int32_t ang1 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            if (ang1<0) ang1 = (ang1%360)+360;
            else if (ang1>=360) ang1 = ang1%360;
            int32_t ang2 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            if (ang2<0) ang2 = (ang2%360)+360;
            else if (ang2>=360) ang2 = ang2%360;

            int32_t ang = (lnumber_value(lisp::car(args))+90/tframes)%360;
            if (ang1>ang2)
            {
                if (ang<ang1 && ang>ang2)
                    return 0;
                else if (ang>=ang1)
                    f = (ang-ang1)*tframes/(359-ang1+ang2+1);
                else
                    f = (359-ang1+ang)*tframes/(359-ang1+ang2+1);
            } else if (ang<ang1 || ang>ang2)
                return 0;
            else f = (ang-ang1)*tframes/(ang2-ang1+1);
            if (current_object->direction>0)
                current_object->current_frame = f;
            else
                current_object->current_frame = tframes-f-1;
            return 1;
        } break;
        case 156:
        {
            int x = current_object->current_frame;
            current_object->set_state((character_state)lnumber_value(lisp::car(args)));
            current_object->current_frame = x;
        } break;

        case 168: if (current_object->morph_status()) return 1; else return 0; break;
        case 169:
        {
            int32_t am = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            GameObject *from = (GameObject *)lpointer_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t hitx = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t hity = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t px = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t py = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            current_object->damage_fun(am, from, hitx, hity, px, py);
        } break;
        case 170: return current_object->gravity(); break;
        case 171:
        {
            view *v = current_object->m_controller;
            if (!v) dprintf("make_view_solid: object has no view\n");
            else
                v->draw_solid = lnumber_value(lisp::car(args));
        } break;
        case 172:
        {
            LObject *a = args;
            int r = lnumber_value(lisp::car(a)); a = lisp::cdr(a);
            int g = lnumber_value(lisp::car(a)); a = lisp::cdr(a);
            int b = lnumber_value(lisp::car(a));
            if (r<0 || b<0 || g<0 || r>255 || g>255 || b>255)
            {
                lisp::print(args);
                lbreak("color out of range (0..255) in color lookup\n");
                exit(0);
            }
            return color_table->Lookup(u8vec3(r >> 3, g >> 3, b >> 3));
        } break;
        case 173:
        {
            view *v = current_object->m_controller;
            if (!v) { lisp::print(args); printf("get_player_inputs: object has no view!\n"); }
            else return v->x_suggestion;
        } break;
        case 174:
        {
            view *v = current_object->m_controller;
            if (!v) { lisp::print(args); printf("get_player_inputs: object has no view!\n"); }
            else return v->y_suggestion;
        } break;
        case 175:
        {
            view *v = current_object->m_controller;
            if (!v) { lisp::print(args); printf("get_player_inputs: object has no view!\n"); }
            else return v->b1_suggestion;
        } break;
        case 176:
        {
            view *v = current_object->m_controller;
            if (!v) { lisp::print(args); printf("get_player_inputs: object has no view!\n"); }
            else return v->b2_suggestion;
        } break;
        case 177:
        {
            view *v = current_object->m_controller;
            if (!v) { lisp::print(args); printf("get_player_inputs: object has no view!\n"); }
            else return v->b3_suggestion;
        } break;
        case 178:
        {
            bg_xmul = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            bg_xdiv = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            bg_ymul = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            bg_ydiv = lnumber_value(lisp::car(args));
            if (bg_xdiv==0) { bg_xdiv = 1; lisp::print(args); dprintf("bg_set_scroll: cannot set xdiv to 0\n"); }
            if (bg_ydiv==0) { bg_ydiv = 1; lisp::print(args); dprintf("bg_set_scroll: cannot set ydiv to 0\n"); }
        } break;
        case 179:
        {
            view *v = lget_view(lisp::car(args), "set_ambient_light");       args = lisp::cdr(args);
            int32_t x = lnumber_value(lisp::car(args));
            if (x>=0 && x<64) v->ambient = x;
        } break;
        case 180: return lget_view(lisp::car(args), "ambient_light")->ambient; break;
        case 181:
        {
            int x = current_object->total_objects();
            GameObject *who = (GameObject *)lpointer_value(lisp::car(args));
            for (int i = 0; i<x; i++)
                if (current_object->get_object(i)==who) return 1;
            return 0;
        } break;
        case 182: current_object->change_type(lnumber_value(lisp::car(args))); break;
        case 184: return current_object->current_frame; break;

        case 185: return current_object->fx(); break;
        case 186: return current_object->fy(); break;
        case 187: return current_object->fxvel(); break;
        case 188: return current_object->fyvel(); break;
        case 189: return current_object->fxacel(); break;
        case 190: return current_object->fyacel(); break;
        case 191:
        {
//      char *fn = lstring_value(lisp::car(args)); args = lisp::cdr(args);
//      stat_bar = cache.reg_object(fn, lisp::car(args), SPEC_IMAGE, 1);
        } break;
        case 192:
        {
            int32_t x = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t y = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t type = lnumber_value(lisp::car(args));
            if (x<0 || y<0 || x>=g_current_level->foreground_width() || y>=g_current_level->foreground_width())
                lbreak("%d %d is out of range of fg map", x, y);
            else
                g_current_level->PutFg(ivec2(x, y), type);
        } break;
        case 193:
        {
            int32_t x = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t y = lnumber_value(lisp::car(args));
            if (x<0 || y<0 || x>=g_current_level->foreground_width() || y>=g_current_level->foreground_width())
                lbreak("%d %d is out of range of fg map", x, y);
            else return g_current_level->GetFg(ivec2(x, y));
        } break;
        case 194:
        {
            int32_t x = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t y = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t type = lnumber_value(lisp::car(args));
            if (x<0 || y<0 || x>=g_current_level->background_width() || y>=g_current_level->background_width())
                lbreak("%d %d is out of range of fg map", x, y);
            else
                g_current_level->PutBg(ivec2(x, y), type);
        } break;
        case 195:
        {
            int32_t x = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t y = lnumber_value(lisp::car(args));
            if (x<0 || y<0 || x>=g_current_level->background_width() || y>=g_current_level->background_width())
                lbreak("%d %d is out of range of fg map", x, y);
            else return g_current_level->GetBg(ivec2(x, y));
        } break;
        case 196: load_tiles(args); break;
        case 197:
        {
            bFILE *fp = open_file(lstring_value(lisp::car(args)), "rb");
            if (fp->open_failure())
            {
                delete fp;
                        lbreak("load_palette: could not open file %s for reading", lstring_value(lisp::car(args)));
                exit(1);
            } else
            {
                SpecDir sd(fp);
                SpecEntry *se = sd.find(SPEC_PALETTE);
                if (!se) lbreak("File %s has no palette!\n", lstring_value(lisp::car(args)));
                else
                {
                    if (g_palette)
                        delete g_palette;
                    g_palette = new Palette(se, fp);
                }
                delete fp;
            }
        } break;
        case 198:
        {
            bFILE *fp = open_file(lstring_value(lisp::car(args)), "rb");
            if (fp->open_failure())
            {
                delete fp;
                        lbreak("load_color_filter: could not open file %s for reading", lstring_value(lisp::car(args)));
                exit(1);
            } else
            {
                SpecDir sd(fp);
                SpecEntry *se = sd.find(SPEC_COLOR_TABLE);
                if (!se) lbreak("File %s has no color filter!", lstring_value(lisp::car(args)));
                else
                {
                    delete color_table;
                    color_table = new ColorFilter(se, fp);
                }
                delete fp;
            }
        } break;
        case 199:
        {
            current_start_type = lnumber_value(lisp::car(args));
            set_local_players(1);
        } break;
        case 200:
        {
            int32_t xv = lnumber_value(lisp::car(args));  args = lisp::cdr(args);
            int32_t yv = lnumber_value(lisp::car(args));  args = lisp::cdr(args);
            int top = 2;
            if (args)
                if (!lisp::car(args)) top = 0;

            int32_t oxv = xv, oyv = yv;
            current_object->try_move(current_object->m_pos.x, current_object->m_pos.y, xv, yv, 1|top);
            current_object->m_pos += ivec2(xv, yv);
            return (oxv==xv && oyv==yv);
        } break;
        case 201:
        {
            int32_t x = lnumber_value(lisp::car(args));
            return figures[current_object->otype]->get_sequence((character_state)x)->length();
        } break;
        case 202:
        {
            int32_t x1 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t y1 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t x2 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t y2 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            LObject *block_all = lisp::car(args);
            int32_t nx2 = x2, ny2 = y2;
            g_current_level->foreground_intersect(x1, y1, x2, y2);
            if (x2!=nx2 || y2!=ny2) return 0;

            if (block_all)
                g_current_level->all_boundary_setback(current_object, x1, y1, x2, y2);
            else
                g_current_level->boundary_setback(current_object, x1, y1, x2, y2);
            return (x2==nx2 && y2==ny2);

        } break;
        case 203:
        {
            char *fn = lstring_value(lisp::car(args)); args = lisp::cdr(args);
            char *name = lstring_value(lisp::car(args));
            big_font_pict = cache.reg(fn, name, SPEC_IMAGE, 1);
        } break;
        case 204:
        {
            char *fn = lstring_value(lisp::car(args)); args = lisp::cdr(args);
            char *name = lstring_value(lisp::car(args));
            small_font_pict = cache.reg(fn, name, SPEC_IMAGE, 1);
        } break;
        case 205:
        {
            char *fn = lstring_value(lisp::car(args)); args = lisp::cdr(args);
            char *name = lstring_value(lisp::car(args));
            console_font_pict = cache.reg(fn, name, SPEC_IMAGE, 1);
        } break;
        case 206:
        {
            int32_t x = lnumber_value(lisp::car(args));
            if (x<current_object->total_frames())
                current_object->current_frame = x;
            else
        lbreak("%d out of range for set_current_frame", x);
        } break;

        case 208:
        {
            current_object->draw_trans(lnumber_value(lisp::car(args)), lnumber_value(lisp::cadr(args)));
        } break;
        case 209:
        {
            current_object->draw_tint(lnumber_value(lisp::car(args)));
        } break;
        case 210:
        {
            current_object->draw_predator();
        } break;
        case 211:
        { return lget_view(lisp::car(args), "shift_down")->m_shift.y; } break;
        case 212:
        { return lget_view(lisp::car(args), "shift_right")->m_shift.x; } break;
        case 213:
        { view *v = lget_view(lisp::car(args), "set_no_scroll_range"); args = lisp::cdr(args);
            v->no_xleft = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            v->no_ytop = lnumber_value(lisp::car(args));  args = lisp::cdr(args);
            v->no_xright = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            v->no_ybottom = lnumber_value(lisp::car(args));
        } break;
        case 215:
        {
            char *fn = lstring_value(lisp::car(args)); args = lisp::cdr(args);
            char *name = lstring_value(lisp::car(args)); args = lisp::cdr(args);
            return cache.reg(fn, name, SPEC_IMAGE, 1);
        } break;
        case 216:
        {
            int32_t x1 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t y1 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t id = lnumber_value(lisp::car(args));
            main_screen->PutImage(cache.img(id), ivec2(x1, y1), 1);
        } break;
        case 217:
        {
            view *v = current_object->m_controller;
            if (!v) lbreak("object has no view: view_x1");
            else return v->m_aa.x;
        } break;
        case 218:
        {
            view *v = current_object->m_controller;
            if (!v) lbreak("object has no view: view_x1");
            else return v->m_aa.y;
        } break;
        case 219:
        {
            view *v = current_object->m_controller;
            if (!v) lbreak("object has no view: view_x1");
            else return v->m_bb.x;
        } break;
        case 220:
        {
            view *v = current_object->m_controller;
            if (!v) lbreak("object has no view: view_x1");
            else return v->m_bb.y;
        } break;
        case 221:
        {
            view *v = current_object->m_controller;
            if (!v) lbreak("object has no view: view_push_down");
            else v->m_lastpos.y-=lnumber_value(lisp::car(args));
        } break;
        case 222:
        {
            view *v = current_object->m_controller;
            if (!v) lbreak("object has no view: local_player");
            else return v->local_player();
        } break;
        case 223:
        {
            char *fn = lstring_value(lisp::car(args));
            g_current_level->save(fn, 1);
        } break;
        case 224:
        {
            current_object->set_hp(lnumber_value(lisp::car(args)));
        } break;
        case 225:
        {
            char fn[255];
            // If a save filename is requested, prepend the savegame directory.
            if( strncmp( lstring_value( lisp::car(args) ), "save", 4 ) == 0 )
            {
                sprintf( fn, "%s%s", get_save_filename_prefix(), lstring_value( lisp::car(args) ) );
            }
            else
            {
                strcpy( fn, lstring_value(lisp::car(args)) );
            }
            the_game->request_level_load(fn);
        } break;
        case 226:
        {
            strcpy(level_file, lstring_value(lisp::car(args)));
        } break;
        case 227:
        {
            return cache.reg(lstring_value(lisp::car(args)), "palette", SPEC_PALETTE, 1);
        } break;
        case 228:
        {
            ivec3 delta;
            delta.r = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            delta.g = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            delta.b = lnumber_value(lisp::car(args));

            Palette *p = g_palette->Copy();
            for (int i = 0; i < 256; i++)
                p->SetColor(i, (u8vec3)clamp((ivec3)p->GetColor(i) + delta, 0, 255));
            p->Load();
            delete p;
        } break;
        case 229:
        {
            view *v = current_object->m_controller;
            if (!v) lbreak("object has no view: local_player");
            else return v->player_number;
        } break;
        case 230:
        {
            view *v = current_object->m_controller;
            if (!v) lbreak("object has no view: local_player");
            else
            {
                int32_t x = lnumber_value(lisp::car(args));
                if (x<0 || x>=total_weapons)
                { lbreak("weapon out of range (%d)\n", x); exit(0); }
                v->current_weapon = x;
            }
        } break;
        case 231:
        {
            view *v = current_object->m_controller;
            if (!v) lbreak("object has no view: local_player");
            else return v->has_weapon(lnumber_value(lisp::car(args)));
        } break;
        case 232:
        {
            ambient_ramp+=lnumber_value(lisp::car(args));
        } break;

        case 233:
        { int x = 0; view *v = player_list; for (; v; v = v->next, x++); return x; } break;

        case 234:
        {
            int32_t x1 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t y1 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t x2 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t y2 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t c = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t s = lnumber_value(lisp::car(args));
            ivec2 pos1 = the_game->GameToMouse(ivec2(x1, y1), current_view);
            ivec2 pos2 = the_game->GameToMouse(ivec2(x2, y2), current_view);
            ScatterLine(pos1, pos2, c, s);
            return 1;

        } break;
        case 235:
        { if (g_current_level) return g_current_level->tick_counter();
            else return 0; } break;
        case 236:
        {
            return current_object->m_controller!=NULL;
        } break;
        case 237:
        {
            (void)lnumber_value(lisp::car(args)); return 1;
        } break;
        case 238:
        {
            return current_object->total_frames();
        } break;
        case 239:
        { g_current_level->to_front(current_object); } break;
        case 240:
        { g_current_level->to_back(current_object); } break;
        case 241:
        {
            view *v = current_object->m_controller;
            if (!v) { lisp::print(args); printf("get_player_inputs: object has no view!\n"); }
            else return v->pointer_x;
        } break;
        case 242:
        {
            view *v = current_object->m_controller;
            if (!v) { lisp::print(args); printf("get_player_inputs: object has no view!\n"); }
            else return v->pointer_y;
        } break;
        case 243:
        {
            if (player_list->next || demo_man.current_state()!=demo_manager::NORMAL)
                return 0;
            else
                return (frame_panic>10);
        } break;
        case 244:
        {
            int32_t x1 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t y1 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t x2 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t y2 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t c1 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t c2 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t s = lnumber_value(lisp::car(args));
            ivec2 pos1 = the_game->GameToMouse(ivec2(x1, y1), current_view);
            ivec2 pos2 = the_game->GameToMouse(ivec2(x2, y2), current_view);
            AScatterLine(pos1, pos2, c1, c2, s);
            return 1;

        } break;
        case 245:
        {
            return 0x0; /* deprecated */
        } break;
        case 246:
        {
            (void)lnumber_value(lisp::car(args)); /* deprecated */
        } break;
        case 247:
        {
            int32_t cx1 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t cy1 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t cx2 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t cy2 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t c1 = lnumber_value(lisp::car(args)); args = lisp::cdr(args);
            main_screen->Bar(ivec2(cx1, cy1), ivec2(cx2, cy2), c1);
        } break;
        case 248:
        {
            return start_argc;
        } break;
        case 249:
        {
            if ((sound_avail&MUSIC_INITIALIZED))
            {
                char *fn = lstring_value(lisp::car(args));
                if (current_song)
                {
                    if (current_song->playing())
                    current_song->stop();
                    delete current_song;
                }
                current_song = new song(fn);
                current_song->play(music_volume);
                dprintf("Playing %s at volume %d\n", fn, music_volume);
            }
        } break;
        case 250:
        {
            if (current_song && current_song->playing())
                current_song->stop();
            delete current_song;
            current_song=NULL;
        } break;
        case 251: return current_object->targetable(); break;
        case 252: current_object->set_targetable( lisp::car(args)==NULL ? 0 : 1); break;
        case 253: show_stats(); break;
        case 254:
        {
            view *v = current_object->m_controller;
            if (!v) { lisp::print(args); printf("get_player_inputs: object has no view!\n"); }
            else return v->kills;
        } break;
        case 255:
        {
            view *v = current_object->m_controller;
            if (!v) { lisp::print(args); printf("get_player_inputs: object has no view!\n"); }
            else return v->tkills;
        } break;
        case 256:
        {
            view *v = current_object->m_controller;
            if (!v) { lisp::print(args); printf("get_player_inputs: object has no view!\n"); }
            else return v->secrets;
        } break;
        case 257:
        {
            view *v = current_object->m_controller;
            if (!v) { lisp::print(args); printf("get_player_inputs: object has no view!\n"); }
            else return v->tsecrets;
        } break;
        case 258:
        {
            view *v = current_object->m_controller;
            if (!v) { lisp::print(args); printf("get_player_inputs: object has no view!\n"); }
            else v->kills = lnumber_value(lisp::car(args));
        } break;
        case 259:
        {
            view *v = current_object->m_controller;
            if (!v) { lisp::print(args); printf("get_player_inputs: object has no view!\n"); }
            else v->tkills = lnumber_value(lisp::car(args));
        } break;
        case 260:
        {
            view *v = current_object->m_controller;
            if (!v) { lisp::print(args); printf("get_player_inputs: object has no view!\n"); }
            else v->secrets = lnumber_value(lisp::car(args));
        } break;
        case 261:
        {
            view *v = current_object->m_controller;
            if (!v) { lisp::print(args); printf("get_player_inputs: object has no view!\n"); }
            else v->tsecrets = lnumber_value(lisp::car(args));
        } break;
        case 262:
        {
            the_game->request_end();
        } break;
        case 263:
        {
            the_game->reset_keymap();
            return load_game(1, symbol_str("SAVE")); //get_save_spot(); shit
        } break;
        case 264:
        {
            printf("mem_report is deprecated\n");
        } break;
        case 265:
        {
            return ENGINE_MAJOR;
        } break;
        case 266:
        {
            return ENGINE_MINOR;
        } break;
        case 267:
        {
            current_object->draw_double_tint(lnumber_value(lisp::car(args)), lnumber_value(lisp::cadr(args)));
        } break;
        case 268:
        {
            return cache.img(lnumber_value(lisp::car(args)))->Size().x;
        } break;
        case 269:
        {
            return cache.img(lnumber_value(lisp::car(args)))->Size().y;
        } break;
        case 270:
        {
            return g_current_level->foreground_width();
        } break;
        case 271:
        {
            return g_current_level->foreground_height();
        } break;
        case 272:
        {
            return g_current_level->background_width();
        } break;
        case 273:
        {
            return g_current_level->background_height();
        } break;
        case 274:
        {
            return get_keycode(lstring_value(lisp::car(args)));
        }
        case 275:
        {
            int id = lnumber_value(lisp::car(args));  args = lisp::cdr(args);
            int x = lnumber_value(lisp::car(args));  args = lisp::cdr(args);
            int y = lnumber_value(lisp::car(args));
            c_target = id;
            if (main_screen)
                wm->SetMouseShape(cache.img(c_target)->copy(), ivec2(x, y));
        } break;
        case 276:
        {
            if (!main_net_cfg) return 0;
            return become_server(game_name);
        } break;
        case 277:
        {
            JCFont *fnt = (JCFont *)lpointer_value(lisp::car(args)); args = lisp::cdr(args);
            int32_t x = lnumber_value(lisp::car(args));       args = lisp::cdr(args);
            int32_t y = lnumber_value(lisp::car(args));       args = lisp::cdr(args);
            char *st = lstring_value(lisp::car(args));     args = lisp::cdr(args);
            int color=-1;
            if (args)
                color = lnumber_value(lisp::car(args));
            fnt->PutString(main_screen, ivec2(x, y), st, color);
        } break;
        case 278: return ((JCFont *)lpointer_value(lisp::car(args)))->Size().x; break;
        case 279: return ((JCFont *)lpointer_value(lisp::car(args)))->Size().y; break;
        case 280: if (chat) chat->put_all(lstring_value(lisp::car(args))); break;
        case 281:
        {
            view *v = current_object->m_controller;
            if (!v) { lbreak("get_player_name: object has no view!\n"); }
            else strcpy(v->name, lstring_value(lisp::car(args)));
        } break;
        case 282:
        {
            int32_t x1 = lnumber_value(lisp::car(args));   args = lisp::cdr(args);
            int32_t y1 = lnumber_value(lisp::car(args));   args = lisp::cdr(args);
            int32_t x2 = lnumber_value(lisp::car(args));   args = lisp::cdr(args);
            int32_t y2 = lnumber_value(lisp::car(args));   args = lisp::cdr(args);
            int32_t c = lnumber_value(lisp::car(args));
            main_screen->Bar(ivec2(x1, y1), ivec2(x2, y2), c);
        } break;
        case 283:
        {
            int32_t x1 = lnumber_value(lisp::car(args));   args = lisp::cdr(args);
            int32_t y1 = lnumber_value(lisp::car(args));   args = lisp::cdr(args);
            int32_t x2 = lnumber_value(lisp::car(args));   args = lisp::cdr(args);
            int32_t y2 = lnumber_value(lisp::car(args));   args = lisp::cdr(args);
            int32_t c = lnumber_value(lisp::car(args));
            main_screen->Rectangle(ivec2(x1, y1), ivec2(x2, y2), c);
        } break;
        case 284:
        {
            if (get_option(lstring_value(lisp::car(args))))
                return 1;
            else return 0;
        } break;
        case 288:
        {
            if (lisp::car(args)) the_game->set_delay(1); else the_game->set_delay(0);
        } break;
        case 289:
        {
            set_login(lstring_value(lisp::car(args)));
        } break;
        case 290:
        {
            chatting_enabled = 1;
        } break;
        case 291:
        {
            demo_start = 1;
        } break;
        case 292:
        {
            if (main_net_cfg && main_net_cfg->state==net_configuration::CLIENT)
                return 1;
        } break;
        case 293:
        {
            if (main_net_cfg && (main_net_cfg->state==net_configuration::CLIENT || main_net_cfg->state==net_configuration::SERVER))
            {
                view *v = player_list;
                for (; v; v = v->next)
                    if (v->kills>=main_net_cfg->kills)
                        return 1;
            } else return 0;
        } break;
        case 294:
        {
            view *v = player_list;
            for (; v; v = v->next)
            {
                v->tkills+=v->kills;

                v->kills = 0;
                GameObject *o = current_object;
                current_object = v->m_focus;

                ((LSymbol *)l_restart_player)->EvalFunction(NULL);
                v->reset_player();
                v->m_focus->set_aistate(0);
                current_object = o;
            }

        } break;
        case 295:
        {
            strncpy(game_name, lstring_value(lisp::car(args)), sizeof(game_name));
            game_name[sizeof(game_name)-1]=0;

        } break;
        case 296:
        {
            if (main_net_cfg)
                main_net_cfg->min_players = lnumber_value(lisp::car(args));
        } break;
        case 1001: // (set_object_tint)
            if(current_object->m_controller)
                current_object->m_controller->set_tint(lnumber_value(lisp::car(args)));
            else
                current_object->set_tint(lnumber_value(lisp::car(args)));
            break;
        case 1002: //(get_object_tint)
            if(current_object->m_controller)
                return current_object->m_controller->get_tint();
            else
                return current_object->get_tint();
            break;
        case 1003: //(set_object_team)
            if(current_object->m_controller)
                current_object->m_controller->set_team(lnumber_value(lisp::car(args)));
            else
                current_object->set_team(lnumber_value(lisp::car(args)));
            break;
        case 1004: //(get_object_team)
            if(current_object->m_controller)
                return current_object->m_controller->get_team();
            else
                return current_object->get_team();
            break;
        default:
            printf("Undefined c function %ld\n", number);
            return 0;
    }
    return 0;
}

