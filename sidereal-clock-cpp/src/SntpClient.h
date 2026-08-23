#pragma once
#include <QObject>
#include <QUdpSocket>
#include <QTimer>

// A real SNTP (RFC 4330) client over UDP — something the web version
// explicitly could NOT do (browsers can't open raw UDP sockets), kept as
// a "Sync now" fallback there. A native app can just ask a time server
// directly, so this queries one and reports the clock offset it implies.
class SntpClient : public QObject {
    Q_OBJECT
public:
    explicit SntpClient(QObject* parent = nullptr);
    void query(const QString& host, quint16 port = 123);

signals:
    void finished(bool ok, qint64 offsetMs, QString message);

private slots:
    void onReadyRead();
    void onTimeout();

private:
    void sendRequest(const QHostAddress& addr, quint16 port);
    void finish(bool ok, qint64 offsetMs, const QString& message);

    QUdpSocket socket_;
    QTimer timeoutTimer_;
    quint16 port_ = 123;
    qint64 t1Ms_ = 0; // local time request was sent (Unix ms)
    bool done_ = false;
};
