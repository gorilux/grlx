#ifndef GRLX_RPC_QTWEBSOCKET_H
#define GRLX_RPC_QTWEBSOCKET_H

#include <memory>

#include <QObject>
#include <QWebSocket>
#include <QWebSocketServer>


namespace grlx {
namespace rpc{

namespace Qt {
namespace Private {

template<typename T>
class WebSocketConnection
{

public:

};

template<typename TDerived>
class WebSocketServer : public QWebSocketServer
{

public:

    WebSocketServer(const QString &serverName, QWebSocketServer::SslMode secureMode, QObject* parent = 0)
        : QWebSocketServer(serverName,secureMode, parent)
    {
        QObject::connect(this, &QWebSocketServer::newConnection, this, &WebSocketServer::handleNewConnection);


    }
private:
    void handleNewConnection()
    {
        auto incomingConnection = nextPendingConnection();

        if(!static_cast<TDerived*>(this)->accept(std::make_shared<WebSocketConnection>(incomingConnection)))
        {
            incomingConnection->close();
            incomingConnection->deleteLater();
        }
    }

};

}


struct WebSocket
{
    template<typename T>
    using Server = Private::WebSocketServer<T>;

    template<typename T>
    using Connection = Private::WebSocketConnection<T>;
};




}
}
}


#endif // GRLX_RPC_QTWEBSOCKET_H
