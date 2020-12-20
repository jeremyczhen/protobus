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
#include <protobus/CFdbRpcController.h>

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
    CFdbRpcController *fdb_ctrl = 0;
    if (controller)
    {
        fdb_ctrl = fdb_dynamic_cast_if_available<CFdbRpcController *>(controller);
    }
    if (mEndpoint->role() == FDB_OBJECT_ROLE_CLIENT)
    {
        if (fdb_ctrl && fdb_ctrl->isPublish())
        {
            if (request)
            {
                CFdbProtoMsgBuilder builder(*request);
                mEndpoint->publish(code,
                                   builder,
                                   fdb_ctrl->topic().c_str(),
                                   fdb_ctrl->forceUpdate(),
                                   fdb_ctrl->qos());
            }
            else
            {
                mEndpoint->publish(code,
                                   (const void *)0,
                                   0,
                                   fdb_ctrl->topic().c_str(),
                                   fdb_ctrl->forceUpdate(),
                                   fdb_ctrl->qos());
            }
            return;
        }
        int32_t timeout = 0;
        EFdbQOS qos = FDB_QOS_RELIABLE;
        if (fdb_ctrl)
        {
            timeout = fdb_ctrl->timeout();
            qos = fdb_ctrl->qos();
        }

        if (done)
        {   //async call
            if (request)
            {
                CFdbProtoMsgBuilder builder(*request);
                mComponent->invoke(mEndpoint, code, builder,
                    [done, response, controller](CBaseJob::Ptr &msg_ref, CFdbBaseObject *obj)
                    {
                        fdb_call_done(castToMessage<CFdbMessage *>(msg_ref), response, done, controller);
                    },
                    timeout);
            }
            else
            {
                mComponent->invoke(mEndpoint, code,
                [done, response, controller](CBaseJob::Ptr &msg_ref, CFdbBaseObject *obj)
                {
                    fdb_call_done(castToMessage<CFdbMessage *>(msg_ref), response, done, controller);
                },
                (const void *)0, 0, timeout);
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
                    mEndpoint->invoke(ref, builder, timeout);
                }
                else
                {
                    mEndpoint->invoke(ref, 0, 0, timeout);
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
                    mEndpoint->send(code, builder, qos);
                }
                else
                {
                    mEndpoint->send(code, (const void *)0, 0, qos);
                }
            }
        }
    }
    else
    {
        const char *topic = 0;
        EFdbQOS qos = FDB_QOS_RELIABLE;
        if (fdb_ctrl)
        {
            topic = fdb_ctrl->topic().c_str();
            qos = fdb_ctrl->qos();
        }

        if (request)
        {
            CFdbProtoMsgBuilder builder(*request);
            mEndpoint->broadcast(code, builder, topic, qos);
        }
        else
        {
            mEndpoint->broadcast(code, (const void *)0, 0, topic, qos);
        }
    }
}

