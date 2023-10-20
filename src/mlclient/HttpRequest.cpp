// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#include "HttpRequest.h"

HttpRequest::HttpRequest(Method method, const QUrl& url) :
    method_{method},
    url_{url}
{
}

void HttpRequest::addHeader(QString name, QString value)
{
    headers_.insert(name, value);
}

void HttpRequest::setBody(const HttpBody& body)
{
    body_ = body;
}
