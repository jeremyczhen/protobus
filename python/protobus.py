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
from google.protobuf import service as _pb_service

class FdbusRpcController(_pb_service.RpcController):
    def __init__(self):
        self.failed = False
        self.timeout = 0
        self.reason = None
        self.one_shot = True
        self.topic = None
        self.publish = False
        self.force_update = False
    def Failed(self):
        return self.failed
    def SetFailed(self, reason):
        self.failed = True
        self.reason = reason
    def SetTimeout(self, timeout):
        self.timeout = timeout
    def EnableSync(self, sync):
        self.one_shot = not sync;
    def SetTopic(self, topic):
        return self.topic
    def SetPublish(self, topic = None, force_update = False):
        self.publish = True
        self.topic = topic
        self.force_update = force_update

class FdbusRpcChannel(_pb_service.RpcChannel):
    @classmethod
    def createResponseMsg(cls, controller, msg_data, status, response_class):
        msg = None
        if status == 0:
            try:
                msg = response_class();
                msg.ParseFromString(msg_data)
            except Exception as e:
                print('Except in createResponseMsg: ', e)
        elif not controller is None:
            controller.SetFailed("Return status not 0")
        return msg

    class FdbReplyClosure(fdbus.ReplyClosure):
        def __init__(self, response_class, done):
            self.done = done
            self.resp_class = response_class
        def handleReply(self, sid, msg_code, msg_data, status):
            try:
                reply = FdbusRpcChannel.createResponseMsg(None, msg_data, status, self.resp_class)
                self.done(reply)
            except Exception as e:
                print('Except in handleReply: ', e)

    def __init__(self, endpoint):
        self.endpoint = endpoint

    def CallMethod(self, method_descriptor, rpc_controller, request, response_class, done):
        code = method_descriptor.index

        req_msg = None
        if not request is None:
            try:
                req_msg = request.SerializeToString()
            except Exception as e:
                print('Except in CallMethod: ', e)
                return None

        timeout = 0
        is_one_shot = True
        topic = None
        is_publish = False
        force_update = False
        if not rpc_controller is None:
            timeout = rpc_controller.timeout
            is_one_shot = rpc_controller.one_shot
            topic = rpc_controller.topic
            is_publish = rpc_controller.publish
            force_update = rpc_controller.force_update
        
        if isinstance(self.endpoint, fdbus.FdbusClient):
            if is_publish: # publish
                self.endpoint.publish(code, topic, req_msg, None, force_update)
            elif not done is None: # async call
                self.endpoint.invoke_callback(
                    FdbusRpcChannel.FdbReplyClosure(response_class, done), # TODO: possibly being removed!!!
                    code, req_msg, timeout)
            else: # sync call or one-shot (send)
                if not is_one_shot: # sync call
                    msg = self.endpoint.invoke_sync(code, req_msg, timeout)
                    if msg is None:
                        return None
                    else:
                        return FdbusRpcChannel.createResponseMsg(rpc_controller,
                                                                 msg['msg_data'],
                                                                 msg['status'],
                                                                 response_class)
                else: # send
                    self.endpoint.send(code, req_msg)
        elif isinstance(self.endpoint, fdbus.FdbusServer): # broadcast
            self.endpoint.broadcast(code, req_msg, topic)

        return None

class FdbusPBComponent(object):
    class PbClosure(fdbus.EventClosure, fdbus.MessageClosure):
        class Controller(_pb_service.RpcController):
            def __init__(self, sid, topic, reply_handle):
                self.sid = sid
                self.topic = topic;
                self.reply_handle = reply_handle
        def __init__(self, pb_server):
            self.pb_server = pb_server
        def process(self, sid, code, data, topic, reply_handle):
            svc_desc = self.pb_server.GetDescriptor()
            method = None
            for m in svc_desc.methods:
                if m.index == code:
                    method = m
                    break
            if method is None:
                return

            if data is None:
                request = None
            else:
                request = self.pb_server.GetRequestClass(method)()
                try:
                    request.ParseFromString(data)
                except Exception as e:
                    print('Except in process: ', e)

            try:
                self.pb_server.CallMethod(method,
                                          FdbusPBComponent.PbClosure.Controller(sid, topic, reply_handle),
                                          request, None)
            except Exception as e:
                print('Except in process: ', e)

        def handleEvent(self, sid, event_code, event_data, topic):
            self.process(sid, event_code, event_data, topic, None)

        def handleMessage(self, sid, msg_code, msg_data, reply_handle):
            self.process(sid, msg_code, msg_data, None, reply_handle)
        
    def __init__(self, name):
        self.client_channels = {}
        self.server_channels = {}
        self.afcomponent = fdbus.FdbusAfComponent(name)

    def getHandleTable(self, pb_server, prefix, handle_table):
        svc_desc = pb_server.GetDescriptor()
        ret_handle_table = []
        for m in svc_desc.methods:
            topic = None
            method = None
            if not handle_table is None:
                for event in handle_table:
                    if m.name == event['method']:
                        if 'topic' in event:
                            topic = event['topic']
                        method = m
            elif not prefix is None:
                if m.name.startswith(prefix):
                    method = m
            if method is None:
                continue

            ret_handle_table.append({'code': method.index, 'topic' : topic,
                                     'callback' : FdbusPBComponent.PbClosure(pb_server)})

        return ret_handle_table

    def queryService(self, bus_name, pb_server, connection_callback, prefix, event_table = None):
        event_handle_tbl = self.getHandleTable(pb_server, prefix, event_table)
        client = self.afcomponent.queryService(bus_name, event_handle_tbl, connection_callback)
        if client is None:
            return None

        if bus_name in self.client_channels:
            return self.client_channels[bus_name]
        channel = FdbusRpcChannel(client)
        self.client_channels[bus_name] = channel
        return channel

    def offerService(self, bus_name, pb_server, connection_callback, prefix, message_table = None):
        message_handle_tbl = self.getHandleTable(pb_server, prefix, message_table)
        server = self.afcomponent.offerService(bus_name, message_handle_tbl, connection_callback)
        if server is None:
            return None

        if bus_name in self.server_channels:
            return self.server_channels[bus_name]
        channel = FdbusRpcChannel(server)
        self.server_channels[bus_name] = channel
        return channel
        
