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

#ifndef __CMYMEDIAPLAYERSERVICE_HPP_
#define __CMYMEDIAPLAYERSERVICE_HPP_

#include FDB_IDL_EXAMPLE_H
#include <iostream>
#include <protobus/CFdbRpcController.h>
#include <common_base/CFdbMessage.h>

class CMyMediaPlayerService : public FDBPB_EXAMPLE::MediaPlayerService
{
public:
    CMyMediaPlayerService()
    {}
    
    void CallNowPlayingInfo(::google::protobuf::RpcController* controller,
                         const ::FDBPB_EXAMPLE::NowPlayingDetails* request,
                         ::FDBPB_EXAMPLE::ElapseTime* response,
                         ::google::protobuf::Closure* done)
    {
        std::cout << "CallNowPlayingInfo is called, "
                  << request->artist() << ", "
                  << request->album() << ", "
                  << request->genre() << ", "
                  << request->title() << ", "
                  << request->file_name() << std::endl;
        ::FDBPB_EXAMPLE::ElapseTime resp;
        resp.set_hour(5);
        resp.set_minute(23);
        resp.set_second(26);
        CFdbSvcController::reply(controller, resp);
    }
    void CallSearchMetaData(::google::protobuf::RpcController* controller,
                         const ::google::protobuf::Empty* request,
                         ::FDBPB_EXAMPLE::ElapseTime* response,
                         ::google::protobuf::Closure* done)
    {
         std::cout << "CallSearchMetaData is called" << std::endl;
         ::FDBPB_EXAMPLE::ElapseTime resp;
         resp.set_hour(6);
         resp.set_minute(42);
         resp.set_second(35);
         CFdbSvcController::reply(controller, resp);
    }
    void EmitElapseTime(::google::protobuf::RpcController* controller,
                         const ::FDBPB_EXAMPLE::ElapseTime* request,
                         ::google::protobuf::Empty* response,
                         ::google::protobuf::Closure* done)
    {
         std::cout << "EmitElapseTime is broadcasted, "
                   << request->hour() << ", "
                   << request->minute() << ", "
                   << request->second() << std::endl;
    }
     void CallNextSong(::google::protobuf::RpcController* controller,
                      const ::FDBPB_EXAMPLE::SongId* request,
                      ::google::protobuf::Empty* response,
                      ::google::protobuf::Closure* done) {
         std::cout << "CallNextSong is called, "
                   << request->id() << std::endl;
     }
};

#endif

