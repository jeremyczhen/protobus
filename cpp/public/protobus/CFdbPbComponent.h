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

#ifndef __CFDBPBCOMPONENT_H__
#define __CFDBPBCOMPONENT_H__

#include <common_base/CFdbAFComponent.h>
#include <map>
#include <string>
#include <mutex>

namespace google
{
    namespace protobuf
    {
        class Service;
        class RpcChannel;
        class Message;
    }
}

class CFdbPbChannel;

class CFdbPbComponent : public CFdbAFComponent
{
public:
    typedef std::map<std::string, std::string> tRpcTbl;
    CFdbPbComponent(const char *name, CBaseWorker *worker = 0)
        : CFdbAFComponent(name, worker)
    {
    }
    void addMethod(tRpcTbl &tbl, const char *method_name);
    void addEvent(tRpcTbl &tbl, const char *event_name, const char *topic);
    google::protobuf::RpcChannel *queryPbService(const char *bus_name,
                                                 google::protobuf::Service *pb_service,
                                                 CFdbBaseObject::tConnCallbackFn connect_callback = 0,
                                                 const tRpcTbl *tbl = 0);
    google::protobuf::RpcChannel *queryPbService(const char *bus_name,
                                                 google::protobuf::Service *pb_service,
                                                 CFdbBaseObject::tConnCallbackFn connect_callback,
                                                 const char *start_with);
    google::protobuf::RpcChannel *offerPbService(const char *bus_name,
                                                 google::protobuf::Service *pb_service,
                                                 CFdbBaseObject::tConnCallbackFn connect_callback = 0,
                                                 const tRpcTbl *tbl = 0);
    google::protobuf::RpcChannel *offerPbService(const char *bus_name,
                                                 google::protobuf::Service *pb_service,
                                                 CFdbBaseObject::tConnCallbackFn connect_callback,
                                                 const char *start_with);
    google::protobuf::RpcChannel *getClientChannel(const char *bus_name);
    google::protobuf::RpcChannel *getServerChannel(const char *bus_name);

    static bool isEmptyMessage(const google::protobuf::Message &msg);
private:
    typedef std::map<std::string, CFdbPbChannel*> tChannelTbl;

    tChannelTbl mClientChannelTbl;
    std::mutex mClientChannelLock;

    tChannelTbl mServerChannelTbl;
    std::mutex mServerChannelLock;

    void processMethodCall(CBaseJob::Ptr &msg_ref, CFdbBaseObject *obj, google::protobuf::Service *pb_service);
    google::protobuf::RpcChannel *queryPbService(const char *bus_name,
                                                 google::protobuf::Service *pb_service,
                                                 CFdbBaseObject::tConnCallbackFn &connect_callback,
                                                 const char *start_with,
                                                 const tRpcTbl *tbl);
    google::protobuf::RpcChannel *offerPbService(const char *bus_name,
                                                 google::protobuf::Service *pb_service,
                                                 CFdbBaseObject::tConnCallbackFn &connect_callback,
                                                 const char *start_with,
                                                 const tRpcTbl *tbl);
};

#endif
