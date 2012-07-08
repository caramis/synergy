/*
 * synergy -- mouse and keyboard sharing utility
 * Copyright (C) 2012 Nick Bolton
 * 
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file COPYING that should have accompanied this file.
 * 
 * This package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "CIpcClient.h"
#include "Ipc.h"
#include "CIpcServerProxy.h"
#include "TMethodEventJob.h"
#include "CIpcMessage.h"

CEvent::Type			CIpcClient::s_connectedEvent = CEvent::kUnknown;

CIpcClient::CIpcClient() :
m_serverAddress(CNetworkAddress(IPC_HOST, IPC_PORT)),
m_server(nullptr)
{
	m_serverAddress.resolve();

	EVENTQUEUE->adoptHandler(
		m_socket.getConnectedEvent(), m_socket.getEventTarget(),
		new TMethodEventJob<CIpcClient>(
		this, &CIpcClient::handleConnected));
}

CIpcClient::~CIpcClient()
{
	delete m_server;
}

void
CIpcClient::connect()
{
	m_socket.connect(m_serverAddress);
	m_server = new CIpcServerProxy(m_socket);
}

void
CIpcClient::send(const CIpcMessage& message)
{
	assert(m_server != NULL);
	m_server->send(message);
}

CEvent::Type
CIpcClient::getConnectedEvent()
{
	return EVENTQUEUE->registerTypeOnce(
		s_connectedEvent, "CIpcClient::connected");
}

void
CIpcClient::handleConnected(const CEvent&, void*)
{
	EVENTQUEUE->addEvent(CEvent(getConnectedEvent(), this, m_server, CEvent::kDontFreeData));

	CIpcMessage message;
	message.m_type = kIpcHello;
	message.m_data = new UInt8(kIpcClientNode);
	send(message);
}