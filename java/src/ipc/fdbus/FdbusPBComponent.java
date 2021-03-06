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
import ipc.fdbus.FdbusAFComponent;
import ipc.fdbus.FdbusAppListener;
import ipc.fdbus.FdbusPBChannel;
import ipc.fdbus.FdbusClient;
import ipc.fdbus.FdbusServer;
import ipc.fdbus.FdbusMessage;
import ipc.fdbus.FdbusPBController;
import ipc.fdbus.protobus.Protobus;

import java.util.ArrayList;
import java.util.List;
import java.util.HashMap;
import ipc.fdbus.SubscribeItem;

import com.google.protobuf.RpcChannel;
import com.google.protobuf.Descriptors;
import com.google.protobuf.RpcController;
import com.google.protobuf.Message;
import com.google.protobuf.RpcCallback;
import com.google.protobuf.Service;

public class FdbusPBComponent
{
    private FdbusAFComponent mAFComponent;
    private HashMap<String, FdbusPBChannel> mClientChannels;
    private HashMap<String, FdbusPBChannel> mServerChannels;

    public FdbusPBComponent(String name)
    {
        mAFComponent = new FdbusAFComponent(name);
        mClientChannels = new HashMap<String, FdbusPBChannel>();
        mServerChannels = new HashMap<String, FdbusPBChannel>();
    }
    private ArrayList<SubscribeItem> getHandleTable(final Service service,
                                                    String prefix,
                                                    HashMap<String, String> event_table)
    {
        ArrayList<SubscribeItem> event_handle_table = new ArrayList<SubscribeItem>();
        Descriptors.ServiceDescriptor svc_desc = service.getDescriptorForType();
        List<Descriptors.MethodDescriptor> methods = svc_desc.getMethods();
        for (Descriptors.MethodDescriptor method : methods)
        {
            String topic = null;
            if (prefix != null)
            {
                if (!method.getName().startsWith(prefix))
                {
                    continue;
                }
            }
            else if (event_table != null)
            {
                if (event_table.containsKey(method.getName()))
                {
                    topic = event_table.get(method.getName());
                }
                else
                {
                    continue;
                }
            }

            event_handle_table.add(SubscribeItem.newAction(getMethodCode(method), topic,
                new FdbusAppListener.Action(){
                    public void handleMessage(FdbusMessage msg)
                    {
                        Descriptors.ServiceDescriptor svc_desc = service.getDescriptorForType();
                        Message request = null;
                        try {
                            request = service.getRequestPrototype(method).
                                                getParserForType().parseFrom(msg.byteArray());
                        } catch (Exception e) {
                            System.out.println(e);
                        }
                        try {
                            service.callMethod(method, new FdbusPBController.Server(msg), request, null);
                        } catch (Exception e) {
                            System.out.println(e);
                        }
                    }
                }));
        }
        return event_handle_table;
    }
    private FdbusPBChannel getChannel(String bus_name, Object endpoint, HashMap<String, FdbusPBChannel> channel_table)
    {
        if (channel_table.containsKey(bus_name))
        {
            return channel_table.get(bus_name);
        }
        else
        {
            FdbusPBChannel channel = new FdbusPBChannel(endpoint);
            channel_table.put(bus_name, channel);
            return channel;
        }
    }
    private FdbusPBChannel queryService(String bus_name,
                                        Service service,
                                        FdbusAppListener.Connection conn_callback,
                                        String prefix,
                                        HashMap<String, String> event_table)
    {
        ArrayList<SubscribeItem> event_handle_table = getHandleTable(service, prefix, event_table);
        FdbusClient client = mAFComponent.queryService(bus_name, event_handle_table, conn_callback);
        return getChannel(bus_name, client, mClientChannels);
    }
    public FdbusPBChannel queryService(String bus_name,
                                       Service service,
                                       FdbusAppListener.Connection conn_callback,
                                       String prefix)
    {
        return queryService(bus_name, service, conn_callback, prefix, null);
    }
    public FdbusPBChannel queryService(String bus_name,
                                       Service service,
                                       FdbusAppListener.Connection conn_callback,
                                       HashMap<String, String> event_table)
    {
        return queryService(bus_name, service, conn_callback, null, event_table);
    }
    private FdbusPBChannel offerService(String bus_name,
                                        Service service,
                                        FdbusAppListener.Connection conn_callback,
                                        String prefix,
                                        ArrayList<String> message_table)
    {
        HashMap<String, String> table = null;
        if (message_table != null)
        {
            table = new HashMap<String, String>();
            for (String method : message_table)
            {
                table.put(method, null);
            }
        }
        
        ArrayList<SubscribeItem> message_handle_table = getHandleTable(service, prefix, table);
        FdbusServer server = mAFComponent.offerService(bus_name, message_handle_table, conn_callback);
        return getChannel(bus_name, server, mServerChannels);
    }
    public FdbusPBChannel offerService(String bus_name,
                                       Service service,
                                       FdbusAppListener.Connection conn_callback,
                                       String prefix)
    {
        return offerService(bus_name, service, conn_callback, prefix, null);
    }
    public FdbusPBChannel offerService(String bus_name,
                                       Service service,
                                       FdbusAppListener.Connection conn_callback,
                                       ArrayList<String> message_table)
    {
        return offerService(bus_name, service, conn_callback, null, message_table);
    }
    public static void printService(Service service)
    {
        Descriptors.ServiceDescriptor svc_desc = service.getDescriptorForType();
        System.out.printf("Service %s\n    -Full Name: %s\n    -ID: %d\n    -Method Count: %d\n    -Interface Description: %s\n    -Method Table:\n",
                          svc_desc.getName(),
                          svc_desc.getFullName(),
                          svc_desc.getIndex(),
                          svc_desc.getMethods().size(),
                          svc_desc.getFile().getName());
        System.out.printf("    | %30s | %5s | %30s | %30s |\n",
                          "Method Name",
                          "ID",
                          "Input Name",
                          "Output Name");
        System.out.printf("    ------------------------------------------------------------------------------------------------------------\n");
        List<Descriptors.MethodDescriptor> methods = svc_desc.getMethods();
        for (Descriptors.MethodDescriptor method : methods)
        {
            System.out.printf("    | %30s | %5d | %30s | %30s |\n",
                              method.getName(),
                              getMethodCode(method),
                              method.getInputType().getName(),
                              method.getOutputType().getName());
        }
    }

    public static int getMethodCode(Descriptors.MethodDescriptor method)
    {
        if (method.getOptions().hasExtension(ipc.fdbus.protobus.Protobus.code))
        {
            return method.getOptions().getExtension(ipc.fdbus.protobus.Protobus.code);
        }
        else
        {
            return method.getIndex();
        }
    }

    public static int getMethodCode(Service service, String method_name)
    {
        Descriptors.ServiceDescriptor svc_desc = service.getDescriptorForType();
        List<Descriptors.MethodDescriptor> methods = svc_desc.getMethods();
        for (Descriptors.MethodDescriptor method : methods)
        {
            if (method.getName() == method_name)
            {
                return getMethodCode(method);
            }
        }
        return -1;
    }
}
