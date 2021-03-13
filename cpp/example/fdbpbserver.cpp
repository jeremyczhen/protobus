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
static google::protobuf::RpcChannel *g_fdb_broadcast_channel = 0;
class CBroadcastTimer : public CBaseLoopTimer
{
public:
    CBroadcastTimer()
        : CBaseLoopTimer(500, true)
    {}
protected:
    void run()
    {
        static int32_t hour = 0;
        FDBPB_EXAMPLE::ElapseTime et;
        et.set_hour(hour++);
        et.set_minute(44);
        et.set_second(39);
        FDBPB_EXAMPLE::MediaPlayerService::Stub server(g_fdb_broadcast_channel);
        server.EmitElapseTime(0, &et, 0, 0);
    }
};

CBroadcastTimer g_fdb_broadcast_timer;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: fdbpbserver server_name" << std::endl;
        return -1;
    }
    FDB_CONTEXT->start();
    g_fdb_media_player_component.worker()->start();
    g_fdb_broadcast_timer.attach(g_fdb_media_player_component.worker(), false);
    CFdbPbComponent::printService(&g_fdb_pb_server);

    // import all methods starting with "Call" as message handle
    g_fdb_broadcast_channel =  g_fdb_media_player_component.offerPbService(argv[1], &g_fdb_pb_server,
        [](CFdbBaseObject *obj, FdbSessionId_t sid, bool online, bool first_or_last){
            if (online && first_or_last)
            {
                g_fdb_broadcast_timer.enable();
            }
            else if (!online && first_or_last)
            {
                g_fdb_broadcast_timer.disable();
            }
        }, "Call");

    /* convert main thread into worker */
    CBaseWorker background_worker;
    background_worker.start(FDB_WORKER_EXE_IN_PLACE);
    return 0;
}

