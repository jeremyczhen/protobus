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

import ipc.fdbus.Fdbus;
import ipc.fdbus.FdbusPBComponent;
import ipc.fdbus.FdbusAppListener;
import ipc.fdbus.FdbusPBController;
import ipc.fdbus.FdbusProtoBuilder;
import ipc.fdbus.FdbusPBChannel;

import fdbus.pb.example.FdbPbExample;

import com.google.protobuf.RpcCallback;

import java.util.ArrayList;
import java.util.Timer;
import java.util.TimerTask;

public class PBComponentServer
{
    private ArrayList<FdbusPBChannel> mRpcChannels;
    private Timer mTimer;
    private int mElapseTime;

    public PBComponentServer()
    {
        mElapseTime = 0;
    }

    private class MessageHandle extends FdbPbExample.MediaPlayerService
    {
        /**
         * <code>rpc CallNowPlayingInfo(.FDBPB_EXAMPLE.NowPlayingDetails) returns (.FDBPB_EXAMPLE.ElapseTime);</code>
         */
        public void callNowPlayingInfo(
            com.google.protobuf.RpcController controller,
            fdbus.pb.example.FdbPbExample.NowPlayingDetails request,
            com.google.protobuf.RpcCallback<fdbus.pb.example.FdbPbExample.ElapseTime> done)
        {
            System.out.println("callNowPlayingInfo is called - " + request.toString());

            FdbPbExample.ElapseTime.Builder proto_builder = FdbPbExample.ElapseTime.newBuilder();
            proto_builder.setHour(1);
            proto_builder.setMinute(1);
            proto_builder.setSecond(mElapseTime++);
            FdbPbExample.ElapseTime et = proto_builder.build();
            
            FdbusPBController.Server ctrl = (FdbusPBController.Server)controller;
            ctrl.message().reply(new FdbusProtoBuilder(et));
        }

        /**
         * <code>rpc CallSearchMetaData(.google.protobuf.Empty) returns (.FDBPB_EXAMPLE.ElapseTime);</code>
         */
        public void callSearchMetaData(
            com.google.protobuf.RpcController controller,
            com.google.protobuf.Empty request,
            com.google.protobuf.RpcCallback<fdbus.pb.example.FdbPbExample.ElapseTime> done)
        {
            System.out.println("callSearchMetaData is called - " + request.toString());

            FdbPbExample.ElapseTime.Builder proto_builder = FdbPbExample.ElapseTime.newBuilder();
            proto_builder.setHour(2);
            proto_builder.setMinute(2);
            proto_builder.setSecond(mElapseTime++);
            FdbPbExample.ElapseTime et = proto_builder.build();
            
            FdbusPBController.Server ctrl = (FdbusPBController.Server)controller;
            ctrl.message().reply(new FdbusProtoBuilder(et));
        }

        /**
         * <code>rpc EmitElapseTime(.FDBPB_EXAMPLE.ElapseTime) returns (.google.protobuf.Empty);</code>
         */
        public void emitElapseTime(
            com.google.protobuf.RpcController controller,
            fdbus.pb.example.FdbPbExample.ElapseTime request,
            com.google.protobuf.RpcCallback<com.google.protobuf.Empty> done)
        {
            // Implemented at client side
        }

        /**
         * <code>rpc CallNextSong(.FDBPB_EXAMPLE.SongId) returns (.google.protobuf.Empty);</code>
         */
        public void callNextSong(
            com.google.protobuf.RpcController controller,
            fdbus.pb.example.FdbPbExample.SongId request,
            com.google.protobuf.RpcCallback<com.google.protobuf.Empty> done)
        {
            System.out.println("callNextSong is called - " + request.toString());
        }
    }

    private class ClientBroadcaster extends TimerTask
    {
        public void run()
        {
            for (int i = 0; i < mRpcChannels.size(); i++)
            {
                FdbPbExample.MediaPlayerService.Stub stub =
                        FdbPbExample.MediaPlayerService.newStub(mRpcChannels.get(i));
                FdbPbExample.ElapseTime.Builder proto_builder = FdbPbExample.ElapseTime.newBuilder();
                proto_builder.setHour(0);
                proto_builder.setMinute(0);
                proto_builder.setSecond(mElapseTime);
                FdbPbExample.ElapseTime et = proto_builder.build();
                stub.emitElapseTime(null, et, null);
            }
            mElapseTime++;
        }
    }

    private ClientBroadcaster createBroadcaster()
    {
        return new ClientBroadcaster();
    }

    private MessageHandle createMessageHandle()
    {
        return new MessageHandle();
    }

    public static void main(String[] args)
    {
        Fdbus fdbus = new Fdbus();
        PBComponentServer comp_server = new PBComponentServer();

        comp_server.mRpcChannels = new ArrayList<FdbusPBChannel>();
        FdbusPBComponent component = new FdbusPBComponent("media client component");

        for (String arg : args)
        {
            FdbusPBChannel channel = component.offerService(arg, comp_server.createMessageHandle(),
                new FdbusAppListener.Connection()
                {
                    public void onConnectionStatus(int sid, boolean is_online, boolean is_first)
                    {
                        System.out.println(arg + ": onConnectionStatus - session: " + sid + ", is_online: " + is_online
                                            + ", is_first: " + is_first);
                    }
                },
                "Call");
            comp_server.mRpcChannels.add(channel);
        }
        
        comp_server.mTimer = new Timer();
        comp_server.mTimer.schedule(comp_server.createBroadcaster(), 300, 300);
        
        try{Thread.sleep(80000000);}catch(InterruptedException e){System.out.println(e);}
    }
}
