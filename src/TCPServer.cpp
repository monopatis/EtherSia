#include "EtherSia.h"
#include "util.h"

TCPServer::TCPServer(EtherSia &ether, uint16_t localPort) : _ether(ether)
{
    _localPort = localPort;
}


uint16_t TCPServer::localPort()
{
    return _localPort;
}

boolean TCPServer::havePacket()
{
    IPv6Packet& packet = _ether.packet();

    if (!packet.isValid()) {
        return 0;
    }

    if (packet.protocol() != IP6_PROTO_TCP) {
        // Wrong protocol
        return 0;
    }

    if (packetDestinationPort() != _localPort) {
        // Wrong destination port
        return 0;
    }

    if (TCP_HEADER_PTR->flags & TCP_FLAG_RST) {
        return 0;
    }

    if (TCP_HEADER_PTR->flags & TCP_FLAG_FIN) {
        sendReplyWithFlags(0, TCP_FLAG_FIN | TCP_FLAG_ACK);
        return 0;
    }

    if (TCP_HEADER_PTR->flags & TCP_FLAG_SYN) {
        // Accept the connection
        sendReplyWithFlags(0, TCP_FLAG_SYN | TCP_FLAG_ACK);

        // We have handled the SYN
        return 0;
    }

    // Packet contains data that needs to be handled
    if (payloadLength() > 0) {
        return 1;
    } else { 
        return 0;
    }

    // Something else?
    return 0;
}


void TCPServer::sendReply(const char *string)
{
    sendReply((const void*)string, strlen(string));
}

void TCPServer::sendReply(const void* data, uint16_t len)
{
    IPv6Packet& packet = _ether.packet();
    struct tcp_header *tcpHeader = TCP_HEADER_PTR;

    if (data) {
        memcpy((uint8_t*)tcpHeader + TCP_HEADER_LEN, data, len);
    }

    // Acknowledge the request packet and send response
    sendReplyWithFlags(len, TCP_FLAG_ACK | TCP_FLAG_FIN | TCP_FLAG_PSH );
}

void TCPServer::sendReplyWithFlags(uint16_t len, uint8_t flags)
{
    IPv6Packet& packet = _ether.packet();
    struct tcp_header *tcpHeader = TCP_HEADER_PTR;

    uint32_t seq = ntohl(tcpHeader->acknowledgementNum);
    uint32_t ack = ntohl(tcpHeader->sequenceNum);
    uint16_t recievedLen = payloadLength();
    if (recievedLen == 0)
        recievedLen = 1;

    // FIXME: find more reliable way of setting this (when SYN received)
    if (seq == 0)
        seq = random();

    _ether.prepareReply();

    tcpHeader->flags = flags;
    tcpHeader->destinationPort = tcpHeader->sourcePort;
    tcpHeader->sourcePort = htons(_localPort);
    tcpHeader->sequenceNum = htonl(seq);
    tcpHeader->acknowledgementNum = htonl(ack + recievedLen);

    tcpHeader->dataOffset = (TCP_HEADER_LEN / 4) << 4;
    tcpHeader->window = htons(TCP_WINDOW_SIZE);
    tcpHeader->urgentPointer = 0;
    tcpHeader->mssOptionKind = 2;
    tcpHeader->mssOptionLen = 4;
    tcpHeader->mssOptionValue = tcpHeader->window;

    packet.setPayloadLength(TCP_HEADER_LEN + len);

    tcpHeader->checksum = 0;
    tcpHeader->checksum = htons(packet.calculateChecksum());

    _ether.send();
}

IPv6Address& TCPServer::packetSource()
{
    return _ether.packet().source();
}

IPv6Address& TCPServer::packetDestination()
{
    return _ether.packet().destination();
}

uint16_t TCPServer::packetSourcePort()
{
    IPv6Packet& packet = _ether.packet();
    return ntohs(TCP_HEADER_PTR->sourcePort);
}

uint16_t TCPServer::packetDestinationPort()
{
    IPv6Packet& packet = _ether.packet();
    return ntohs(TCP_HEADER_PTR->destinationPort);
}


uint8_t* TCPServer::payload()
{
    IPv6Packet& packet = _ether.packet();
    return (uint8_t *)(packet.payload()) + payloadLength();
}

uint16_t TCPServer::payloadLength()
{
    IPv6Packet& packet = _ether.packet();
    uint8_t headerLen = ((TCP_HEADER_PTR->dataOffset & 0xF0) >> 4) * 4;
    return packet.payloadLength() - headerLen;
}

boolean TCPServer::payloadEquals(const char *str)
{
    return strncmp((char*)payload(), str, payloadLength()) == 0;
}
