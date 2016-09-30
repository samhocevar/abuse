/*
 *  Abuse - dark 2D side-scrolling platform game
 *  Copyright (c) 1995 Crack dot Com
 *  Copyright (c) 2005-2013 Sam Hocevar <sam@hocevar.net>
 *
 *  This software was released into the Public Domain. As with most public
 *  domain software, no warranty is made or implied by Crack dot Com, by
 *  Jonathan Clark, or by Sam Hocevar.
 */

#if HAVE_CONFIG_H
#   include "config.h"
#endif

#include <string.h>

#include "common.h"

#include "imlib/pmenu.h"

void pmenu::move(int new_x, int new_y)
{
  wm->move_window(bar,new_x,new_y);
}

pmenu::pmenu(int X, int Y, pmenu_item *first, AImage *screen)
{
  top=first;
  active=NULL;

  ivec2 caa, cbb;
  screen->GetClip(caa, cbb);
  if (caa.x<X) caa.x=X;
  int w = cbb.x - caa.x - AWindow::left_border() - AWindow::right_border();
  int h = AWindow::top_border() + AWindow::bottom_border();

  bar = wm->CreateWindow(ivec2(X, Y), ivec2(w, 0));
  bar->freeze();  // can't drag this window
  bar->m_surf->WidgetBar(ivec2(0, 0), ivec2(w - 1, h - 1),
                         wm->bright_color(), wm->medium_color(),
                         wm->dark_color());

  int total=0,tx,tw;
  pmenu_item *p=top;
  for (; p; p=p->next) total++;

  tw=w/(total+1);
  tx=tw/2;

  for (p=top; p; p=p->next,tx+=tw)
    p->draw_self(bar,itemx(p),1,itemw(p),1,p==active);
/*  }
  else
  {
    for (p=top; p; p=p->next,tx+=tw)
      p->draw(bar,itemx(p),1,itemw(p),1,p==active);
  }*/

}

pmenu_item::pmenu_item(int ID, char const *Name, char const *on_off_flag, int Hotkey, pmenu_item *Next)
{
  xp=-1;
  id=ID;
  hotkey=Hotkey;
  on_off=on_off_flag;
  if (Name)
    n = strdup(Name);
  else n=NULL;
  next=Next;
  sub=NULL;
}

pmenu_item::pmenu_item(char const *Name, psub_menu *Sub, pmenu_item *Next, int xpos)
{
  xp=xpos;
  id=0; hotkey=-1;
  next=Next;
  on_off=NULL;
  ASSERT(Name, "Sub menu cannot have a NULL name");
  n = strdup(Name);
  sub=Sub;
}

pmenu_item *pmenu_item::find_id(int search_id)
{
  if (id==search_id) return this;
  else if (sub) return sub->find_id(search_id);
  else return NULL;
}

pmenu_item *pmenu_item::find_key(int key)
{
  if (key==hotkey && hotkey!=-1) return this;
  else if (sub) return sub->find_key(key);
  else return NULL;
}

pmenu::~pmenu()
{
  while (top)
  {
    pmenu_item *p=top;
    top=top->next;
    delete p;
  }
  if (bar) wm->close_window(bar);
}

psub_menu::~psub_menu()
{
    if (m_win)
        wm->close_window(m_win);

    while (m_first)
    {
        pmenu_item *tmp = m_first;
        m_first = m_first->next;
        delete tmp;
    }
}

pmenu_item *psub_menu::find_id(int search_id)
{
    for (pmenu_item *f = m_first; f; f = f->next)
        if (pmenu_item *ret = f->find_id(search_id))
            return ret;

    return nullptr;
}

pmenu_item *psub_menu::find_key(int key)
{
    for (pmenu_item *f = m_first; f; f = f->next)
        if (pmenu_item *ret = f->find_key(key))
            return ret;

    return nullptr;
}

void psub_menu::hide(AWindow *parent, int x, int y)
{
    ivec2 caa, cbb, size = calc_size();
    main_screen->GetClip(caa, cbb);

    // FIXME: is this correct? it looks like it used to be incorrect
    // before the GetClip refactoring...
    if (size.x + x > cbb.x - 1)
        x = cbb.x - 1 - size.x;

    if (m_win)
    {
        if (m_active != -1)
        {
            ivec2 newsize = calc_size();
            item_num(m_active)->draw(m_win, x + 3, y + 3 + m_active * (wm->font()->Size().y + 1), newsize.x - 6, 0, 0);
        }
        wm->close_window(m_win);
        m_win = nullptr;
    }
}

ivec2 psub_menu::calc_size() const
{
    ivec2 ts = wm->font()->Size();
    ivec2 ret(0);
    for (pmenu_item *p = m_first; p; p = p->next)
    {
        if (p->name())
        {
            int l = strlen(p->name()) * ts.x + 8;
            if (p->on_off)
                l += ts.x * 4;
            if (l > ret.x)
                ret.x = l;
        }
        ret.y++;
    }
    ret.y = ret.y * (ts.y + 1) + 8;
    return ret;
}

void psub_menu::draw(AWindow *parent, int x, int y)
{
    if (m_win)
        wm->close_window(m_win);

    ivec2 caa, cbb, size = calc_size();
    int i = 0;
    main_screen->GetClip(caa, cbb);
    if (parent->m_pos.x + size.x + x >= cbb.x)
        x = cbb.x - 1 - size.x - parent->m_pos.x;
    if (size.y + y + parent->m_pos.y >= cbb.y)
    {
        if (parent->m_pos.y + parent->m_size.y + wm->font()->Size().y >= cbb.y)
            y = -size.y;
        else
            y = y - size.y + wm->font()->Size().y + 5;
    }

    m_win = wm->CreateWindow(parent->m_pos + ivec2(x, y),
             size - ivec2(AWindow::left_border() - AWindow::right_border(),
                          AWindow::top_border() - AWindow::bottom_border()));
    m_win->freeze();
    m_win->m_surf->WidgetBar(ivec2(0, 0), size - ivec2(1, 1),
                             wm->bright_color(), wm->medium_color(),
                             wm->dark_color());

    int has_flags = 0;
    pmenu_item *p = m_first;
    for (; p; p = p->next)
        if (p->on_off)
            has_flags = 1;
    x = has_flags ? 3 + wm->font()->Size().x : 3;
    y = 3;

    for (p = m_first; p; p = p->next, i++, y += wm->font()->Size().y + 1)
        p->draw(m_win, x, y, size.x - 6, 0, i == m_active);

}

void pmenu_item::draw_self(AWindow *parent, int x, int y, int w, int top, int active)
{
  int bx=x;
  if (on_off) bx=x-wm->font()->Size().x;

  if (!n)
  {
    int h=wm->font()->Size().y;
    parent->m_surf->WidgetBar(ivec2(x, y + h / 2 - 1),
                              ivec2(x + w - 1, y + h / 2), wm->dark_color(),
                              wm->medium_color(), wm->bright_color());
  } else
  {
    if (active)
    {
      if (xp!=-1)
        parent->m_surf->xor_bar(bx,y,x+w-1,y+wm->font()->Size().y+1,wm->dark_color());
      else
      {
    parent->m_surf->Bar(ivec2(bx, y),
                        ivec2(x + w - 1, y + wm->font()->Size().y + 1),
                        wm->dark_color());
    wm->font()->PutString(parent->m_surf, ivec2(x+1, y+1), n, wm->medium_color());
    if (on_off && *on_off) wm->font()->PutString(parent->m_surf, ivec2(bx+1, y+1), "*", wm->medium_color());
      }
    } else
    {
      if (xp!=-1)
        parent->m_surf->xor_bar(bx,y,x+w-1,y+wm->font()->Size().y+1,wm->dark_color());
      else
      {
    parent->m_surf->Bar(ivec2(bx, y),
                        ivec2(x + w - 1, y + wm->font()->Size().y + 1),
                        wm->medium_color());
    wm->font()->PutString(parent->m_surf, ivec2(x + 1, y + 1), n, wm->bright_color());
    if (on_off && *on_off) wm->font()->PutString(parent->m_surf, ivec2(bx + 1, y + 1), "*", wm->bright_color());
      }
    }
  }
}

void pmenu_item::draw(AWindow *parent, int x, int y, int w, int top,
              int active)
{
  if (n)
  {
    if (active)
    {
      draw_self(parent,x,y,w,top,active);
      if (sub)
      {
    if (top)
          sub->draw(parent,x,y+wm->font()->Size().y+2);
    else
      sub->draw(parent,x+w,y);
      }
    }
    else
    {
      if (sub)
      {
    if (top)
          sub->hide(parent,x,y+wm->font()->Size().y+2);
    else
      sub->hide(parent,x+w,y);
      }
      draw_self(parent,x,y,w,top,active);

    }

  } else draw_self(parent,x,y,w,top,active);
}

int pmenu::itemx(pmenu_item *p)
{
  if (p->xp!=-1) return p->xp;
  int w=bar->m_surf->Size().x;


  int total=0,tw,i=0,x=0;
  for (pmenu_item *pp=top; pp; pp=pp->next,i++)
  { if (pp==p) x=i;
    total++;
  }


  tw=w/(total+1);
  return tw/2+x*tw;
}


void pmenu::draw(AImage *screen, int top_only)
{

}


int psub_menu::handle_event(AWindow *parent, int x, int y, Event &ev)
{
    ivec2 size = calc_size();

    if (m_win)
    {
        x = m_win->m_pos.x;
        y = m_win->m_pos.y;
    }

    int has_flags = 0, dx = 3;
    for (pmenu_item *p = m_first; p; p = p->next)
        if (p->on_off)
            has_flags = 1;
    if (has_flags)
        dx += wm->font()->Size().x;

    int th = wm->font()->Size().y;
    if (ev.mouse_move.x >= x && ev.mouse_move.y >= y
         && ev.mouse_move.x < x + size.x && ev.mouse_move.y < y + size.y)
    {
        int new_active = (ev.mouse_move.y - y - 3) / (th + 1);
        if (item_num(new_active) == nullptr)
            new_active = -1;

        if (new_active != m_active)
        {
            if (m_active != -1)
                item_num(m_active)->draw(m_win, dx, 3 + m_active * (th + 1), size.x - 6, 0, 0);
            m_active = new_active;
            if (m_active != -1)
                item_num(m_active)->draw(m_win, dx, 3 + m_active * (th + 1), size.x - 6, 0, 1);
        }
        if (ev.type == EV_MOUSE_BUTTON)
        {
            if (m_active != -1)
                return item_num(m_active)->handle_event(m_win, dx, 3 + m_active * (th + 1), size.x - 6, 0, ev);
            return 0;
        }
        return 1;
    }
    else if (m_active != -1)
    {
        return item_num(m_active)->handle_event(m_win, m_win->m_pos.x + dx, m_win->m_pos.y + 3 + m_active * (th + 1), size.x - 6, 0, ev);
    }

    return 0;
}

int pmenu_item::handle_event(AWindow *parent, int x, int y, int w, int top,
                 Event &ev)
{
  x+=parent->m_pos.x;
  y+=parent->m_pos.y;
  if (ev.mouse_move.x>=x && ev.mouse_move.y>=y && ev.mouse_move.x<x+w &&
      ev.mouse_move.y<y+wm->font()->Size().y+2)
  {
    if (sub) return 1;
    else
    {
      if (ev.type==EV_MOUSE_BUTTON &&n)
        wm->Push(Event(id, (char *)this));
      return 1;
    }
  } else if (sub)
  {
    if (top)
      return sub->handle_event(parent,x,y+wm->font()->Size().y+2,ev);
    else return sub->handle_event(parent,x+w,y,ev);
  } else return 0;
}

pmenu_item *pmenu::inarea(int mx, int my, AImage *screen)
{
  mx-=bar->m_pos.x;
  my-=bar->m_pos.y;
  if (mx<0 || my<0 || mx>=bar->m_surf->Size().x || my>=bar->m_surf->Size().y) return NULL;
  else
  {
    for (pmenu_item *p=top; p; p=p->next)
    {
      if (!p->next) return p;
      else if (itemx(p->next)>mx) return p;
    }
    return NULL;
  }
}

int psub_menu::own_event(Event &ev)
{
    if (m_win && ev.window == m_win)
        return 1;
    for (pmenu_item *p = m_first; p; p = p->next)
        if (p->own_event(ev))
            return 1;
    return 0;
}

int pmenu_item::own_event(Event &ev)
{
  if (sub)
    return sub->own_event(ev);
  else return 0;
}

pmenu_item::~pmenu_item()
{ if (n) free(n); if (sub) delete sub;
}

int pmenu::handle_event(Event &ev, AImage *screen)
{
  if (!active && ev.window!=bar) return 0;
/*
    int yes=0;
    if (ev.window==bar) yes=1;    // event in top bar?
    else
    {
      for (pmenu_item *p=top; p && !yes; p=p->next)  // event in submenu?
      if (p->own_event(ev)) yes=1;
    }
    if (!yes) return 0;        // event is not for us...
  }*/

  switch (ev.type)
  {
    case EV_KEY :
    {
      for (pmenu_item *p=top; p; p=p->next)
      {
    pmenu_item *r=p->find_key(ev.key);
    if (r)
    {
      wm->Push(Event(r->id, (char *)r));
      return 1;
    }
      }
      return 0;
    } break;
    case EV_MOUSE_MOVE :
    {
      pmenu_item *new_selection=inarea(ev.mouse_move.x,ev.mouse_move.y,screen);
      if (!new_selection && active &&
      active->handle_event(bar,itemx(active),1,itemw(active),1,ev))
    return 1;
      else if (active!=new_selection)
      {
    if (active)
      active->draw(bar,itemx(active),1,itemw(active),1,0);
    active=new_selection;
    if (active)
      active->draw(bar,itemx(active),1,itemw(active),1,1);
      }
      if (active) return 1;
      else return 0;
    } break;
    case EV_MOUSE_BUTTON :
    {
      if (active)
      {
        if (active->handle_event(bar,itemx(active),1,itemw(active),1,ev))
    {
      active->draw(bar,itemx(active),1,itemw(active),1,0);
      active=NULL;
      return 1;
    } else return 0;
      }
      else return 0;
    } break;
  }
  return 0;
}


