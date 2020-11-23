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

#include "CFdbPbChannel.h"
#include <common_base/CFdbProtoMsgBuilder.h>
#include <common_base/CFdbMessage.h>
#define FDB_LOG_TAG "FDB_PB"
#include <common_base/fdb_log_trace.h>
#include <common_base/CBaseEndpoint.h>
#include <protobus/CFdbPbComponent.h>

static void fdb_decode_status(CFdbMessage *msg, google::protobuf::RpcController *controller)
{
    /* Unable to get intended reply from server... Check what happen. */
    int32_t id;
    std::string reason;
    if (!msg->decodeStatus(id, reason))
    {
        if (controller)
        {
            controller->SetFailed("onReply: fail to decode status!");
        }
        return;
    }
    if (controller)
    {
        controller->SetFailed(reason);
    }
}

static void fdb_call_done(CFdbMessage *msg, google::protobuf::Message* response, google::protobuf::Closure* done,
                          google::protobuf::RpcController *controller)
{
    if (response)
    {
        if (msg->isStatus())
        {
            fdb_decode_status(msg, controller);
            return;
        }
        CFdbProtoMsgParser parser(*response);
        if (!msg->deserialize(parser))
        {
            FDB_LOG_E("CFdbPbChannel: unable to deode message!\n");
            return;
        }
    }
    if (done)
    {
        done->Run();
    }
}

void CFdbPbChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                               google::protobuf::RpcController *controller,
                               const google::protobuf::Message *request,
                               google::protobuf::Message *response,
                               google::protobuf::Closure *done)
{
    FdbMsgCode_t code = method->index();
    if (mEndpoint->role() == FDB_OBJECT_ROLE_CLIENT)
    {
        if (done)
        {   //async call
            if (request)
            {
                CFdbProtoMsgBuilder builder(*request);
                mComponent->invoke(mEndpoint, code, builder,
                    [done, response, controller](CBaseJob::Ptr &msg_ref, CFdbBaseObject *obj)
                    {
                        fdb_call_done(castToMessage<CFdbMessage *>(msg_ref), response, done, controller);
                    });
            }
            else
            {
                mComponent->invoke(mEndpoint, code,
                [done, response, controller](CBaseJob::Ptr &msg_ref, CFdbBaseObject *obj)
                {
                    fdb_call_done(castToMessage<CFdbMessage *>(msg_ref), response, done, controller);
                });
            }
        }
        else
        {   // sync call or send
            if (response)
            {   // sync
                auto msg = new CFdbMessage(code);
                CBaseJob::Ptr ref(msg);
                if (request)
                {
                    CFdbProtoMsgBuilder builder(*request);
                    mEndpoint->invoke(ref, builder);
                }
                else
                {
                    mEndpoint->invoke(ref);
                }
                if (msg->isStatus())
                {
                    fdb_decode_status(msg, controller);
                    return;
                }
                fdb_call_done(msg, response, done, controller);
            }
            else
            {   // send
                if (request)
                {
                    CFdbProtoMsgBuilder builder(*request);
                    mEndpoint->send(code, builder);
                }
                else
                {
                    mEndpoint->send(code);
                }
            }
        }
    }
    else
    {
        if (request)
        {
            CFdbProtoMsgBuilder builder(*request);
            mEndpoint->broadcast(code, builder);
        }
        else
        {
            mEndpoint->broadcast(code);
        }
    }
}

