#include "amneziaWireGuardProtocol.h"

AmneziaWireGuardProtocol::AmneziaWireGuardProtocol(const QJsonObject &configuration, QObject *parent)
    : WireguardProtocol(configuration, parent)
{
}

AmneziaWireGuardProtocol::~AmneziaWireGuardProtocol()
{
}
