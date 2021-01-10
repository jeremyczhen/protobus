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

package ipc.fdbus;
import com.google.protobuf.RpcChannel;
import com.google.protobuf.BlockingRpcChannel;
import com.google.protobuf.Descriptors;
import com.google.protobuf.RpcController;
import com.google.protobuf.Message;
import com.google.protobuf.RpcCallback;
import ipc.fdbus.FdbusPBController;
import ipc.fdbus.FdbusClient;
import ipc.fdbus.FdbusServer;
import ipc.fdbus.FdbusProtoBuilder;

public class FdbusPBChannel implements RpcChannel, BlockingRpcChannel
{
    public FdbusPBChannel(Object endpoint)
    {
        mEndpoint = endpoint;
    }
    private Message handleResponse(RpcController controller, FdbusMessage msg, Message responsePrototype)
    {
        Message response = null;
        if ((msg == null) || (msg.returnValue() != Fdbus.FDB_ST_OK))
        {
            if (controller != null)
            {
                controller.setFailed("Failed!");
            }
        }
        else if (responsePrototype != null)
        {
            try {
                response = responsePrototype.getParserForType().
                                    parseFrom(msg.byteArray());
            } catch (Exception e) {
                System.out.println(e);
            }
        }
        return response;
    }
    public void callMethod(Descriptors.MethodDescriptor method,
                           RpcController controller,
                           Message request,
                           Message responsePrototype,
                           RpcCallback<Message> done)
    {
        FdbusPBController.Client ctrl = null;
        if ((controller != null) && (controller instanceof FdbusPBController.Client))
        {
            ctrl = (FdbusPBController.Client)controller;
        }
        int code = method.getIndex();
        if (mEndpoint instanceof FdbusClient)
        {
            FdbusClient client = (FdbusClient)mEndpoint;
            if ((ctrl != null) && ctrl.isPublish())
            {   // publish
                if (request == null)
                {
                    client.publish(code, null, null, false);
                }
                else
                {
                    FdbusProtoBuilder builder = new FdbusProtoBuilder(request);
                    client.publish(code, ctrl.topic(), builder, ctrl.forceUpdate());
                }
                return;
            }

            int timeout = 0;
            if (ctrl != null)
            {
                timeout = ctrl.timeout();
            }
            if (done != null)
            {   // async invoke by client
                if (request == null)
                {
                    client.invokeAsync(code, null,
                        new FdbusAppListener.Action()
                        {
                            public void handleMessage(FdbusMessage msg)
                            {
                                done.run(handleResponse(controller, msg, responsePrototype));
                            }
                        },
                        timeout);
                }
                else
                {
                    FdbusProtoBuilder builder = new FdbusProtoBuilder(request);
                    client.invokeAsync(code, builder,
                        new FdbusAppListener.Action()
                        {
                            public void handleMessage(FdbusMessage msg)
                            {
                                done.run(handleResponse(controller, msg, responsePrototype));
                            }
                        },
                        timeout);
                }
            }
            else
            {   // send by client
                System.out.println("jeremy 1");
                if (request == null)
                {
                    System.out.println("jeremy 2");
                    client.send(code, null);
                }
                else
                {
                    System.out.println("jeremy 3");
                    FdbusProtoBuilder builder = new FdbusProtoBuilder(request);
                    client.send(code, builder);
                }
            }
        }
        else
        {   // broadcast by server
            FdbusServer server = (FdbusServer)mEndpoint;
            String topic = null;
            if (ctrl != null)
            {
                topic = ctrl.topic();
            }
            if (request == null)
            {
                server.broadcast(code, topic);
            }
            else
            {
                FdbusProtoBuilder builder = new FdbusProtoBuilder(request);
                server.broadcast(code, topic, builder);
            }
        }
    }

    public Message callBlockingMethod(
        Descriptors.MethodDescriptor method,
        RpcController controller,
        Message request,
        Message responsePrototype)
    {
        FdbusClient client = null;
        if (mEndpoint instanceof FdbusClient)
        {
            client = (FdbusClient)mEndpoint;
        }
        else
        {
            return null;
        }
    
        FdbusPBController.Client ctrl = null;
        if ((controller != null) && (controller instanceof FdbusPBController.Client))
        {
            ctrl = (FdbusPBController.Client)controller;
        }

        int timeout = 0;
        if (ctrl != null)
        {
            timeout = ctrl.timeout();
        }

        int code = method.getIndex();
        FdbusMessage ret_msg;
        if (request == null)
        {
            ret_msg = client.invokeSync(code, null, timeout);
        }
        else
        {
            FdbusProtoBuilder builder = new FdbusProtoBuilder(request);
            ret_msg = client.invokeSync(code, builder, timeout);
        }

        return handleResponse(controller, ret_msg, responsePrototype);
    }

    private Object mEndpoint;
}
