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

#include "imlib/image.h"

class ASprite
{
public:
    ASprite(AImage *screen, AImage *visual, ivec2 pos);
    ~ASprite();

    void SetVisual(AImage *visual, int delete_old = 0);

    AImage *m_visual, *m_screen, *m_save;

private:
    ivec2 m_pos;
};

