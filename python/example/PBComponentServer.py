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

el_time = 0
class MyService(ex.MediaPlayerService):
    def __init__(self):
        pass

    def CallNowPlayingInfo(self, rpc_controller, request, done):
        global el_time
        print('CallNowPlayingInfo: ', request, ', sid: ', rpc_controller.sid)
        et = ex.ElapseTime()
        et.hour = 13
        et.minute = 59
        et.second = el_time
        el_time += 1
        rpc_controller.reply_handle.reply(et.SerializeToString())
        rpc_controller.reply_handle.destroy()

    def CallSearchMetaData(self, rpc_controller, request, done):
        global el_time
        if request is None:
            print('CallSearchMetaData: ', 'no request' ', sid: ', rpc_controller.sid)
        else:
            print('CallSearchMetaData: ', request, ', sid: ', rpc_controller.sid)
        et = ex.ElapseTime()
        et.hour = 13
        et.minute = 59
        et.second = el_time
        el_time += 1
        rpc_controller.reply_handle.reply(et.SerializeToString())
        rpc_controller.reply_handle.destroy()

    def CallNextSong(self, rpc_controller, request, done):
        print('CallNextSong: ', request, ', sid: ', rpc_controller.sid)
        rpc_controller.reply_handle.destroy()

fdbus.fdbusStart(os.getenv('FDB_CLIB_PATH'))
component = protobus.FdbusPBComponent("my component")
channel_list = []
nr_channels = len(sys.argv) - 1

server = MyService()
protobus.FdbusPBComponent.printService(server)

for i in range(nr_channels):
    name = sys.argv[i+1]
    channel = component.offerService(name, server, MyConnectionHandle(), 'Call')
    channel_list.append(channel)

while True:
    for i in range(nr_channels):
        channel = channel_list[i]
        control = protobus.FdbusRpcController()
        stub = ex.MediaPlayerService_Stub(channel)

        et = ex.ElapseTime()
        et.hour = 13
        et.minute = 59
        et.second = el_time
        el_time += 1
        stub.EmitElapseTime(None, et, None)

    time.sleep(1)

