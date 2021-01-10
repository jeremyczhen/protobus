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
import com.google.protobuf.Descriptors;
import com.google.protobuf.RpcController;
import com.google.protobuf.Message;
import com.google.protobuf.RpcCallback;

public class FdbusPBController
{
    public static class Client implements RpcController
    {
        public Client()
        {
            mIsPublish = false;
            mTimeout = 0;
            mFailed = false;
            mForceUpdate = false;
        }

        public void timeout(int timeout)
        {
            mTimeout = timeout;
        }

        public int timeout()
        {
            return mTimeout;
        }

        public void enablePublish(boolean force_update)
        {
            mIsPublish = true;
            mForceUpdate = force_update;
        }
        public boolean isPublish()
        {
            return mIsPublish;
        }

        public void topic(String topic)
        {
            mTopic = topic;
        }

        public String topic()
        {
            return mTopic;
        }

        public void forceUpdate(boolean force_update)
        {
            mForceUpdate = force_update;
        }
        public boolean forceUpdate()
        {
            return mForceUpdate;
        }

        public void reset()
        {
            mFailed = false;
            mReason = null;
        }

        public boolean failed()
        {
            return mFailed;
        }
        public void notifyOnCancel(RpcCallback<Object> callback)
        {
        }
        public String errorText()
        {
            return mReason;
        }
        public void startCancel()
       {}
       public  boolean isCanceled()
       {
           return false;
       }
       public void setFailed(String reason)
       {
           mFailed = true;
           mReason = reason;
       }
        
        private boolean mIsPublish;
        private String mTopic;
        private int mTimeout;
        private boolean mFailed;
        private String mReason;
        //EFdbQOS mQOS;
        private boolean mForceUpdate;
    }

    public static class Server implements RpcController
    {
        public Server(FdbusMessage message)
        {
            mMessage = message;
            mFailed = false;
        }
        public FdbusMessage message()
        {
            return mMessage;
        }
        public void reset()
        {
            mFailed = false;
            mReason = null;
        }
        public boolean failed()
        {
            return mFailed;
        }
        public void notifyOnCancel(RpcCallback<Object> callback)
        {
        }
        public String errorText()
        {
            return mReason;
        }
        public void startCancel()
       {}
       public  boolean isCanceled()
       {
           return false;
       }
       public void setFailed(String reason)
       {
           mFailed = true;
           mReason = reason;
       }
       private FdbusMessage mMessage;
       private boolean mFailed;
       private String mReason;
    }
}

