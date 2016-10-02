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

class AProperty;

class APropertyManager
{
public:
    APropertyManager();
    ~APropertyManager();

    void load(char const *filename);
    void save(char const *filename);

    int getd(char const *name, int def) { return (int)get(name, def); }
    int get(char const *name, int def);
    char const *get(char const *name, char const *def);

    void setd(char const *name, int def) { set(name,def); }
    void set(char const *name, double def);
    void set(char const *name, char const *def);

private:
    array<AProperty *> m_props;

    AProperty *find(char const *name);
};

