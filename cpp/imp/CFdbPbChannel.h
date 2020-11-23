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

#ifndef __CFDBPBCHANNEL_H__
#define __CFDBPBCHANNEL_H__

#include <google/protobuf/service.h>

class CBaseEndpoint;
class CFdbPbComponent;

class CFdbPbChannel : public google::protobuf::RpcChannel
{
public:
    CFdbPbChannel(CFdbPbComponent *component, CBaseEndpoint *endpoint)
        : mComponent(component)
        , mEndpoint(endpoint)
    {};
    void CallMethod(const google::protobuf::MethodDescriptor *method,
                    google::protobuf::RpcController *controller,
                    const google::protobuf::Message *request,
                    google::protobuf::Message *response,
                    google::protobuf::Closure *done);
private:
    CFdbPbComponent *mComponent;
    CBaseEndpoint *mEndpoint;
};

#endif
