// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "HttpRequest.h"

HttpRequest::HttpRequest(Method method, QUrl url) :
    method_{method},
        url_{std::move(url)}
{
}

void HttpRequest::addHeader(const QString& name, const QString& value)
{
    headers_.insert(name, value);
}

void HttpRequest::setBody(const HttpBody& body)
{
    body_ = body;
}
