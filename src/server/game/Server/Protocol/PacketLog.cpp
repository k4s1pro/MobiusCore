/**
 * This file is part of the MobiusCore project.
 * See AUTHORS file for copyright information.
 */

#include "PacketLog.h"
#include "Config.h"
#include "IpAddress.h"
#include "Realm.h"
#include "Timer.h"
#include "World.h"
#include "WorldPacket.h"

#pragma pack(push, 1)

// Packet logging structures in PKT 3.1 format
struct LogHeader
{
    char Signature[3];
    uint16 FormatVersion;
    uint8 SnifferId;
    uint32 Build;
    char Locale[4];
    uint8 SessionKey[40];
    uint32 SniffStartUnixtime;
    uint32 SniffStartTicks;
    uint32 OptionalDataSize;
};

struct PacketHeader
{
    // used to uniquely identify a connection
    struct OptionalData
    {
        uint8 SocketIPBytes[16];
        uint32 SocketPort;
    };

    uint32 Direction;
    uint32 ConnectionId;
    uint32 ArrivalTicks;
    uint32 OptionalDataSize;
    uint32 Length;
    OptionalData OptionalData;
    uint32 Opcode;
};

#pragma pack(pop)

PacketLog::PacketLog() : _file(NULL)
{
    std::call_once(_initializeFlag, &PacketLog::Initialize, this);
}

PacketLog::~PacketLog()
{
    if (_file)
        fclose(_file);

    _file = NULL;
}

PacketLog* PacketLog::instance()
{
    static PacketLog instance;
    return &instance;
}

void PacketLog::Initialize()
{
    std::string logsDir = sConfigMgr->GetStringDefault("LogsDir", "");

    if (!logsDir.empty())
        if ((logsDir.at(logsDir.length() - 1) != '/') && (logsDir.at(logsDir.length() - 1) != '\\'))
            logsDir.push_back('/');

    std::string logname = sConfigMgr->GetStringDefault("PacketLogFile", "");
    if (!logname.empty())
    {
        _file = fopen((logsDir + logname).c_str(), "wb");

        if (CanLogPacket())
        {
            LogHeader header;
            header.Signature[0] = 'P'; header.Signature[1] = 'K'; header.Signature[2] = 'T';
            header.FormatVersion = 0x0301;
            header.SnifferId = 'T';
            header.Build = realm.Build;
            header.Locale[0] = 'e'; header.Locale[1] = 'n'; header.Locale[2] = 'U'; header.Locale[3] = 'S';
            std::memset(header.SessionKey, 0, sizeof(header.SessionKey));
            header.SniffStartUnixtime = time(NULL);
            header.SniffStartTicks = getMSTime();
            header.OptionalDataSize = 0;

            fwrite(&header, sizeof(header), 1, _file);
        }
    }
}

void PacketLog::LogPacket(WorldPacket const& packet, Direction direction, boost::asio::ip::address const& addr, uint16 port, ConnectionType connectionType)
{
    std::lock_guard<std::mutex> lock(_logPacketLock);

    PacketHeader header;
    header.Direction = direction == CLIENT_TO_SERVER ? 0x47534d43 : 0x47534d53;
    header.ConnectionId = connectionType;
    header.ArrivalTicks = getMSTime();

    header.OptionalDataSize = sizeof(header.OptionalData);
    memset(header.OptionalData.SocketIPBytes, 0, sizeof(header.OptionalData.SocketIPBytes));
    if (addr.is_v4())
    {
        auto bytes = addr.to_v4().to_bytes();
        memcpy(header.OptionalData.SocketIPBytes, bytes.data(), bytes.size());
    }
    else if (addr.is_v6())
    {
        auto bytes = addr.to_v6().to_bytes();
        memcpy(header.OptionalData.SocketIPBytes, bytes.data(), bytes.size());
    }

    header.OptionalData.SocketPort = port;
    std::size_t size = packet.size();
    if (direction == CLIENT_TO_SERVER)
        size -= 2;

    header.Length = size + sizeof(header.Opcode);
    header.Opcode = packet.GetOpcode();

    fwrite(&header, sizeof(header), 1, _file);
    if (size)
        fwrite(packet.contents(), 1, packet.size(), _file);

    fflush(_file);
}
