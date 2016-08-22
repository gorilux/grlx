#ifndef GRLX_RPC_QTWEBSOCKET_H
#define GRLX_RPC_QTWEBSOCKET_H

#include <memory>

#include <QObject>
#include <QByteArray>
#include <QWebSocket>
#include <QWebSocketServer>




namespace grlx {
namespace rpc{

namespace Qt {

struct WebSocket
{

    template<typename TConnection>
    class ConnectionImpl
    {
    public:
        ConnectionImpl(QWebSocket* webSocket)
            : webSocket(webSocket)
        {
            hookEvents(webSocket);
        }

        int sendMsg(const char* data, int size)
        {
            return webSocket->sendBinaryMessage(QByteArray(data,size));
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
                static_cast<TConnection*>(this)->msgHandler(msg.data(), msg.size());
            });


            QObject::connect(webSocket, &QWebSocket::textMessageReceived, [&] (const QString& msg)
            {

            });

            QObject::connect(webSocket, &QWebSocket::disconnected, [&]()
            {
                //heartbeatTimer.stop();
            });
            //QObject::connect(webSocket, &QWebSocket::pong, this, );
            //QObject::connect(webSocket, &QWebSocket::sslErrors, this, );
        }

        QWebSocket* webSocket;
    };

    template<typename TServer>
    class ServerImpl : public QWebSocketServer {
    public:

        ServerImpl(const QString &serverName, QWebSocketServer::SslMode secureMode, QObject* parent = 0)
            : QWebSocketServer(serverName,secureMode, parent)
        {
            QObject::connect(this, &QWebSocketServer::newConnection, this, &ServerImpl::handleNewConnection);
            QObject::connect(this, &QWebSocketServer::closed, this, &ServerImpl::handleClosed);
            //QObject::connect(this, &QWebSocketServer::acceptError, this, &ServerImpl::handleNewConnection);
//            QObject::connect(this, &QWebSocketServer::originAuthenticationRequired, this, &ServerImpl::handleNewConnection);
//            QObject::connect(this, &QWebSocketServer::peerVerifyError, this, &ServerImpl::handleNewConnection);
//            QObject::connect(this, &QWebSocketServer::sslErrors, this, &ServerImpl::handleNewConnection);



        }
    private:
        void handleNewConnection()
        {
            QWebSocket* incomingConnection = nextPendingConnection();

            if(!static_cast<TServer*>(this)->accept(std::make_shared< typename TServer::ConnectionType >(incomingConnection)))
            {
                incomingConnection->close();
                incomingConnection->deleteLater();
            }
        }
        void handleClosed()
        {

        }

    };
};




}
}
}


#endif // GRLX_RPC_QTWEBSOCKET_H
