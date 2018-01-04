/*
 *  Abuse — dark 2D side-scrolling platform game
 *  Copyright © 1995 Crack dot Com
 *  Copyright © 2005—2018 Sam Hocevar <sam@hocevar.net>
 *
 *  This software was released into the Public Domain. As with most public
 *  domain software, no warranty is made or implied by Crack dot Com, by
 *  Jonathan Clark, or by Sam Hocevar.
 */

#pragma once

#include "common.h"

#include "imlib/specs.h"

class SpecDirCache
{
    class FileNode
    {
        friend class SpecDirCache;

    public:
        FileNode(std::string const &filename, SpecDir *dir)
        {
            m_name = filename;
            m_sd = dir;
            m_next = m_left = m_right = nullptr;
        }

    private:
        FileNode *m_left, *m_right, *m_next;
        SpecDir *m_sd;
        std::string m_name;
        uint16_t m_size;
    }
    *m_root = nullptr, *m_list = nullptr;

public:
    inline SpecDirCache() {}
    ~SpecDirCache();

    SpecDir *GetSpecDir(std::string const &filename, bFILE *fp = nullptr);
    void Load(bFILE *fp);
    void Save(bFILE *fp);
    void Clear();

private:
    void ClearNode(FileNode *f);

    uint16_t m_size = 0;
};

extern SpecDirCache g_sd_cache;

