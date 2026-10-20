#pragma once

#include <QtGlobal>

#include "EmulatorRequest.h"


class EmulatorResponse
{
public:
    enum class Result
    {
        Success,
        Failed,
        InvalidState,
        InvalidParameter,
        Cancelled
    };

    EmulatorResponse(EmulatorRequest::RequestId id, EmulatorRequest::Type type, Result result)
        : m_id(id)
        , m_type(type)
        , m_result(result)
    {
    }

    virtual ~EmulatorResponse() = default;

    EmulatorRequest::RequestId id() const { return m_id; }
    EmulatorRequest::Type type() const { return m_type; }
    Result result() const { return m_result; }

    bool success() const { return m_result == Result::Success; }

private:
    EmulatorRequest::RequestId m_id;
    EmulatorRequest::Type m_type;
    Result m_result;
};
