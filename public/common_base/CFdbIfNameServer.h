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

#ifndef __CFDINTERFACENAMESERVER_H__
#define __CFDINTERFACENAMESERVER_H__

#include <common_base/CFdbSimpleSerializer.h>
#include "CFdbIfMsgTokens.h"

namespace NFdbBase {
enum FdbNsMsgCode
{
    REQ_ALLOC_SERVICE_ADDRESS = 0,
    REQ_REGISTER_SERVICE = 1,
    REQ_UNREGISTER_SERVICE = 2,

    REQ_QUERY_SERVICE = 3,
    REQ_QUERY_SERVICE_INTER_MACHINE = 4,

    REQ_QUERY_HOST_LOCAL = 5,

    NTF_SERVICE_ONLINE = 6,
    NTF_SERVICE_ONLINE_INTER_MACHINE = 7,
    NTF_MORE_ADDRESS = 8,
    NTF_SERVICE_ONLINE_MONITOR = 9,
    NTF_SERVICE_ONLINE_MONITOR_INTER_MACHINE = 10,

    NTF_HOST_ONLINE_LOCAL = 11,
    NTF_HOST_INFO = 12,
};

enum FdbHsMsgCode
{
    REQ_REGISTER_HOST = 0,
    REQ_UNREGISTER_HOST = 1,
    REQ_QUERY_HOST = 2,
    REQ_HEARTBEAT_OK = 3,
    REQ_HOST_READY = 4,

    NTF_HOST_ONLINE = 5,
    NTF_HEART_BEAT = 6,
};

class FdbMsgAddressList : public IFdbParcelable
{
public:
    FdbMsgAddressList()
        : mOptions(0)
    {}
    std::string &service_name()
    {
        return mServiceName;
    }
    void set_service_name(const std::string &name)
    {
        mServiceName = name;
    }
    void set_service_name(const char *name)
    {
        mServiceName = name;
    }
    std::string &host_name()
    {
        return mHostName;
    }
    void set_host_name(const std::string &name)
    {
        mHostName = name;
    }
    void set_host_name(const char *name)
    {
        mHostName = name;
    }
    bool is_local() const
    {
        return mIsLocal;
    }
    void set_is_local(bool local)
    {
        mIsLocal = local;
    }
    CFdbParcelableArray<std::string> &address_list()
    {
        return mAddressList;
    }
    void add_address_list(const std::string &address)
    {
        mAddressList.Add(address);
    }
    void add_address_list(const char *address)
    {
        mAddressList.Add(address);
    }
    NFdbBase::FdbMsgTokens &token_list()
    {
        mOptions |= mMaskTokenList;
        return mTokenList;
    }
    bool has_token_list() const
    {
        return !!(mOptions & mMaskTokenList);
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mServiceName
                   << mHostName
                   << mIsLocal
                   << mAddressList
                   << mOptions;
        if (mOptions & mMaskTokenList)
        {
            serializer << mTokenList;
        }
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mServiceName
                     >> mHostName
                     >> mIsLocal
                     >> mAddressList
                     >> mOptions;
        if (mOptions & mMaskTokenList)
        {
            deserializer >> mTokenList;
        }
    }
private:
    std::string mServiceName;
    std::string mHostName;
    bool mIsLocal;
    CFdbParcelableArray<std::string> mAddressList;
    FdbMsgTokens mTokenList;
    uint8_t mOptions;
        static const uint8_t mMaskTokenList = 1 << 0;
};

class FdbMsgServerName : public IFdbParcelable
{
public:
    std::string &name()
    {
        return mName;
    }
    void set_name(const std::string &n)
    {
        mName = n;
    }
    void set_name(const char *n)
    {
        mName = n;
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mName;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mName;
    }
private:
    std::string mName;
};

class FdbMsgHostAddress : public IFdbParcelable
{
public:
    FdbMsgHostAddress()
        : mOptions(0)
    {}
    std::string &ip_address()
    {
        return mIpAddress;
    }
    void set_ip_address(const std::string &address)
    {
        mIpAddress = address;
    }
    void set_ip_address(const char *address)
    {
        mIpAddress = address;
    }
    std::string &ns_url()
    {
        return mNsUrl;
    }
    void set_ns_url(const std::string &url)
    {
        mNsUrl = url;
    }
    void set_ns_url(const char *url)
    {
        mNsUrl = url;
    }
    std::string &host_name()
    {
        return mHostName;
    }
    void set_host_name(const std::string &name)
    {
        mHostName = name;
    }
    void set_host_name(const char *name)
    {
        mHostName = name;
    }
    FdbMsgTokens &token_list()
    {
        mOptions |= mMaskTokenList;
        return mTokenList;
    }
    bool has_token_list() const
    {
        return !!(mOptions & mMaskTokenList);
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mIpAddress
                   << mNsUrl
                   << mHostName
                   << mOptions;
        if (mOptions & mMaskTokenList)
        {
            serializer << mTokenList;
        }
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mIpAddress
                     >> mNsUrl
                     >> mHostName
                     >> mOptions;
        if (mOptions & mMaskTokenList)
        {
            deserializer >> mTokenList;
        }
    }
private:
    std::string mIpAddress;
    std::string mNsUrl;
    std::string mHostName;
    FdbMsgTokens mTokenList;
    uint8_t mOptions;
        static const uint8_t mMaskTokenList = 1 << 0;
};

class FdbMsgHostRegisterAck : public IFdbParcelable
{
public:
    FdbMsgHostRegisterAck()
        : mOptions(0)
    {}
    NFdbBase::FdbMsgTokens &token_list()
    {
        mOptions |= mMaskTokenList;
        return mTokenList;
    }
    bool has_token_list() const
    {
        return !!(mOptions & mMaskTokenList);
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mOptions;
        if (mOptions & mMaskTokenList)
        {
            serializer << mTokenList;
        }
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mOptions;
        if (mOptions & mMaskTokenList)
        {
            deserializer >> mTokenList;
        }
    }
private:
    FdbMsgTokens mTokenList;
    uint8_t mOptions;
        static const uint8_t mMaskTokenList = 1 << 0;
};

class FdbMsgHostInfo : public IFdbParcelable
{
public:
    std::string &name()
    {
        return mName;
    }
    void set_name(const std::string &n)
    {
        mName = n;
    }
    void set_name(const char *n)
    {
        mName = n;
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mName;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mName;
    }
private:
    std::string mName;
};

class FdbMsgHostAddressList : public IFdbParcelable
{
public:
    CFdbParcelableArray<FdbMsgHostAddress> &address_list()
    {
        return mAddressList;
    }
    FdbMsgHostAddress *add_address_list()
    {
        return mAddressList.Add();
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mAddressList;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mAddressList;
    }
private:
    CFdbParcelableArray<FdbMsgHostAddress> mAddressList;
};

class FdbMsgServiceInfo : public IFdbParcelable
{
public:
    FdbMsgAddressList &service_addr()
    {
        return mServiceAddr;
    }
    FdbMsgHostAddress &host_addr()
    {
        return mHostAddr;
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mServiceAddr
                   << mHostAddr;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mServiceAddr
                     >> mHostAddr;
    }
private:
   FdbMsgAddressList mServiceAddr;
   FdbMsgHostAddress mHostAddr;
};

class FdbMsgServiceTable : public IFdbParcelable
{
public:
    CFdbParcelableArray<FdbMsgServiceInfo> &service_tbl()
    {
        return mServiceTbl;
    }
    FdbMsgServiceInfo *add_service_tbl()
    {
        return mServiceTbl.Add();
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mServiceTbl;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mServiceTbl;
    }
private:
    CFdbParcelableArray<FdbMsgServiceInfo> mServiceTbl;
};

}

#endif

