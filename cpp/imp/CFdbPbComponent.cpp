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

#include <protobus/CFdbPbComponent.h>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <common_base/CFdbProtoMsgBuilder.h>
#include <common_base/CBaseClient.h>
#include <common_base/CBaseServer.h>
#define FDB_LOG_TAG "FDB_PB"
#include <common_base/fdb_log_trace.h>
#include "CFdbPbChannel.h"

using namespace std::placeholders;

void CFdbPbComponent::processMethodCall(CBaseJob::Ptr &msg_ref, CFdbBaseObject *obj,
                                        google::protobuf::Service *pb_service)
{
    auto msg = castToMessage<CFdbMessage *>(msg_ref);
    auto descriptor = pb_service->GetDescriptor();
    auto method = descriptor->method(msg->code());
    if (!method)
    {
        return;
    }

    auto request = isEmptyMessage(pb_service->GetRequestPrototype(method)) ?
                    0 : pb_service->GetRequestPrototype(method).New();
    if (request)
    {
        CFdbProtoMsgParser parser(*request);
        if (!msg->deserialize(parser))
        {
            FDB_LOG_E("CFdbPbComponent: Unable to decode message %s!\n", method->full_name().c_str());
            delete request;
            return;
        }
    }

    auto response = isEmptyMessage(pb_service->GetResponsePrototype(method)) ?
                    0 : pb_service->GetResponsePrototype(method).New();

    //TODO: pass message reference through controller
    pb_service->CallMethod(method, 0, request, response, 0);
    if (response && msg->needReply(msg_ref))
    {
        CFdbProtoMsgBuilder builder(*response);
        msg->reply(msg_ref, builder);
    }
    if (request)
    {
        delete request;
    }
    if (response)
    {
        delete response;
    }
}

google::protobuf::RpcChannel *CFdbPbComponent::queryPbService(const char *bus_name,
                                                              google::protobuf::Service *pb_service,
                                                              CFdbBaseObject::tConnCallbackFn &connect_callback,
                                                              const char *start_with,
                                                              const tRpcTbl *tbl)
{
    if (!bus_name)
    {
        return 0;
    }
    
    CFdbEventDispatcher::CEvtHandleTbl evt_tbl;
    if (pb_service)
    {
        uint32_t start_size = start_with ? (uint32_t)strlen(start_with) : 0;
        auto descriptor = pb_service->GetDescriptor();
        const char *topic = 0;
        for (int i = 0; i < descriptor->method_count(); ++i)
        {
            auto method = descriptor->method(i);
            if (tbl)
            {
                auto it = tbl->find(method->name());
                if (it == tbl->end())
                {
                    continue;
                }
                topic = it->second.c_str();
            }
            else if (start_with && start_size)
            {
                if (method->name().compare(0, start_size, start_with))
                {
                    continue;
                }
            }

            tDispatcherCallbackFn callback = std::bind(&CFdbPbComponent::processMethodCall,
                                                       this, _1, _2, pb_service);
            addEvtHandle(evt_tbl, i, callback, topic);
        }
    }
    
    auto client = queryService(bus_name, evt_tbl, connect_callback);
    if (client)
    {
        CFdbPbChannel *channel = 0;
        std::lock_guard<std::mutex> _l(mClientChannelLock);
        auto it = mClientChannelTbl.find(bus_name);
        if (it == mClientChannelTbl.end())
        {
            channel = new CFdbPbChannel(this, client);
            mClientChannelTbl[bus_name] = channel;
        }
        else
        {
            channel = it->second;
        }
        return channel;
    }
    return 0;
}

google::protobuf::RpcChannel *CFdbPbComponent::queryPbService(const char *bus_name,
                                                              google::protobuf::Service *pb_service,
                                                              CFdbBaseObject::tConnCallbackFn connect_callback,
                                                              const tRpcTbl *tbl)
{
    return queryPbService(bus_name, pb_service, connect_callback, 0, tbl);
}

google::protobuf::RpcChannel *CFdbPbComponent::queryPbService(const char *bus_name,
                                                              google::protobuf::Service *pb_service,
                                                              CFdbBaseObject::tConnCallbackFn connect_callback,
                                                            const char *start_with)
{
    return queryPbService(bus_name, pb_service, connect_callback, start_with, 0);
}

google::protobuf::RpcChannel *CFdbPbComponent::getClientChannel(const char *bus_name)
{
    if (!bus_name)
    {
        return 0;
    }
    std::lock_guard<std::mutex> _l(mClientChannelLock);
    auto it = mClientChannelTbl.find(bus_name);
    return it == mClientChannelTbl.end() ? 0 : it->second;
}

google::protobuf::RpcChannel *CFdbPbComponent::getServerChannel(const char *bus_name)
{
    if (!bus_name)
    {
        return 0;
    }
    std::lock_guard<std::mutex> _l(mServerChannelLock);
    auto it = mServerChannelTbl.find(bus_name);
    return it == mServerChannelTbl.end() ? 0 : it->second;
}

google::protobuf::RpcChannel *CFdbPbComponent::offerPbService(const char *bus_name,
                                                              google::protobuf::Service *pb_service,
                                                              CFdbBaseObject::tConnCallbackFn &connect_callback,
                                                              const char *start_with,
                                                              const tRpcTbl *tbl)
{
    if (!pb_service || !bus_name)
    {
        return 0;
    }

    uint32_t start_size = start_with ? (uint32_t)strlen(start_with) : 0;
    CFdbMsgDispatcher::CMsgHandleTbl msg_tbl;
    auto descriptor = pb_service->GetDescriptor();
    for (int i = 0; i < descriptor->method_count(); ++i)
    {
        auto method = descriptor->method(i);
        if (tbl)
        {
            if (tbl->find(method->name()) == tbl->end())
            {
                continue;
            }
        }
        else if (start_with && start_size)
        {
            if (method->name().compare(0, start_size, start_with))
            {
                continue;
            }
        }

        tDispatcherCallbackFn callback = std::bind(&CFdbPbComponent::processMethodCall,
                                                   this, _1, _2, pb_service);
        addMsgHandle(msg_tbl, i, callback);
    }
    auto server = offerService(bus_name, msg_tbl, connect_callback);
    if (server)
    {
        CFdbPbChannel *channel = 0;
        std::lock_guard<std::mutex> _l(mServerChannelLock);
        auto it = mServerChannelTbl.find(bus_name);
        if (it == mServerChannelTbl.end())
        {
            channel = new CFdbPbChannel(this, server);
            mServerChannelTbl[bus_name] = channel;
        }
        else
        {
            channel = it->second;
        }
        return channel;
    }
    return 0;
}

google::protobuf::RpcChannel *CFdbPbComponent::offerPbService(const char *bus_name,
                                                              google::protobuf::Service *pb_service,
                                                              CFdbBaseObject::tConnCallbackFn connect_callback,
                                                              const tRpcTbl *tbl)
{
    return offerPbService(bus_name, pb_service, connect_callback, 0, tbl);
}

google::protobuf::RpcChannel *CFdbPbComponent::offerPbService(const char *bus_name,
                                                              google::protobuf::Service *pb_service,
                                                              CFdbBaseObject::tConnCallbackFn connect_callback,
                                                              const char *start_with)
{
  return offerPbService(bus_name, pb_service, connect_callback, start_with, 0);
}

 void CFdbPbComponent::addMethod(tRpcTbl &tbl, const char *method_name)
{
    tbl[method_name] = "";
}

void CFdbPbComponent::addEvent(tRpcTbl &tbl, const char *event_name, const char *topic)
{
    if (!topic)
    {
        topic = "";
    }
    tbl[event_name] = topic;
}

bool CFdbPbComponent::isEmptyMessage(const google::protobuf::Message &msg)
{
    return !msg.GetDescriptor()->full_name().compare("google.protobuf.Empty");
}

