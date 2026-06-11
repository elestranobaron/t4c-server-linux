#include "Socket.h"

#ifndef _WIN32
#include <cstring>
#else
#include <conio.h>
#include <stdio.h>
#include <iostream>
#include <assert.h>
#endif

NMSocket::NMSocket()
:	m_hSocket(INVALID_SOCKET)
{
}

NMSocket::~NMSocket()
{
	DeInit();
}

int NMSocket::Init(const SOCKET& rSocketHandle)
{
#ifdef _WIN32
	WSAEVENT hEventSend = WSACreateEvent();
	WSAEVENT hEventRecv = WSACreateEvent();
	if (hEventSend == NULL ||
		hEventRecv == NULL)
	{
		WSACloseEvent(hEventSend);
		WSACloseEvent(hEventRecv);
		return eERROR_INITIALIZATION;
	}

	memset(&m_OverlappedRecv, 0, sizeof(m_OverlappedRecv));
	m_OverlappedRecv.hEvent = hEventRecv;
	memset(&m_OverlappedSend, 0, sizeof(m_OverlappedSend));
	m_OverlappedSend.hEvent = hEventSend;
#endif

	m_hSocket = rSocketHandle;
	return eNO_ERROR;
}

int NMSocket::DeInit()
{
#ifdef _WIN32
	if (m_OverlappedRecv.hEvent) {
		WSACloseEvent(m_OverlappedRecv.hEvent);
		m_OverlappedRecv.hEvent = NULL;
	}
	if (m_OverlappedSend.hEvent) {
		WSACloseEvent(m_OverlappedSend.hEvent);
		m_OverlappedSend.hEvent = NULL;
	}
#endif
	return eNO_ERROR;
}
