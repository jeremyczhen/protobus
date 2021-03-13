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

#include <iostream>
#define FDB_LOG_TAG "FDB_PB_TEST"
#include <common_base/fdbus.h>
#include <protobus/protobus.h>
#include "CMyMediaPlayerService.hpp"

static CMyMediaPlayerService g_fdb_pb_server;
static CBaseWorker g_fdb_media_player_worker;
static CFdbPbComponent g_fdb_media_player_component("media player", &g_fdb_media_player_worker);
static google::protobuf::RpcChannel *g_fdb_invoke_channel = 0;

static void my_callback(FDBPB_EXAMPLE::ElapseTime &response)
{
    std::cout << "Async to CallNowPlayingInfo is received, "
              << response.hour() << ", "
              << response.minute() << ", "
              << response.second() << std::endl;
}

class CMyCallback
{
public:
    CMyCallback(const char *data)
        : mData(data)
    {
    }
    void handleMsg(FDBPB_EXAMPLE::ElapseTime &response)
    {
        std::cout << "Async reply to CallSearchMetaData is received, "
                  << "data: " << mData << ", "
                  << response.hour() << ", "
                  << response.minute() << ", "
                  << response.second() << std::endl;
    }
private:
    std::string mData;
};

class CInvokeTimer : public CBaseLoopTimer
{
public:
    CInvokeTimer()
        : CBaseLoopTimer(500, true)
        , mCallback("hello, world")
    {}
protected:
    void run()
    {
        {
            FDBPB_EXAMPLE::NowPlayingDetails request;
            request.set_artist("Jeremy");
            request.set_album("Programmer");
            request.set_genre("70 Years");
            request.set_title("Good Luck");
            request.set_file_name("~home/jeremy/music/song.mp3");
            request.set_folder_name("~/home/jeremy");
            request.set_elapse_time(126);
            FDBPB_EXAMPLE::MediaPlayerService::Stub server(g_fdb_invoke_channel);
            // done != 0: async call
            server.CallNowPlayingInfo(0, &request, PBS_INVOKE_ASYNC,
                                      newPbsFunction<FDBPB_EXAMPLE::ElapseTime>(my_callback));
        }
        {
            FDBPB_EXAMPLE::MediaPlayerService::Stub server(g_fdb_invoke_channel);
            // done != 0: async call
            server.CallSearchMetaData(0, 0, PBS_INVOKE_ASYNC,
                      newPbsMethod<CMyCallback, FDBPB_EXAMPLE::ElapseTime>(&mCallback, &CMyCallback::handleMsg));
        }
        {
            FDBPB_EXAMPLE::NowPlayingDetails request;
            request.set_artist("Jeremy");
            request.set_album("Programmer");
            request.set_genre("70 Years");
            request.set_title("Good Luck");
            request.set_file_name("~home/jeremy/music/song.mp3");
            request.set_folder_name("~/home/jeremy");
            request.set_elapse_time(126);
            FDBPB_EXAMPLE::ElapseTime response;
            FDBPB_EXAMPLE::MediaPlayerService::Stub server(g_fdb_invoke_channel);
            // done == 0 && response != 0: sync call
            server.CallNowPlayingInfo(0, &request, &response, 0);
            std::cout << "Sync to CallNowPlayingInfo is received, "
                      << response.hour() << ", "
                      << response.minute() << ", "
                      << response.second() << std::endl;
        }
        {
            ::FDBPB_EXAMPLE::SongId request;
            request.set_id(123456);
            FDBPB_EXAMPLE::MediaPlayerService::Stub server(g_fdb_invoke_channel);
            // done == 0 && response == PBS_INVOKE_ASYNC: async call without reply (send)
            server.CallNextSong(0, &request, PBS_INVOKE_ASYNC, 0);
        }
        {
            ::FDBPB_EXAMPLE::SongId request;
            request.set_id(123456);
            FDBPB_EXAMPLE::MediaPlayerService::Stub server(g_fdb_invoke_channel);
            // done == 0 && response == PBS_INVOKE_SYNC: sync call without reply
            server.CallNextSong(0, &request, PBS_INVOKE_SYNC, 0);
        }
    }
private:
    CMyCallback mCallback;
};

CInvokeTimer g_fdb_invoke_timer;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: fdbpbserver server_name" << std::endl;
        return -1;
    }
    FDB_CONTEXT->start();
    g_fdb_media_player_component.worker()->start();
    g_fdb_invoke_timer.attach(g_fdb_media_player_component.worker(), false);
    CFdbPbComponent::printService(&g_fdb_pb_server);

    // import all methods starting with "Emit" and subscribe as event handle
    g_fdb_invoke_channel =  g_fdb_media_player_component.queryPbService(argv[1], &g_fdb_pb_server,
        [](CFdbBaseObject *obj, FdbSessionId_t sid, bool online, bool first_or_last){
            if (online && first_or_last)
            {
                g_fdb_invoke_timer.enable();
            }
            else if (!online && first_or_last)
            {
                g_fdb_invoke_timer.disable();
            }
        }, "Emit");

    /* convert main thread into worker */
    CBaseWorker background_worker;
    background_worker.start(FDB_WORKER_EXE_IN_PLACE);
    return 0;
}

