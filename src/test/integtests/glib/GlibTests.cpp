/*
 * InputLeap -- mouse and keyboard sharing utility
 * Copyright (C) 2012-2016 Symless Ltd.
 * Copyright (C) 2012 Nick Bolton
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

#define INPUTLEAP_TEST_ENV

#include "base/Log.h"
#include "test/global/gtest.h"
#include "test/global/TestGlibEventQueue.h"
#include <gio/gio.h>

class GlibTests : public ::testing::Test
{
public:
    GlibTests();
    virtual ~GlibTests();

    void glibAddTimeout(int ms);
    void glibAddDBusWatch();

public:
    bool m_glibTimeoutReceived;
    bool m_glibDBusReplyReceived;
    TestGlibEventQueue *m_events;
};

TEST_F(GlibTests, glibTimeout)
{
    glibAddTimeout(50);
    m_events->initQuitTimeout(5);
    m_events->loop();
    m_events->cleanupQuitTimeout();

    EXPECT_EQ(true, m_glibTimeoutReceived);
}

TEST_F(GlibTests, glibDBus)
{
    glibAddDBusWatch();
    m_events->initQuitTimeout(5);
    m_events->loop();
    m_events->cleanupQuitTimeout();

    EXPECT_EQ(true, m_glibDBusReplyReceived);
}

GlibTests::GlibTests() :
m_glibTimeoutReceived(false),
m_glibDBusReplyReceived(false)
{
    m_events = new TestGlibEventQueue();
}

GlibTests::~GlibTests()
{
    delete m_events;
}

extern "C" gboolean
glibTimeoutCallback(gpointer data)
{
    GlibTests *p = (GlibTests *) data;

    LOG((CLOG_DEBUG "Glib timeout triggered"));
    p->m_glibTimeoutReceived = true;
    p->m_events->raiseQuitEvent();

    return FALSE;
}

void
GlibTests::glibAddTimeout(int ms)
{
    g_timeout_add(ms, glibTimeoutCallback, this);
}

extern "C" void
dbus_name_appeared(GDBusConnection *connection,
                   const gchar *name,
                   const gchar *name_owner,
                   gpointer data)
{
    GlibTests *p = (GlibTests *) data;

    LOG((CLOG_DEBUG "DBus name found: '%s' (owner '%s')", name, name_owner));
    p->m_glibDBusReplyReceived = true;
    p->m_events->raiseQuitEvent();
}

extern "C" void
dbus_name_vanished(GDBusConnection *connection,
                   const gchar *name,
                   gpointer data)
{
    GlibTests *p = (GlibTests *) data;

    LOG((CLOG_DEBUG "DBus name missing: '%s'", name));
    p->m_glibDBusReplyReceived = true;
    p->m_events->raiseQuitEvent();
}

void
GlibTests::glibAddDBusWatch()
{
    guint dbus_name;

    dbus_name = g_bus_watch_name(G_BUS_TYPE_SESSION,
                                 "org.freedesktop.portal.Desktop",
                                 G_BUS_NAME_WATCHER_FLAGS_NONE,
                                 dbus_name_appeared,
                                 dbus_name_vanished,
                                 this,
                                 NULL);
}
