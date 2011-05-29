/*
    RoomControlServer. Home automation for controlling sockets, leds and music.
    Copyright (C) 2010  David Gräff

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Purpose: https server
*/

// QT
#include <QDebug>
#include <QDateTime>

#include "paths.h"
#include "config.h"
#include "httpresponsefile.h"
#include "httprequest.h"
#include "httpserver.h"

HttpResponseFile::HttpResponseFile(HttpRequest* request, QObject* parent) : QObject(parent) {
    QFile www(request->m_requestedfile);
    QFileInfo wwwinfo(www);
    if (wwwinfo.exists()) {
        if (!wwwinfo.isReadable()) {
            request->m_socket->write("HTTP/1.1 403 Forbidden\r\n");
            request->m_socket->write("Content-Length: 0\r\n");
            HttpServer::writeDefaultHeaders(request->m_socket);
            request->m_socket->write("\r\n");
            request->m_socket->flush();
            return;
        }
        QByteArray lastModified = QLocale(QLocale::English).toString(wwwinfo.lastModified(), QLatin1String("ddd, d MMMM yyyy hh:mm:ss")).toAscii() + " GMT";
        QByteArray type = "text/html";
        if (wwwinfo.suffix()==QLatin1String("gif")||wwwinfo.suffix()==QLatin1String("png")) {
            type = "image/"+wwwinfo.suffix().toAscii();
        } else if (wwwinfo.suffix()==QLatin1String("ico")) {
            type = "image/vnd.microsoft.icon";
        } else if (wwwinfo.suffix()==QLatin1String("jpg")||wwwinfo.suffix()==QLatin1String("jpeg")) {
            type = "image/jpeg";
        } else if (wwwinfo.suffix()==QLatin1String("css")) {
            type = "text/css";
        } else if (wwwinfo.suffix()==QLatin1String("js")) {
            type = "application/x-javascript";
        } else if (wwwinfo.suffix()==QLatin1String("xml")) {
            type = "text/xml";
        }


        if (request->m_header.value("If-Modified-Since") == lastModified) {
            request->m_socket->write("HTTP/1.1 304 Not modified\r\n");
            HttpServer::writeDefaultHeaders(request->m_socket);
            request->m_socket->write("\r\n");
            request->m_socket->flush();
        } else {
            request->m_socket->write("HTTP/1.1 200 OK\r\n");
            HttpServer::writeDefaultHeaders(request->m_socket);

            request->m_socket->write("Last-Modified: " + lastModified + "\r\n");
            request->m_socket->write("Content-Type: " + type + "\r\n");
            request->m_socket->write("Content-Length:"+QByteArray::number(wwwinfo.size())+"\r\n");

            request->m_socket->write("\r\n");

            if (request->m_requestType==HttpRequest::Get || request->m_requestType==HttpRequest::Post) {
                www.open(QIODevice::ReadOnly);
                request->m_socket->write(www.readAll());
                www.close();
            }
            request->m_socket->flush();
        }
    } else {
        qWarning() << "File not found: " << wwwinfo.absoluteFilePath();
        request->m_socket->write("HTTP/1.1 404 Not Found\r\n");
        request->m_socket->write("Content-Length: 0\r\n");
        HttpServer::writeDefaultHeaders(request->m_socket);
        request->m_socket->write("\r\n");
        request->m_socket->flush();
    }
}
HttpResponseFile::~HttpResponseFile() {}
