"""
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
"""

import fdbus
import protobus
import os
import sys
import Example_pb2 as ex
import time

class MyConnectionHandle(fdbus.ConnectionClosure):
    def handleConnection(self, sid, is_online, first_or_last):
        print('Connection status is changed - sid: ', sid, ', online: ', is_online, ', f or l: ', first_or_last)

class MyService(ex.MediaPlayerService):
    def __init__(self):
        pass

    def EmitElapseTime(self, rpc_controller, request, done):
        print('EmitElapseTime: ', request, ', sid: ', rpc_controller.sid)

def invoke_done(reply):
    if not reply is None:
        print('Done! reply is: ', reply)

event_handle_table = [{'method' : 'EmitElapseTime', 'topic' : 'my_topic'}]
fdbus.fdbusStart(os.getenv('FDB_CLIB_PATH'))
component = protobus.FdbusPBComponent("my component")
channel_list = []
nr_channels = len(sys.argv) - 1

server = MyService()

for i in range(nr_channels):
    name = sys.argv[i+1]
    channel = component.queryService(name, server, MyConnectionHandle(),
                                     None, event_handle_table)
    channel_list.append(channel)

while True:
    req = ex.SongId()
    for i in range(nr_channels):
        channel = channel_list[i]
        control = protobus.FdbusRpcController()
        stub = ex.MediaPlayerService_Stub(channel)

        now_playing = ex.NowPlayingDetails()
        now_playing.artist = 'The Weeknd'
        now_playing.album = 'Blinding Lights - Single'
        now_playing.genre = 'Alternative R&B'
        now_playing.title = 'Blinding Lights'
        now_playing.file_name = 'file-1'
        now_playing.folder_name = 'folder-1'
        now_playing.elapse_time = 100
        stub.CallNowPlayingInfo(None, now_playing, invoke_done)

        control.EnableSync(True)
        ret = stub.CallSearchMetaData(control, None, None)
        if ret is None:
            print('Error when sync invoke!')
        else:
            print('Sync reply to CallSearchMetaData: ', ret)

    time.sleep(1)

