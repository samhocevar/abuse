/*
 *  Abuse — dark 2D side-scrolling platform game
 *  Copyright © 1995 Crack dot Com
 *  Copyright © 2005—2016 Sam Hocevar <sam@hocevar.net>
 *
 *  This software was released into the Public Domain. As with most public
 *  domain software, no warranty is made or implied by Crack dot Com, by
 *  Jonathan Clark, or by Sam Hocevar.
 */

#pragma once

#define DEFAULT_COMM_PORT 20202
#define DEFAULT_GAME_PORT 20203
#define MAX_JOINERS 32  // maximum clients that can join at the same time

#include "sock.h"

void undrv_cleanup();

extern net_socket *game_sock,*comm_sock;

