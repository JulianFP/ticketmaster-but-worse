#include "QRCodeWindow.h"
#include <QVBoxLayout>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QFont>
#include <qrencode.h>
#include <iostream>

QRCodeWindow::QRCodeWindow(const QString &text, ServerConnection *conn, QWidget *parent)
    : QWidget(parent), qrCodeLabel(new QLabel(this)), statusLabel(new QLabel(this)),
      checkButton(new QPushButton("Check Ticket Validity", this)), conn(conn) {

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(qrCodeLabel);
    layout->addWidget(statusLabel);  // Label für Ticket-Status
    layout->addWidget(checkButton);  // Button zum Überprüfen der Gültigkeit

    generateQRCode(text);

    connect(checkButton, &QPushButton::clicked, this, &QRCodeWindow::onCheckButtonClicked);

    setLayout(layout);
    setWindowTitle("QR Code Viewer");

    resize(400, 400);
}

void QRCodeWindow::generateQRCode(const QString &text) {
    QRcode *qr = QRcode_encodeString(text.toUtf8().constData(), 0, QR_ECLEVEL_Q, QR_MODE_8, 1);

    if (qr) {
        int size = qr->width > 0 ? qr->width : 1;
        QImage image(size, size, QImage::Format_RGB32);
        image.fill(Qt::white);

        QPainter painter(&image);
        painter.setBrush(Qt::black);

        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                if (qr->data[y * size + x] & 0x01) {
                    painter.drawRect(x, y, 1, 1);
                }
            }
        }

        qrCodeLabel->setPixmap(QPixmap::fromImage(image).scaled(350, 350, Qt::KeepAspectRatio));
        QRcode_free(qr);
    } else {
        qrCodeLabel->setText("Failed to generate QR code");
    }
}

void QRCodeWindow::onCheckButtonClicked() {
    checkTicketValidity();  // Direkt im Fenster die Ticketprüfung durchführen
}

void QRCodeWindow::checkTicketValidity() {
    bool isValid = conn->check_ticket_validity(ticket);
    if (isValid) {
        updateStatus("Ticket is valid");
    } else {
        updateStatus("Ticket is not valid");
    }
}

void QRCodeWindow::updateStatus(const QString &status) {
    statusLabel->setText(status);  // Zeigt den Status im Fenster an
}
