/*
 * InputLeap -- mouse and keyboard sharing utility
 * Copyright (C) 2013-2016 Symless Ltd.
 * Copyright © 2022 Red Hat, Inc.
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 *
 * This package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "base/GlibEventQueue.h"
#include "base/Log.h"

void
GlibEventQueue::glib_thread()
{
    auto *maincontext = g_main_loop_get_context(m_gmainloop);

    LOG((CLOG_DEBUG "in GLIB thread"));
    // Run the Glib main loop in the separate thread
    while (g_main_loop_is_running(m_gmainloop)) {
        if (g_main_context_iteration(maincontext, true)) {
            LOG((CLOG_DEBUG "Something GLIB occured"));
        }
    }
    LOG((CLOG_DEBUG "GLIB mainloop stopped"));
}

GlibEventQueue::GlibEventQueue()
{
    m_gmainloop = g_main_loop_new(NULL, true);
    m_glibthread = new Thread([this](){ glib_thread(); });
    LOG((CLOG_DEBUG "GLIB thread created"));
}

GlibEventQueue::~GlibEventQueue()
{
    if (g_main_loop_is_running(m_gmainloop)) {
        LOG((CLOG_DEBUG "stopping GLIB mainloop"));
        g_main_loop_quit(m_gmainloop);
    }

    if (m_glibthread != nullptr) {
        m_glibthread->cancel();
        m_glibthread->wait();
        delete m_glibthread;

        LOG((CLOG_DEBUG "GLIB thread destroyed"));
        g_main_loop_unref(m_gmainloop);
        m_gmainloop = NULL;
    }
}
