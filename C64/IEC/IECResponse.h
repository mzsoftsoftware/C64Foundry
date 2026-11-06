#pragma once

#include <QtGlobal>


class IECResponse
{
public:
    using RequestId = quint64;

    enum class Result
    {
        Success,
        Failed,
        InvalidState,
        InvalidParameter,
        Cancelled
    };

    explicit IECResponse(RequestId requestId)
        : m_requestId(requestId)
    {
    }

    virtual ~IECResponse() = default;

    RequestId requestId() const { return m_requestId; }
    Result result() const { return m_result; }

    bool success() const { return m_result == Result::Success; }

    void setResult(Result result) { m_result = result; }

private:
    RequestId m_requestId = 0;
    Result m_result = Result::Failed;
};
