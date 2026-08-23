#include "SntpClient.h"
#include <QHostInfo>
#include <QDateTime>
#include <cstring>

// Seconds between the NTP epoch (1900-01-01) and the Unix epoch (1970-01-01).
static const qint64 NTP_UNIX_EPOCH_DELTA = 2208988800LL;

SntpClient::SntpClient(QObject* parent) : QObject(parent) {
    connect(&socket_, &QUdpSocket::readyRead, this, &SntpClient::onReadyRead);
    timeoutTimer_.setSingleShot(true);
    connect(&timeoutTimer_, &QTimer::timeout, this, &SntpClient::onTimeout);
}

void SntpClient::query(const QString& host, quint16 port) {
    done_ = false;
    port_ = port;
    timeoutTimer_.start(5000);
    QHostInfo::lookupHost(host, this, [this](const QHostInfo& info) {
        if (done_) return;
        if (info.error() != QHostInfo::NoError || info.addresses().isEmpty()) {
            finish(false, 0, "Could not resolve " + info.hostName());
            return;
        }
        sendRequest(info.addresses().first(), port_);
    });
}

void SntpClient::sendRequest(const QHostAddress& addr, quint16 port) {
    if (done_) return;
    quint8 packet[48] = {0};
    packet[0] = 0x23; // LI=0, VN=4, Mode=3 (client)
    t1Ms_ = QDateTime::currentMSecsSinceEpoch();
    socket_.writeDatagram(reinterpret_cast<const char*>(packet), sizeof(packet), addr, port);
}

void SntpClient::onReadyRead() {
    while (socket_.hasPendingDatagrams()) {
        QByteArray buf;
        buf.resize(int(socket_.pendingDatagramSize()));
        socket_.readDatagram(buf.data(), buf.size());
        if (done_) continue;
        if (buf.size() < 48) continue;

        auto be32 = [&](int offset) -> quint32 {
            return (quint8(buf[offset]) << 24) | (quint8(buf[offset+1]) << 16) |
                   (quint8(buf[offset+2]) << 8) | quint8(buf[offset+3]);
        };

        // Transmit Timestamp: seconds since 1900 (bytes 40-43) + fraction (44-47)
        quint32 txSec = be32(40);
        quint32 txFrac = be32(44);
        qint64 t3Ms = (qint64(txSec) - NTP_UNIX_EPOCH_DELTA) * 1000LL
                      + (qint64(txFrac) * 1000LL) / 4294967296LL;

        qint64 t4Ms = QDateTime::currentMSecsSinceEpoch();
        // Simplified (single-timestamp) offset estimate: server transmit time
        // vs. the midpoint of our round trip. Good enough for a wall clock —
        // not claiming NTP's full 4-timestamp precision here.
        qint64 roundTrip = t4Ms - t1Ms_;
        qint64 assumedServerNowAtArrival = t3Ms + roundTrip / 2;
        qint64 offsetMs = assumedServerNowAtArrival - t4Ms;

        finish(true, offsetMs, QString("Round trip %1 ms").arg(roundTrip));
        return;
    }
}

void SntpClient::onTimeout() {
    finish(false, 0, "Timed out waiting for a response.");
}

void SntpClient::finish(bool ok, qint64 offsetMs, const QString& message) {
    if (done_) return;
    done_ = true;
    timeoutTimer_.stop();
    emit finished(ok, offsetMs, message);
}
