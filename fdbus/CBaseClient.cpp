/*
 * Copyright (C) 2015   Jeremy Chen jeremy_cz@yahoo.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <common_base/CBaseClient.h>
#include <common_base/CFdbContext.h>
#include <common_base/CBaseSocketFactory.h>
#include <common_base/CFdbSession.h>
#include <common_base/CFdbIfMessageHeader.h>
#include <utils/Log.h>

#define FDB_CLIENT_RECONNECT_WAIT_MS    1

CClientSocket::CClientSocket(CBaseClient *owner
                             , FdbSocketId_t skid
                             , CClientSocketImp *socket
                             , const char *host_name)
    : CFdbSessionContainer(skid, owner)
    , mSocket(socket)
    , mConnectedHost(host_name ? host_name : "")
{
}

CClientSocket::~CClientSocket()
{
    // so that onSessionDeleted() will not be called upon session destroy
    enableSessionDestroyHook(false);
    disconnect();
}

CFdbSession *CClientSocket::connect()
{
    CFdbSession *session = 0;
    auto sock_imp = mSocket->connect();
    if (sock_imp)
    {
        session = new CFdbSession(FDB_INVALID_ID, this, sock_imp);
    }
    return session;
}

void CClientSocket::disconnect()
{
    if (mSocket)
    {
        delete mSocket;
        mSocket = 0;
    }
}

void CClientSocket::getSocketInfo(CFdbSocketInfo &info)
{
    info.mAddress = &mSocket->getAddress();
}

void CClientSocket::onSessionDeleted(CFdbSession *session)
{
    auto client = dynamic_cast<CBaseClient *>(mOwner);
    
    if (mOwner->isReconnect() && session->internalError() && client)
    {
        session->internalError(false);
        auto url = mSocket->getAddress().mUrl;

        CFdbSessionContainer::onSessionDeleted(session);
        delete this;

        if (!client->requestServiceAddress())
        {
            if (FDB_CLIENT_RECONNECT_WAIT_MS)
            {
                sysdep_sleep(FDB_CLIENT_RECONNECT_WAIT_MS);
            }
            if (client->doConnect(url.c_str()))
            {
                LOG_E("CClientSocket: shutdown due to IO error but reconnected to %s@%s.\n", client->nsName().c_str(), url.c_str());
            }
            else
            {
                LOG_E("CClientSocket: shutdown due to IO error and fail to reconnect to %s@%s.\n", client->nsName().c_str(), url.c_str());
            }
        }
        else
        {
            LOG_E("CClientSocket: %s shutdown due to IO error but try to connect to name server.\n", client->nsName().c_str());
        }
    }
    else
    {
        CFdbSessionContainer::onSessionDeleted(session);
        delete this;
    }
}

CBaseClient::CBaseClient(const char *name, CBaseWorker *worker)
    : CBaseEndpoint(name, worker, FDB_OBJECT_ROLE_CLIENT)
    , mIsLocal(true)
{

}

CBaseClient::~CBaseClient()
{
}

class CConnectClientJob : public CMethodJob<CBaseClient>
{
public:
    CConnectClientJob(CBaseClient *client, M method, FdbSessionId_t &sid, const char *url)
        : CMethodJob<CBaseClient>(client, method, JOB_FORCE_RUN)
        , mSid(sid)
    {
        if (url)
        {
            mUrl = url;
        }
    }
    FdbSessionId_t &mSid;
    std::string mUrl;
};
FdbSessionId_t CBaseClient::connect(const char *url)
{
    FdbSessionId_t sid = FDB_INVALID_ID;
    CFdbContext::getInstance()->sendSyncEndeavor(
                new CConnectClientJob(this, &CBaseClient::cbConnect, sid, url), 0, true);
    return sid;
}

void CBaseClient::cbConnect(CBaseWorker *worker, CMethodJob<CBaseClient> *job, CBaseJob::Ptr &ref)
{
    auto the_job = dynamic_cast<CConnectClientJob *>(job);
    if (!the_job)
    {
        return;
    }

    std::string svc_url;
    const char *url;
    if (the_job->mUrl.empty())
    {
        getDefaultSvcUrl(svc_url);
        url = svc_url.c_str();
    }
    else
    {
        url = the_job->mUrl.c_str();
    }

    auto sk = doConnect(url);
    if (sk)
    {
        CFdbSession *session = sk->getDefaultSession();
        if (session)
        {
            the_job->mSid = session->sid();
        }
        else
        {
            the_job->mSid = FDB_INVALID_ID;
            LOG_E("CBaseClient: client is already connected but no session is found!\n");
            /* Just try to connect again */
        }
    }
}

CClientSocket *CBaseClient::doConnect(const char *url, const char *host_name)
{
    CFdbSocketAddr addr;
    EFdbSocketType skt_type;
    const char *server_name;

    if (url)
    {
        if (!CBaseSocketFactory::parseUrl(url, addr))
        {
            return 0;
        }
        skt_type = addr.mType;
        server_name = addr.mAddr.c_str();
    }
    else
    {
        skt_type = FDB_SOCKET_SVC;
        server_name = 0;
    }
    

    if (skt_type == FDB_SOCKET_SVC)
    {
        requestServiceAddress(server_name);
        return 0;
    }

    auto session_container = getSocketByUrl(url);
    if (session_container) /* If the address is already connected, do nothing */
    {
        return dynamic_cast<CClientSocket *>(session_container);
    }

    auto client_imp = CBaseSocketFactory::createClientSocket(addr);
    if (client_imp)
    {
        FdbSocketId_t skid = allocateEntityId();
        auto sk = new CClientSocket(this, skid, client_imp, host_name);
        addSocket(sk);

        auto session = sk->connect();
        if (session)
        {
            CFdbContext::getInstance()->registerSession(session);
            session->attach(CFdbContext::getInstance());
            if (addConnectedSession(sk, session))
            {
                return sk;
            }
            else
            {
                delete session;
                deleteSocket(skid);
                return 0;
            }
        }
        else
        {
            deleteSocket(skid);
        }
    }

    return 0;
}

class CDisconnectClientJob : public CMethodJob<CBaseClient>
{
public:
    CDisconnectClientJob(CBaseClient *client, M method, FdbSessionId_t &sid)
        : CMethodJob<CBaseClient>(client, method, JOB_FORCE_RUN)
        , mSid(sid)
    {}
    FdbSessionId_t mSid;
};

void CBaseClient::cbDisconnect(CBaseWorker *worker, CMethodJob<CBaseClient> *job, CBaseJob::Ptr &ref)
{
    auto the_job = dynamic_cast<CDisconnectClientJob *>(job);
    if (!the_job)
    {
        return;
    }
    
    doDisconnect(the_job->mSid);
    if (!fdbValidFdbId(the_job->mSid))
    {
        unregisterSelf();
        // From now on, there will be no jobs migrated to worker thread. Applying a
        // flush to worker thread to ensure no one refers to the object.
    }
}

void CBaseClient::doDisconnect(FdbSessionId_t sid)
{
    FdbSocketId_t skid = FDB_INVALID_ID;
    
    if (fdbValidFdbId(sid))
    {
        auto session = CFdbContext::getInstance()->getSession(sid);
        if (session)
        {
            skid = session->container()->skid();
        }
    }

    deleteSocket(skid);
}

void CBaseClient::disconnect(FdbSessionId_t sid)
{
    CFdbContext::getInstance()->sendSyncEndeavor(
                new CDisconnectClientJob(this, &CBaseClient::cbDisconnect, sid), 0, true);
}

void CBaseClient::updateSecurityLevel()
{
    if (!mTokens.empty())
    {
        NFdbBase::FdbAuthentication authen;
        authen.token_list().set_crypto_algorithm(NFdbBase::CRYPTO_NONE); 
        for (auto it = mTokens.begin(); it != mTokens.end(); ++it)
        {
            authen.token_list().add_tokens(*it);
        }
        CFdbParcelableBuilder builder(authen);
        sendSideband(FDB_SIDEBAND_AUTH, builder);
    }
}

bool CBaseClient::hostConnected(const char *host_name)
{
    if (!host_name)
    {
        return false;
    }
    auto &containers = getContainer();
    for (auto it = containers.begin(); it != containers.end(); ++it)
    {
        auto sessions = it->second;
        auto client_socket = dynamic_cast<CClientSocket *>(sessions);
        if (client_socket)
        {
            if (!client_socket->connectedHost().compare(host_name))
            {
                return true;
            }
        }
    }

    return false;
}


