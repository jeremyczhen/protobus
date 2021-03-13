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

#ifndef __CFDBPBRESPONSE_H__
#define __CFDBPBRESPONSE_H__
#include <google/protobuf/service.h>

namespace google
{
    namespace protobuf
    {
        class Empty;
    }
}

#define PBS_INVOKE_ASYNC        0
#define PBS_INVOKE_SYNC         (google::protobuf::Empty *)1

class CPbClosure : public google::protobuf::Closure
{
public:
    virtual google::protobuf::Message &getResponse() = 0;
};

//====================== response without extra argument ==========================
template <typename tMessage>
class CPbFunctionClosure0 : public CPbClosure {
public:
    typedef void (*FunctionType)(tMessage &);

    CPbFunctionClosure0(FunctionType function)
        : mFunction(function)
    {}
    ~CPbFunctionClosure0()
    {}

    void Run() override
    {
        mFunction(mResponse);
        delete this;
    }

    google::protobuf::Message &getResponse()
    {
        return mResponse;
    }

private:
    FunctionType mFunction;
    tMessage mResponse;
};

template <typename tClass, typename tMessage>
class CPbMethodClosure0 : public CPbClosure
{
public:
    typedef void (tClass::*MethodType)(tMessage &);

    CPbMethodClosure0(tClass* object, MethodType method)
        : mObject(object)
        , mMethod(method)
    {}
    ~CPbMethodClosure0()
    {}

    void Run() override
    {
        (mObject->*mMethod)(mResponse);
        delete this;
    }

    google::protobuf::Message &getResponse()
    {
        return mResponse;
    }

 private:
  tClass* mObject;
  MethodType mMethod;
  tMessage mResponse;
};

//====================== response with 1 argument ==========================
template <typename tMessage, typename tArg1>
class CPbFunctionClosure1 : public CPbClosure {
public:
    typedef void (*FunctionType)(tMessage &, tArg1);

    CPbFunctionClosure1(FunctionType function, tArg1 arg1)
        : mFunction(function)
        , mArg1(arg1)
    {}
    ~CPbFunctionClosure1()
    {}

    void Run() override
    {
        mFunction(mResponse, mArg1);
        delete this;
    }

    google::protobuf::Message &getResponse()
    {
        return mResponse;
    }

private:
    FunctionType mFunction;
    tMessage mResponse;
    tArg1 mArg1;
};

template <typename tClass, typename tMessage, typename tArg1>
class CPbMethodClosure1 : public CPbClosure
{
public:
    typedef void (tClass::*MethodType)(tMessage &, tArg1);

    CPbMethodClosure1(tClass* object, MethodType method, tArg1 arg1)
        : mObject(object)
        , mMethod(method)
        , mArg1(arg1)
    {}
    ~CPbMethodClosure1()
    {}

    void Run() override
    {
        (mObject->*mMethod)(mResponse, mArg1);
        delete this;
    }

    google::protobuf::Message &getResponse()
    {
        return mResponse;
    }

 private:
  tClass* mObject;
  MethodType mMethod;
  tMessage mResponse;
  tArg1 mArg1;
};

//====================== response with 2 arguments ==========================
template <typename tMessage, typename tArg1, typename tArg2>
class CPbFunctionClosure2 : public CPbClosure {
public:
    typedef void (*FunctionType)(tMessage &, tArg1, tArg2);

    CPbFunctionClosure2(FunctionType function, tArg1 arg1, tArg2 arg2)
        : mFunction(function)
        , mArg1(arg1)
        , mArg2(arg2)
    {}
    ~CPbFunctionClosure2()
    {}

    void Run() override
    {
        mFunction(mResponse, mArg1, mArg2);
        delete this;
    }

    google::protobuf::Message &getResponse()
    {
        return mResponse;
    }

private:
    FunctionType mFunction;
    tMessage mResponse;
    tArg1 mArg1;
    tArg2 mArg2;
};

template <typename tClass, typename tMessage, typename tArg1, typename tArg2>
class CPbMethodClosure2 : public CPbClosure
{
public:
    typedef void (tClass::*MethodType)(tMessage &, tArg1, tArg2);

    CPbMethodClosure2(tClass* object, MethodType method, tArg1 arg1, tArg2 arg2)
        : mObject(object)
        , mMethod(method)
        , mArg1(arg1)
        , mArg2(arg2)
    {}
    ~CPbMethodClosure2()
    {}

    void Run() override
    {
        (mObject->*mMethod)(mResponse, mArg1, mArg2);
        delete this;
    }

    google::protobuf::Message &getResponse()
    {
        return mResponse;
    }

 private:
  tClass* mObject;
  MethodType mMethod;
  tMessage mResponse;
  tArg1 mArg1;
  tArg2 mArg2;
};

//====================== response with 3 arguments ==========================
template <typename tMessage, typename tArg1, typename tArg2, typename tArg3>
class CPbFunctionClosure3 : public CPbClosure {
public:
    typedef void (*FunctionType)(tMessage &, tArg1, tArg2, tArg3);

    CPbFunctionClosure3(FunctionType function, tArg1 arg1, tArg2 arg2, tArg3 arg3)
        : mFunction(function)
        , mArg1(arg1)
        , mArg2(arg2)
        , mArg3(arg3)
    {}
    ~CPbFunctionClosure3()
    {}

    void Run() override
    {
        mFunction(mResponse, mArg1, mArg2, mArg3);
        delete this;
    }

    google::protobuf::Message &getResponse()
    {
        return mResponse;
    }

private:
    FunctionType mFunction;
    tMessage mResponse;
    tArg1 mArg1;
    tArg2 mArg2;
    tArg3 mArg3;
};

template <typename tClass, typename tMessage, typename tArg1, typename tArg2, typename tArg3>
class CPbMethodClosure3 : public CPbClosure
{
public:
    typedef void (tClass::*MethodType)(tMessage &, tArg1, tArg2, tArg3);

    CPbMethodClosure3(tClass* object, MethodType method, tArg1 arg1, tArg2 arg2, tArg3 arg3)
        : mObject(object)
        , mMethod(method)
        , mArg1(arg1)
        , mArg2(arg2)
        , mArg3(arg3)
    {}
    ~CPbMethodClosure3()
    {}

    void Run() override
    {
        (mObject->*mMethod)(mResponse, mArg1, mArg2, mArg3);
        delete this;
    }

    google::protobuf::Message &getResponse()
    {
        return mResponse;
    }

 private:
  tClass* mObject;
  MethodType mMethod;
  tMessage mResponse;
  tArg1 mArg1;
  tArg2 mArg2;
  tArg3 mArg3;
};

//====================== response with 4 arguments ==========================
template <typename tMessage, typename tArg1, typename tArg2, typename tArg3, typename tArg4>
class CPbFunctionClosure4 : public CPbClosure {
public:
    typedef void (*FunctionType)(tMessage &, tArg1, tArg2, tArg3, tArg3);

    CPbFunctionClosure4(FunctionType function, tArg1 arg1, tArg2 arg2, tArg3 arg3, tArg4 arg4)
        : mFunction(function)
        , mArg1(arg1)
        , mArg2(arg2)
        , mArg3(arg3)
        , mArg4(arg4)
    {}
    ~CPbFunctionClosure4()
    {}

    void Run() override
    {
        mFunction(mResponse, mArg1, mArg2, mArg3, mArg4);
        delete this;
    }

    google::protobuf::Message &getResponse()
    {
        return mResponse;
    }

private:
    FunctionType mFunction;
    tMessage mResponse;
    tArg1 mArg1;
    tArg2 mArg2;
    tArg3 mArg3;
    tArg4 mArg4;
};

template <typename tClass, typename tMessage, typename tArg1, typename tArg2, typename tArg3, typename tArg4>
class CPbMethodClosure4 : public CPbClosure
{
public:
    typedef void (tClass::*MethodType)(tMessage &, tArg1, tArg2, tArg3, tArg4);

    CPbMethodClosure4(tClass* object, MethodType method, tArg1 arg1, tArg2 arg2, tArg3 arg3, tArg4 arg4)
        : mObject(object)
        , mMethod(method)
        , mArg1(arg1)
        , mArg2(arg2)
        , mArg3(arg3)
        , mArg4(arg4)
    {}
    ~CPbMethodClosure4()
    {}

    void Run() override
    {
        (mObject->*mMethod)(mResponse, mArg1, mArg2, mArg3, mArg4);
        delete this;
    }

    google::protobuf::Message &getResponse()
    {
        return mResponse;
    }

 private:
  tClass* mObject;
  MethodType mMethod;
  tMessage mResponse;
  tArg1 mArg1;
  tArg2 mArg2;
  tArg3 mArg3;
  tArg4 mArg4;
};

//====================== response without argument ==========================
template <typename tMessage>
inline google::protobuf::Closure* newPbsFunction(typename CPbFunctionClosure0<tMessage>::FunctionType function)
{
    return new CPbFunctionClosure0<tMessage>(function);
}

template <typename tClass, typename tMessage>
inline google::protobuf::Closure* newPbsMethod(tClass* object,
        typename CPbMethodClosure0<tClass, tMessage>::MethodType method)
{
    return new CPbMethodClosure0<tClass, tMessage>(object, method);
}

//====================== response with 1 argument ==========================
template <typename tMessage, typename tArg1>
inline google::protobuf::Closure* newPbsFunction(typename CPbFunctionClosure1<tMessage, tArg1>::FunctionType function,
         tArg1 arg1)
{
    return new CPbFunctionClosure1<tMessage, tArg1>(function, arg1);
}

template <typename tClass, typename tMessage, typename tArg1>
inline google::protobuf::Closure* newPbsMethod(tClass* object,
        typename CPbMethodClosure1<tClass, tMessage, tArg1>::MethodType method,
         tArg1 arg1)
{
    return new CPbMethodClosure1<tClass, tMessage, tArg1>(object, method, arg1);
}

//====================== response with 2 arguments ==========================
template <typename tMessage, typename tArg1, typename tArg2>
inline google::protobuf::Closure* newPbsFunction(typename CPbFunctionClosure2<tMessage, tArg1, tArg2>::FunctionType function,
         tArg1 arg1, tArg2 arg2)
{
    return new CPbFunctionClosure2<tMessage, tArg1, tArg2>(function, arg1, arg2);
}
 
template <typename tClass, typename tMessage, typename tArg1, typename tArg2>
inline google::protobuf::Closure* newPbsMethod(tClass* object,
        typename CPbMethodClosure2<tClass, tMessage, tArg1, tArg2>::MethodType method,
         tArg1 arg1, tArg2 arg2)
{
    return new CPbMethodClosure2<tClass, tMessage, tArg1, tArg2>(object, method, arg1, arg2);
}

//====================== response with 3 arguments ==========================
template <typename tMessage, typename tArg1, typename tArg2, typename tArg3>
inline google::protobuf::Closure* newPbsFunction(typename CPbFunctionClosure3<tMessage, tArg1, tArg2, tArg3>::FunctionType function,
         tArg1 arg1, tArg2 arg2, tArg3 arg3)
{
    return new CPbFunctionClosure3<tMessage, tArg1, tArg2, tArg3>(function, arg1, arg2, arg3);
}
  
template <typename tClass, typename tMessage, typename tArg1, typename tArg2, typename tArg3>
inline google::protobuf::Closure* newPbsMethod(tClass* object,
        typename CPbMethodClosure3<tClass, tMessage, tArg1, tArg2, tArg3>::MethodType method,
         tArg1 arg1, tArg2 arg2, tArg3 arg3)
{
    return new CPbMethodClosure3<tClass, tMessage, tArg1, tArg2, tArg3>(object, method, arg1, arg2, arg3);
}

//====================== response with 3 arguments ==========================
template <typename tMessage, typename tArg1, typename tArg2, typename tArg3, typename tArg4>
inline google::protobuf::Closure* newPbsFunction(typename CPbFunctionClosure4<tMessage, tArg1, tArg2, tArg3, tArg4>::FunctionType function,
      tArg1 arg1, tArg2 arg2, tArg3 arg3, tArg4 arg4)
{
    return new CPbFunctionClosure4<tMessage, tArg1, tArg2, tArg3, tArg4>(function, arg1, arg2, arg3, arg4);
}

template <typename tClass, typename tMessage, typename tArg1, typename tArg2, typename tArg3, typename tArg4>
inline google::protobuf::Closure* newPbsMethod(tClass* object,
     typename CPbMethodClosure4<tClass, tMessage, tArg1, tArg2, tArg3, tArg4>::MethodType method,
      tArg1 arg1, tArg2 arg2, tArg3 arg3, tArg4 arg4)
{
    return new CPbMethodClosure4<tClass, tMessage, tArg1, tArg2, tArg3, tArg4>(object, method, arg1, arg2, arg3, arg4);
}

#endif
