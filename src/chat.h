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

#include "console.h"

class chat_console : public console
{

  public :
  int chat_event(Event &ev) { if (!con_win) return 0; else return con_win==ev.window; }
  void draw_user(char *st);
  void put_all(char *st);
  void clear();
  chat_console(JCFont *font, int width, int height);

};

extern chat_console *chat;

