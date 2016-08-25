#ifndef GRLX_RPC_QTWEBSOCKET_H
#define GRLX_RPC_QTWEBSOCKET_H

#include <memory>

#include <QObject>
#include <QByteArray>
#include <QWebSocket>
#include <QWebSocketServer>

#include <QDebug>


#include "../types.h"



namespace grlx {
namespace rpc{

namespace Qt {

struct WebSocket
{


    template<typename TClient>
    class ClientImpl : public QWebSocket {
    public:
        using Type = ClientImpl;

        ClientImpl(const QString &origin = QString(), QWebSocketProtocol::Version version = QWebSocketProtocol::VersionLatest, QObject* parent = nullptr)
            : QWebSocket(origin,version, parent)
        {
            hookEvents(this);
        }

        int sendMsg(const char* data, int size)
        {
            QByteArray msg(data,size);
            qDebug() << "CLT>:" << msg;
            return this->sendBinaryMessage(msg);
        }

    private:

        void hookEvents(QWebSocket* webSocket)
        {
            QObject::connect(webSocket, &QWebSocket::connected, [&]()
            {
                //heartbeatTimer.start();
            });


            QObject::connect(webSocket, &QWebSocket::binaryMessageReceived, [&](const QByteArray& msg)
            {
                qDebug() << "CLT<:" << msg;
                static_cast<typename TClient::ConnectionType*>(this)->msgHandler(msg.data(), msg.size());
            });


            QObject::connect(webSocket, &QWebSocket::textMessageReceived, [&] (const QString& msg)
            {
                qDebug() << "SRV:<" << msg;
            });

            QObject::connect(webSocket, &QWebSocket::disconnected, [&]()
            {
                //heartbeatTimer.stop();
            });
            //QObject::connect(webSocket, &QWebSocket::pong, this, );
            //QObject::connect(webSocket, &QWebSocket::sslErrors, this, );
        }


    };


    template<typename TServer>
    class SocketWrapper : public std::enable_shared_from_this< SocketWrapper<TServer> >
    {
    public:

        using Type = SocketWrapper;

        SocketWrapper(QWebSocket* webSocket, TServer* server)
            : webSocket(webSocket),
              server( server )
        {
            hookEvents(webSocket);
        }
        ~SocketWrapper()
        {
            webSocket->deleteLater();
        }

        int sendMsg(const char* data, int size)
        {
            QByteArray msg(data,size);
            qDebug() << "SRV:>" << msg;
            return webSocket->sendBinaryMessage(msg);
        }

    private:

        void hookEvents(QWebSocket* webSocket)
        {
            QObject::connect(webSocket, &QWebSocket::connected, [&]()
            {
                //heartbeatTimer.start();
            });


            QObject::connect(webSocket, &QWebSocket::binaryMessageReceived, [&](const QByteArray& msg)
            {
                qDebug() << "SRV:<" << msg;
                static_cast<typename TServer::ConnectionType*>(this)->msgHandler(msg.data(), msg.size());
            });


            QObject::connect(webSocket, &QWebSocket::textMessageReceived, [&] (const QString& msg)
            {
                qDebug() << "SRV:<" << msg;
            });

            QObject::connect(webSocket, &QWebSocket::disconnected, [&]()
            {
                //heartbeatTimer.stop();



            });
            //QObject::connect(webSocket, &QWebSocket::pong, this, );
            //QObject::connect(webSocket, &QWebSocket::sslErrors, this, );
        }

        void handleDisconnected()
        {
            server->handleDisconnected();
        }

        QWebSocket* webSocket;
        TServer* server;
    };

    template<typename TServer>
    class ServerImpl : public QWebSocketServer {
    public:

        using Type = ServerImpl;
        using ConnectionType = typename SocketWrapper<TServer>::Type;

        ServerImpl(const QString &serverName, QWebSocketServer::SslMode secureMode, QObject* parent = 0)
            : QWebSocketServer(serverName,secureMode, parent)
        {
            QObject::connect(this, &QWebSocketServer::newConnection, this, &ServerImpl::handleNewConnection);
            QObject::connect(this, &QWebSocketServer::closed, [&]()
            {
                static_cast<typename TServer::Type*>(this)->handleClosed();
            });
            //QObject::connect(this, &QWebSocketServer::acceptError, this, &ServerImpl::handleNewConnection);
//            QObject::connect(this, &QWebSocketServer::originAuthenticationRequired, this, &ServerImpl::handleNewConnection);
//            QObject::connect(this, &QWebSocketServer::peerVerifyError, this, &ServerImpl::handleNewConnection);
//            QObject::connect(this, &QWebSocketServer::sslErrors, this, &ServerImpl::handleNewConnection);
        }

        template<typename T>
        void handleDisconnected(std::shared_ptr<T> connection)
        {
            static_cast<typename TServer::Type*>(this)->disconnected( connection );
        }


    private:

        void handleNewConnection()
        {
            QWebSocket* incomingConnection = nextPendingConnection();

            auto connection = std::make_shared< typename TServer::ConnectionType >(incomingConnection, static_cast<TServer*>( this ) );

            if(!static_cast<TServer*>(this)->accept(std::move(connection)))
            {
                incomingConnection->close();
                incomingConnection->deleteLater();
            }
        }

    };
};




}
}
}


#endif // GRLX_RPC_QTWEBSOCKET_H
