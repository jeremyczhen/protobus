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

#ifndef __CFDBRPCCONTROLLER_H__
#define __CFDBRPCCONTROLLER_H__

#include <google/protobuf/service.h>
#include <common_base/common_defs.h>

namespace google
{
    namespace protobuf
    {
        class Closure;
    }
}

class CFdbRpcController : public google::protobuf::RpcController
{
public:
    CFdbRpcController()
        : mIsPublish(false)
        , mTimeout(0)
        , mFailed(false)
        , mQOS(FDB_QOS_RELIABLE)
        , mForceUpdate(false)
    {}
    void timeout(int32_t timeout)
    {
        mTimeout = timeout;
    }
    int32_t timeout() const
    {
        return mTimeout;
    }
    void enablePublish(bool force_update = false)
    {
        mIsPublish = true;
        mForceUpdate = force_update;
    }
    bool isPublish() const
    {
        return mIsPublish;
    }
    void topic(const char *topic)
    {
        if (topic)
        {
            mTopic = topic;
        }
    }
    const std::string &topic() const
    {
        return mTopic;
    }
    void qos(EFdbQOS qos)
    {
        mQOS = qos;
    }
    EFdbQOS qos() const
    {
        return mQOS;
    }
    bool forceUpdate() const
    {
        return mForceUpdate;
    }
    void Reset()
    {
        mFailed = false;
        mReason.clear();
    }
    bool Failed() const
    {
        return mFailed;
    }
    void StartCancel()
    {}
    void setFailed(const std::string &reason)
    {
        mFailed = true;
        mReason = reason;
    }
    bool IsCancelled() const
    {
        return false;
    }
    void NotifyOnCancel(google::protobuf::Closure *callback)
    {}
    bool IsPublish() const
    {
        return mIsPublish;
    }

private:
    bool mIsPublish;
    std::string mTopic;
    int32_t mTimeout;
    bool mFailed;
    std::string mReason;
    EFdbQOS mQOS;
    bool mForceUpdate;
    
};

#endif
