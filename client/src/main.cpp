#include <QApplication>
#include <iostream>
#include "server-connection.hpp"
#include "QRCodeWindow.h"

const std::string argument_error_message = ": Please provide the backends address, the backends port and optionally the backends CA-bundle (if it uses a self-signed certificate)\n\n./ticketmaster-but-worse <backend address> <backend port> [<path to backends CA-bundle file>]\nFor example: ./ticketmaster-but-worse localhost 8000 ./rootCA.crt";

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        std::cerr << "Wrong number of arguments" << argument_error_message << std::endl;
        return 1;
    }
    ServerConnection *conn;
    try {
        if (argc == 4) {
            conn = new ServerConnection(argv[1], std::stoi(argv[2]), argv[3]);
        }
        else {
            conn = new ServerConnection(argv[1], std::stoi(argv[2]));
        }
    }
    catch (const std::exception&) {
        std::cerr << "Exception occurred while processing arguments, maybe the provided port number is not an unsigned 16-bit integer" << argument_error_message << std::endl;
        return 1;
    }

    try {
        conn->login();
        std::vector<Ticket> tickets;
        conn->get_tickets(tickets);

        if (tickets.empty()) {
            std::cerr << "This user doesn't own any tickets" << std::endl;
            return 1;
        } else if (tickets.size() > 1) {
            std::cout << "This user owns multiple tickets. Displaying first one only" << std::endl;
        }

        Ticket ticket = *tickets.begin();

        QApplication app(argc, argv);

        // Initialisiere das Fenster mit dem QR-Code des aktuellen Tickets
        std::string ticketToken = ticket.get_ticket_token();  
        std::string totpSecret = ticket.get_totp_secret();

        QRCodeWindow window(QString::fromStdString(ticket.get_ticket_string()), ticketToken, totpSecret, conn);
        window.show();
        std::cout << "Ticket QR-Code string: " << ticket.get_ticket_string() << std::endl;

        // Timer, um alle 15 Sekunden das Ticket zu aktualisieren
        QTimer timer;
        QObject::connect(&timer, &QTimer::timeout, [&]() {
            std::string new_ticket_string = ticket.get_ticket_string();
            std::cout << "Updated ticket QR-Code string: " << new_ticket_string << std::endl;
            window.updateQRCode(QString::fromStdString(new_ticket_string));
        });
        timer.start(15000);  // Alle 15 Sekunden

        return app.exec();
    }
    catch (const CommunicationError &e) {
        std::cerr << "Error while communicating with server: " << e.get_error_msg() << std::endl;
        return 1;
    }
}
