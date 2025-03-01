// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2008-2015 Hellground <http://hellground.net/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/** \file WorldSocketMgr.cpp
*  \ingroup u2w
*  \author Derex <derex101@gmail.com>
*/

#include "WorldSocketMgr.h"

#include <ace/ACE.h>
#include <ace/Log_Msg.h>
#include <ace/Reactor.h>
#include <ace/Reactor_Impl.h>
#include <ace/TP_Reactor.h>
#include <ace/Dev_Poll_Reactor.h>
#include <ace/Guard_T.h>
#include <ace/Atomic_Op.h>
#include <ace/os_include/arpa/os_inet.h>
#include <ace/os_include/netinet/os_tcp.h>
#include <ace/os_include/sys/os_types.h>
#include <ace/os_include/sys/os_socket.h>

#include <set>

#include "Log.h"
#include "Common.h"
#include "Config/Config.h"
#include "Database/DatabaseEnv.h"
#include "WorldSocket.h"

/**
* This is a helper class to WorldSocketMgr ,that manages
* network threads, and assigning connections from acceptor thread
* to other network threads
*/
class ReactorRunnable : protected ACE_Task_Base
{
    public:
        ReactorRunnable() :
            m_Reactor (0),
            m_Connections (0),
            m_ThreadId (-1)
        {
            ACE_Reactor_Impl* imp = 0;

            #if defined (ACE_HAS_EVENT_POLL) || defined (ACE_HAS_DEV_POLL)

            imp = new ACE_Dev_Poll_Reactor();

            imp->max_notify_iterations(128);
            imp->restart(1);

            #else

            imp = new ACE_TP_Reactor();
            imp->max_notify_iterations(128);

            #endif

            m_Reactor = new ACE_Reactor(imp, 1);
        }

        virtual ~ReactorRunnable()
        {
            Stop();
            Wait();

            if (m_Reactor)
                delete m_Reactor;
        }

        void Stop()
        {
            m_Reactor->end_reactor_event_loop();
        }

        int Start()
        {
            if (m_ThreadId != -1)
                return -1;

            return (m_ThreadId = activate());
        }

        void Wait() { ACE_Task_Base::wait(); }

        long Connections()
        {
            return static_cast<long> (m_Connections.value());
        }

        int AddSocket (WorldSocket* sock)
        {
            ACE_GUARD_RETURN (ACE_Thread_Mutex, Guard, m_NewSockets_Lock, -1);

            ++m_Connections;
            sock->AddReference();
            sock->reactor (m_Reactor);
            m_NewSockets.insert (sock);

            return 0;
        }

        ACE_Reactor* GetReactor()
        {
            return m_Reactor;
        }

    protected:
        void AddNewSockets()
        {
            ACE_GUARD (ACE_Thread_Mutex, Guard, m_NewSockets_Lock);

            if (m_NewSockets.empty())
                return;

            for (SocketSet::const_iterator i = m_NewSockets.begin(); i != m_NewSockets.end(); ++i)
            {
                WorldSocket* sock = (*i);

                if (sock->IsClosed())
                {
                    sock->RemoveReference();
                    --m_Connections;
                }
                else
                    m_Sockets.insert(sock);
            }

            m_NewSockets.clear();
        }

        virtual int svc()
        {
            DEBUG_LOG ("Network Thread Starting");

            GameDataDatabase.ThreadStart();

            ACE_ASSERT(m_Reactor);

            SocketSet::iterator i, t;

            while (!m_Reactor->reactor_event_loop_done())
            {
                // dont be too smart to move this outside the loop
                // the run_reactor_event_loop will modify interval
                ACE_Time_Value interval (0, 10000);

                if (m_Reactor->run_reactor_event_loop (interval) == -1)
                    break;

                AddNewSockets();

                for (i = m_Sockets.begin(); i != m_Sockets.end();)
                {
                    if ((*i)->Update() == -1)
                    {
                        t = i;
                        ++i;
                        (*t)->CloseSocket();
                        (*t)->RemoveReference();
                        --m_Connections;
                        m_Sockets.erase(t);
                    }
                    else
                        ++i;
                }
            }

            GameDataDatabase.ThreadEnd();

            DEBUG_LOG ("Network Thread Exitting");

            return 0;
        }

    private:
        typedef ACE_Atomic_Op<ACE_SYNCH_MUTEX, long> AtomicInt;
        typedef std::set<WorldSocket*> SocketSet;

        ACE_Reactor* m_Reactor;
        AtomicInt m_Connections;
        int m_ThreadId;

        SocketSet m_Sockets;

        SocketSet m_NewSockets;
        ACE_Thread_Mutex m_NewSockets_Lock;
};

WorldSocketMgr::WorldSocketMgr():
    m_NetThreads(0),
    m_NetThreadsCount(0),
    m_SockOutKBuff(-1),
    m_SockOutUBuff(65536),
    m_UseNoDelay(true),
    m_Acceptor(0)
{
}

WorldSocketMgr::~WorldSocketMgr()
{
    if (m_NetThreads)
        delete [] m_NetThreads;

    if(m_Acceptor)
        delete m_Acceptor;
}

int WorldSocketMgr::StartReactiveIO(ACE_UINT16 port, const char* address)
{
    m_UseNoDelay = sConfig.GetBoolDefault ("Network.TcpNodelay", true);

    int num_threads = sConfig.GetIntDefault ("Network.Threads", 1);

    if (num_threads <= 0)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Network.Threads is wrong in your config file");
        return -1;
    }

    m_NetThreadsCount = static_cast<size_t> (num_threads + 1);

    m_NetThreads = new ReactorRunnable[m_NetThreadsCount];

    sLog.outBasic ("Max allowed socket connections %d",ACE::max_handles ());

    // -1 means use default
    m_SockOutKBuff = sConfig.GetIntDefault("Network.OutKBuff", -1);

    m_SockOutUBuff = sConfig.GetIntDefault("Network.OutUBuff", 65536);
    m_SockInKBuff = sConfig.GetIntDefault("Network.InKBuff", 65536);

    if (m_SockOutUBuff <= 0)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Network.OutUBuff is wrong in your config file");
        return -1;
    }

    WorldSocket::Acceptor* acc = new WorldSocket::Acceptor;
    m_Acceptor = acc;

    ACE_INET_Addr listen_addr(port, address);

    if (acc->open (listen_addr, m_NetThreads[0].GetReactor(), ACE_NONBLOCK) == -1)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Failed to open acceptor, check if the port is free");
        return -1;
    }

    for (size_t i = 0; i < m_NetThreadsCount; ++i)
        m_NetThreads[i].Start();

    return 0;
}

int WorldSocketMgr::StartNetwork(ACE_UINT16 port, std::string& address)
{
    if (!sLog.IsOutDebug ())
        ACE_Log_Msg::instance()->priority_mask (LM_ERROR, ACE_Log_Msg::PROCESS);

    if (StartReactiveIO (port, address.c_str()) == -1)
        return -1;

    return 0;
}

void WorldSocketMgr::StopNetwork()
{
    if (m_Acceptor)
    {
        WorldSocket::Acceptor* acc = dynamic_cast<WorldSocket::Acceptor*>(m_Acceptor);

        if (acc)
            acc->close();
    }

    if (m_NetThreadsCount != 0)
    {
        for (size_t i = 0; i < m_NetThreadsCount; ++i)
            m_NetThreads[i].Stop();
    }

	// because SIGINT causes freeze
    //Wait();
}

void WorldSocketMgr::Wait()
{
    volatile size_t num_threads = m_NetThreadsCount; //debug

    if (m_NetThreadsCount != 0)
    {
        for (size_t i = 0; i < m_NetThreadsCount; ++i)
            m_NetThreads[i].Wait();
    }
}

int WorldSocketMgr::OnSocketOpen(WorldSocket* sock)
{
    // set some options here
    if (m_SockOutKBuff >= 0)
    {
        if (sock->peer().set_option(SOL_SOCKET, SO_SNDBUF, (void*)&m_SockOutKBuff, sizeof(int)) == -1 && errno != ENOTSUP)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: WorldSocketMgr::OnSocketOpen set_option SO_SNDBUF");
            return -1;
        }
    }

    static const int ndoption = 1;

    // Set TCP_NODELAY.
    if (m_UseNoDelay)
    {
        if (sock->peer().set_option(ACE_IPPROTO_TCP, TCP_NODELAY, (void*)&ndoption, sizeof (int)) == -1)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: WorldSocketMgr::OnSocketOpen: peer().set_option TCP_NODELAY errno = %s", ACE_OS::strerror(errno));
            return -1;
        }
    }

    sock->m_OutBufferSize = static_cast<size_t> (m_SockOutUBuff);
    sock->m_InBufferSize = static_cast<size_t> (m_SockInKBuff);

    // we skip the Acceptor Thread
    size_t min = 1;

    ACE_ASSERT(m_NetThreadsCount >= 1);

    for (size_t i = 1; i < m_NetThreadsCount; ++i)
        if (m_NetThreads[i].Connections() < m_NetThreads[min].Connections())
            min = i;

    return m_NetThreads[min].AddSocket (sock);
}

WorldSocketMgr* WorldSocketMgr::Instance()
{
    return ACE_Singleton<WorldSocketMgr, ACE_Thread_Mutex>::instance();
}
