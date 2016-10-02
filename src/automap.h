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

#include "imlib/window.h"

#include "level.h"

class automap
{
  AWindow *automap_window;
  Level *cur_lev;
  int tick,w,h;                // used to draw your position as a blinking spot
  long old_dx,old_dy;

public:
  automap(Level *l, int width, int height);
  void toggle_window();
  void handle_event(Event &ev);
  void draw();
  ~automap() { if (automap_window) toggle_window(); }
};

extern automap *current_automap;

