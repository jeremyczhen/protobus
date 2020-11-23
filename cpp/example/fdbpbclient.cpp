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
#include <protobus/CFdbPbComponent.h>
#include "CMyMediaPlayerService.hpp"

static CMyMediaPlayerService g_fdb_pb_server;
static CBaseWorker g_fdb_media_player_worker;
static CFdbPbComponent g_fdb_media_player_component("media player", &g_fdb_media_player_worker);
static google::protobuf::RpcChannel *g_fdb_invoke_channel = 0;

static void my_callback1(FDBPB_EXAMPLE::ElapseTime *response)
{
    std::cout << "CallNowPlayingInfo is replied, "
              << response->hour() << ", "
              << response->minute() << ", "
              << response->second() << std::endl;
    delete response;
}

static void my_callback2(FDBPB_EXAMPLE::ElapseTime *response)
{
    std::cout << "CallSearchMetaData is replied, "
              << response->hour() << ", "
              << response->minute() << ", "
              << response->second() << std::endl;
    delete response;
}

class CInvokeTimer : public CBaseLoopTimer
{
public:
    CInvokeTimer()
        : CBaseLoopTimer(500, true)
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
        auto response = new FDBPB_EXAMPLE::ElapseTime();
        FDBPB_EXAMPLE::MediaPlayerService::Stub server(g_fdb_invoke_channel);
        server.CallNowPlayingInfo(0, &request, response, google::protobuf::NewCallback(my_callback1, response));
        }
        {
        auto response = new FDBPB_EXAMPLE::ElapseTime();
        FDBPB_EXAMPLE::MediaPlayerService::Stub server(g_fdb_invoke_channel);
        server.CallSearchMetaData(0, 0, response, google::protobuf::NewCallback(my_callback2, response));
        }
    }
};

CInvokeTimer g_fdb_invoke_timer;

int main(int argc, char **argv)
{
#ifdef __WIN32__
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0)
    {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }
#endif
    if (argc != 2)
    {
        std::cout << "Usage: fdbpbserver server_name" << std::endl;
        return -1;
    }
    FDB_CONTEXT->start();
    g_fdb_media_player_component.worker()->start();
    g_fdb_invoke_timer.attach(g_fdb_media_player_component.worker(), false);

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

