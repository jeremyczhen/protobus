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

public class PBComponentClient
{
    private ArrayList<FdbusPBChannel> mRpcChannels;
    private Timer mTimer;
    private int mElapseTime;

    public PBComponentClient()
    {
        mElapseTime = 0;
    }

    private class EventHandle extends FdbPbExample.MediaPlayerService
    {
        /**
         * <code>rpc CallNowPlayingInfo(.FDBPB_EXAMPLE.NowPlayingDetails) returns (.FDBPB_EXAMPLE.ElapseTime);</code>
         */
        public void callNowPlayingInfo(
            com.google.protobuf.RpcController controller,
            fdbus.pb.example.FdbPbExample.NowPlayingDetails request,
            com.google.protobuf.RpcCallback<fdbus.pb.example.FdbPbExample.ElapseTime> done)
        {
            // Implemented at server side
        }

        /**
         * <code>rpc CallSearchMetaData(.google.protobuf.Empty) returns (.FDBPB_EXAMPLE.ElapseTime);</code>
         */
        public void callSearchMetaData(
            com.google.protobuf.RpcController controller,
            com.google.protobuf.Empty request,
            com.google.protobuf.RpcCallback<fdbus.pb.example.FdbPbExample.ElapseTime> done)
        {
            // Implemented at server side
        }

        /**
         * <code>rpc EmitElapseTime(.FDBPB_EXAMPLE.ElapseTime) returns (.google.protobuf.Empty);</code>
         */
        public void emitElapseTime(
            com.google.protobuf.RpcController controller,
            fdbus.pb.example.FdbPbExample.ElapseTime request,
            com.google.protobuf.RpcCallback<com.google.protobuf.Empty> done)
        {
            System.out.println("Broadcast is received - Elapse time is received: " +
                               request.toString());
        }

        /**
         * <code>rpc CallNextSong(.FDBPB_EXAMPLE.SongId) returns (.google.protobuf.Empty);</code>
         */
        public void callNextSong(
            com.google.protobuf.RpcController controller,
            fdbus.pb.example.FdbPbExample.SongId request,
            com.google.protobuf.RpcCallback<com.google.protobuf.Empty> done)
        {
            // Implemented at server side
        }
    }

    private class ServerInvoker extends TimerTask
    {
        public void run()
        {
            for (int i = 0; i < mRpcChannels.size(); i++)
            {
                {   // async invoke
                FdbPbExample.MediaPlayerService.Stub stub =
                        FdbPbExample.MediaPlayerService.newStub(mRpcChannels.get(i));
                FdbPbExample.NowPlayingDetails.Builder proto_builder =
                                                    FdbPbExample.NowPlayingDetails.newBuilder();
                proto_builder.setArtist("Artist from Java");
                proto_builder.setAlbum("Album from Java");
                proto_builder.setGenre("Genre from Java");
                proto_builder.setTitle("Title from Java");
                proto_builder.setFileName("Filename from Java");
                proto_builder.setElapseTime(mElapseTime);
                FdbPbExample.NowPlayingDetails now_playing = proto_builder.build();
                stub.callNowPlayingInfo(null, now_playing,
                    new RpcCallback<FdbPbExample.ElapseTime>() {
                        public void run(FdbPbExample.ElapseTime reply)
                        {
                            if (reply == null)
                            {
                                System.out.println("callNowPlayingInfo: something wrong!");
                                return;
                            }
                            System.out.println("Async reply is received - Elapse time is received: " +
                                               reply.toString());
                        }
                    });
                }

                {   // sync invoke
                FdbPbExample.MediaPlayerService.BlockingInterface stub =
                        FdbPbExample.MediaPlayerService.newBlockingStub(mRpcChannels.get(i));
                FdbPbExample.NowPlayingDetails.Builder proto_builder =
                                                    FdbPbExample.NowPlayingDetails.newBuilder();
                proto_builder.setArtist("Artist from Java");
                proto_builder.setAlbum("Album from Java");
                proto_builder.setGenre("Genre from Java");
                proto_builder.setTitle("Title from Java");
                proto_builder.setFileName("Filename from Java");
                proto_builder.setElapseTime(mElapseTime);
                FdbPbExample.NowPlayingDetails now_playing = proto_builder.build();
                fdbus.pb.example.FdbPbExample.ElapseTime reply = null;
                try {
                    reply = stub.callNowPlayingInfo(null, now_playing);
                } catch (Exception e) {
                    System.out.println(e);
                }
                if (reply == null)
                {
                    System.out.println("callNowPlayingInfo: something wrong!");
                    return;
                }
                System.out.println("Sync reply is received - Elapse time is received: " +
                                   reply.toString());
                }

                /*
                {   // send, but not supported yet
                    FdbPbExample.MediaPlayerService.Stub stub =
                            FdbPbExample.MediaPlayerService.newStub(mRpcChannels.get(i));
                    FdbPbExample.SongId.Builder proto_builder =
                                                        FdbPbExample.SongId.newBuilder();
                    proto_builder.setId(888888);
                    FdbPbExample.SongId song_id = proto_builder.build();
                    stub.callNextSong(null, song_id, null);
                }
                */
            }
            mElapseTime++;
        }
    }

    private ServerInvoker createInvoker()
    {
        return new ServerInvoker();
    }

    private EventHandle createEventHandle()
    {
        return new EventHandle();
    }

    public static void main(String[] args)
    {
        Fdbus fdbus = new Fdbus();
        PBComponentClient comp_client = new PBComponentClient();

        comp_client.mRpcChannels = new ArrayList<FdbusPBChannel>();
        FdbusPBComponent component = new FdbusPBComponent("media client component");

        for (String arg : args)
        {
            FdbusPBChannel channel = component.queryService(arg, comp_client.createEventHandle(),
                new FdbusAppListener.Connection()
                {
                    public void onConnectionStatus(int sid, boolean is_online, boolean is_first)
                    {
                        System.out.println(arg + ": onConnectionStatus - session: " + sid + ", is_online: " + is_online
                                            + ", is_first: " + is_first);
                    }
                },
                "Emit");
            comp_client.mRpcChannels.add(channel);
        }
        
        comp_client.mTimer = new Timer();
        comp_client.mTimer.schedule(comp_client.createInvoker(), 300, 300);
        
        try{Thread.sleep(80000000);}catch(InterruptedException e){System.out.println(e);}
    }
}

