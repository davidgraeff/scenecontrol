/*
 *    RoomControlServer. Home automation for controlling sockets, leds and music.
 *    Copyright (C) 2010  David Gräff
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once
#include "abstractplugin.h"
#include <QStringList>
#include <QObject>
#include <QMap>
#include <QUdpSocket>
#include <QTimer>
#include <QVariantMap>
#include <stdint.h>



/* ----------------------------------------------------------------------------
 * op-codes
 */
#define OP_POLL			0x2000
#define OP_POLLREPLY		0x2100
#define OP_OUTPUT		0x5000
#define OP_ADDRESS		0x6000
#define OP_IPPROG		0xf800
#define OP_IPPROGREPLY		0xf900

/* ----------------------------------------------------------------------------
 * status
 */
#define RC_POWER_OK		0x01
#define RC_PARSE_FAIL		0x04
#define RC_SH_NAME_OK		0x06
#define RC_LO_NAME_OK		0x07

/* ----------------------------------------------------------------------------
 * default values
 */
#define SUBNET_DEFAULT		0
#define NETCONFIG_DEFAULT	1

/* ----------------------------------------------------------------------------
 * other defines
 */
#define SHORT_NAME_LENGTH	18
#define LONG_NAME_LENGTH	64
#define PORT_NAME_LENGTH	32
#define ARTNET_MAX_DATA_LENGTH  511
#define ARTNET_MAX_CHANNELS     512
#define ARTNET_MAX_PORTS	4
#define PROTOCOL_VERSION 	14      /* DMX-Hub protocol version. */
#define FIRMWARE_VERSION 	0x0100  /* DMX-Hub firmware version. */
#define STYLE_NODE 		0       /* Responder is a Node (DMX <-> Ethernet Device) */

#define PORT_TYPE_DMX_OUTPUT	0x80
#define PORT_TYPE_DMX_INPUT 	0x40

/* ----------------------------------------------------------------------------
 * packet formats
 */
struct artnet_packet_addr
{
	uint8_t ip[4];
	uint16_t port;
};

struct artnet_header
{
	uint8_t id[8];
	uint16_t opcode;
};

struct artnet_poll
{
	char id[8];
	uint16_t opcode;
	uint8_t versionH;
	uint8_t version;
	uint8_t talkToMe;
	uint8_t priority;
};

struct artnet_pollreply
{
	char id[8];
	uint16_t opcode;
	struct artnet_packet_addr addr;
	uint8_t versionInfoH;
	uint8_t versionInfo;
	uint8_t subSwitchH;
	uint8_t subSwitch;
	uint16_t oem;
	uint8_t ubeaVersion;
	uint8_t status;
	uint16_t estaMan;
	char shortName[SHORT_NAME_LENGTH];
	char longName[LONG_NAME_LENGTH];
	char nodeReport[LONG_NAME_LENGTH];
	uint8_t numPortsH;
	uint8_t numPorts;
	uint8_t portTypes[ARTNET_MAX_PORTS];
	uint8_t goodInput[ARTNET_MAX_PORTS];
	uint8_t goodOutput[ARTNET_MAX_PORTS];
	uint8_t swin[ARTNET_MAX_PORTS];
	uint8_t swout[ARTNET_MAX_PORTS];
	uint8_t swVideo;
	uint8_t swMacro;
	uint8_t swRemote;
	uint8_t spare1;
	uint8_t spare2;
	uint8_t spare3;
	uint8_t style;
	uint8_t mac[6];
	uint8_t filler[32];
};

struct artnet_ipprog
{
	char id[8];
	uint16_t opcode;
	uint8_t versionH;
	uint8_t version;
	uint8_t filler1;
	uint8_t filler2;
	uint8_t command;
	uint8_t filler3;
	uint8_t progIp[4];
	uint8_t progSm[4];
	uint8_t progPort[2];
	uint8_t spare[8];
};

struct artnet_ipprogreply
{
	char id[8];
	uint16_t opcode;
	uint8_t versionH;
	uint8_t version;
	uint8_t filler1;
	uint8_t filler2;
	uint8_t filler3;
	uint8_t filler4;
	uint8_t progIp[4];
	uint8_t progSm[4];
	uint8_t progPort[2];
	uint8_t spare[8];
};

struct artnet_address
{
	char id[8];
	uint16_t opcode;
	uint8_t versionH;
	uint8_t version;
	uint8_t filler1;
	uint8_t filler2;
	int8_t shortName[SHORT_NAME_LENGTH];
	int8_t longName[LONG_NAME_LENGTH];
	uint8_t swin[ARTNET_MAX_PORTS];
	uint8_t swout[ARTNET_MAX_PORTS];
	uint8_t subSwitch;
	uint8_t swVideo;
	uint8_t command;
};

struct artnet_dmx
{
	char id[8];
	uint16_t opcode;
	uint8_t versionH;
	uint8_t version;
	uint8_t sequence;
	uint8_t physical;
	uint16_t universe;
	uint8_t lengthHi;
	uint8_t length;
	uint8_t dataStart[];
};

const char artnet_ID[8] = "Art-Net";

class plugin : public AbstractPlugin
{
    Q_OBJECT
public:
    virtual ~plugin();

    virtual void initialize();
    virtual void clear();
    virtual void requestProperties();
    virtual void instanceConfiguration(const QVariantMap& data);
private:
    void ledChanged(QString channel, int value);

    struct ledchannel {
        int value;
        QString name;
        uint8_t channel;
		QHostAddress remote;
		uint16_t universe;
		ledchannel(uint8_t channel, int value,const QHostAddress& remote, uint16_t universe) {
            this->channel = channel;
            this->value = value;
			this->remote = remote;
			this->universe = universe;
        }
        ledchannel() {
            value = -1;
        }
    };
    QMap<QString,ledchannel> m_leds;

    int m_channels;
    // udp
    int m_sendPort;
    QUdpSocket *m_socket;
    int m_connectTime;
    QTimer m_connectTimer;
private Q_SLOTS:
    // LIGHTS //
    void readyRead();
    void resendConnectSequence();
public Q_SLOTS:
    int countChannels();
    void setLed ( const QString& channel, int value, int fade );
    void toggleLed(const QString& channel, int fade);
    void setLedExponential ( const QString& channel, int multiplikator, int fade );
    void setLedRelative ( const QString& channel, int value, int fade );
    int getLed(const QString& channel) const;
    bool isLedValue( const QString& channel, int lower, int upper );
};
