// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#include "HttpResponse.h"

HttpResponse::HttpResponse(QNetworkReply* reply, QObject* parent) :
    QObject{parent},
    reply_{reply}
{
    reply_->setParent(this);

    connect(reply_, &QIODevice::readyRead, this, &HttpResponse::onReplyReadyRead);
    connect(reply_, &QNetworkReply::finished, this, &HttpResponse::onReplyFinished);
    connect(reply_, &QNetworkReply::downloadProgress, this, &HttpResponse::downloadProgress);
}

void HttpResponse::onReplyFinished()
{
    if (reply_->error())
    {
        //statusLabel->setText(tr("Download failed:\n%1.").arg(reply->errorString()));
    }
    else
    {
        body_.setContentType(reply_->header(QNetworkRequest::ContentTypeHeader).toString());
        body_.setBinaryData(receiveBuffer_);
    }

    auto statusCode = reply_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    emit finished(reply_->error(), statusCode);
}

void HttpResponse::onReplyReadyRead()
{
    receiveBuffer_.append(reply_->readAll());
}
