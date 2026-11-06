#pragma once

#include <QtGlobal>
#include <QSharedPointer>

#include "IECResponse.h"


class IECRequest
{
public:
    using RequestId = IECResponse::RequestId;

    virtual ~IECRequest() = default;

    IECRequest(const IECRequest&) = delete;
    IECRequest& operator=(const IECRequest&) = delete;

    RequestId id() const { return m_id; }
    void setId(RequestId id) { m_id = id; }

    virtual QSharedPointer<IECResponse> createResponse() const = 0;

protected:
    IECRequest() = default;

private:
    RequestId m_id = 0;
};
