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
    // Add an method to rpc table. The table will be given to offerPbService()
    // and the methods in rpc table will be registered. Only the registered
    // method will be invoked when assocated method in stub is called at client.
    //
    // @tbl - the method table holding methods to be registered
    // @method_name - name of method defined in proto file
    void addMethod(tRpcTbl &tbl, const char *method_name);
    // Add an event handle with associated topic (if needed) to rpc table. The
    // table will be given to queryPbService() and the event handles in rpc table
    // will be registered. The event handles in the table will be called when
    // associated event is broadcasted by server.
    //
    // @tbl - the event table holding event handles and topics
    // @event_name - name of event handle (also method indeed) to be registered
    //      Since event handle is not supported by protoc, event name is actually
    //      method name but it is registered by client to handle event broadcasted
    //      by server
    // @topic - the optional topic
    void addEvent(tRpcTbl &tbl, const char *event_name, const char *topic = 0);
    // The method is called at client side to register event handle and obtain
    // message channel required by stub to talk with server.
    //
    // @bus_name - name of server on FDBus
    // @pb_service - the generated protobuf service: it can be 0 if no event will
    //      will be handled
    // @connect_callback - callback to notify online/offline of server
    // @tbl - the event handle table containing event handles define by pb_service
    //      in proto file. If 0 all event handles will be registered
    google::protobuf::RpcChannel *queryPbService(const char *bus_name,
                                                 google::protobuf::Service *pb_service = 0,
                                                 CFdbBaseObject::tConnCallbackFn connect_callback = 0,
                                                 const tRpcTbl *tbl = 0);
    // the same as above but no event table is given; instead a prefix string
    // is provided to select the methods starting with 'prefix' as event
    // handles. For example, for the following service:
    // service MediaPlayerService {
    //      rpc CallNowPlayingInfo(NowPlayingDetails) returns (ElapseTime);
    //      rpc CallSearchMetaData(google.protobuf.Empty) returns (ElapseTime);
    //      rpc EmitElapseTime(ElapseTime) returns (google.protobuf.Empty);
    //      rpc CallNextSong(SongId) returns (google.protobuf.Empty);
    //  }
    // if 'prefix' is "Emit", the method 'EmitElapseTime' will be registered
    // as event handle.
    //
    // The method return with an rpc channel which will be given to
    // google::protobuf::Service::stub for subsequent calls upon server.
    google::protobuf::RpcChannel *queryPbService(const char *bus_name,
                                                 google::protobuf::Service *pb_service,
                                                 CFdbBaseObject::tConnCallbackFn connect_callback,
                                                 const char *prefix);
    // The method is called at server side to register method handles and obtain
    // the rpc channel for event broadcast.
    //
    // @bus_name - name of the server will own on FDBus
    // @pb_service - the generated protobuf service: it can be 0 if no method
    //      call will be handled
    // @connect_callback - callback to notify online/offline of client
    // @tbl - method table containing methods defined by proto file. If 0 all
    //      methods will be registered
    google::protobuf::RpcChannel *offerPbService(const char *bus_name,
                                                 google::protobuf::Service *pb_service = 0,
                                                 CFdbBaseObject::tConnCallbackFn connect_callback = 0,
                                                 const tRpcTbl *tbl = 0);
    // the same as above but no method table is given; instead a prefix string
    // is provided to select the methods starting with 'prefix' as method
    // handles. For example, for the following service:
    // service MediaPlayerService {
    //      rpc CallNowPlayingInfo(NowPlayingDetails) returns (ElapseTime);
    //      rpc CallSearchMetaData(google.protobuf.Empty) returns (ElapseTime);
    //      rpc EmitElapseTime(ElapseTime) returns (google.protobuf.Empty);
    //      rpc CallNextSong(SongId) returns (google.protobuf.Empty);
    //  }
    // if 'prefix' is "Call", the method 'CallNowPlayingInfo',
    // 'CallSearchMetaData' and 'CallNextSong' will be registered
    // as method handle.
    //
    // The method return with an rpc channel which will be given to
    // google::protobuf::Service::stub for subsequent event broadcast.
    google::protobuf::RpcChannel *offerPbService(const char *bus_name,
                                                 google::protobuf::Service *pb_service,
                                                 CFdbBaseObject::tConnCallbackFn connect_callback,
                                                 const char *prefix);
    // Obtain the rpc channel at client side to call method upon the server
    google::protobuf::RpcChannel *getClientChannel(const char *bus_name);
    // Obtain the rpc channel at server side to broadcast events to clients
    google::protobuf::RpcChannel *getServerChannel(const char *bus_name);
    // check if a message is google.protobuf.Empty
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
                                                 const char *prefix,
                                                 const tRpcTbl *tbl);
    google::protobuf::RpcChannel *offerPbService(const char *bus_name,
                                                 google::protobuf::Service *pb_service,
                                                 CFdbBaseObject::tConnCallbackFn &connect_callback,
                                                 const char *prefix,
                                                 const tRpcTbl *tbl);
};

#endif
