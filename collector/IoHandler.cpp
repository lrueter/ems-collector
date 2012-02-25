#include <iostream>
#include <iomanip>
#include "EmsMessage.h"
#include "IoHandler.h"
#include "Options.h"

IoHandler::IoHandler(Database& db) :
    boost::asio::io_service(),
    m_active(true),
    m_db(db),
    m_state(Syncing),
    m_pos(0)
{
    /* pre-alloc buffer to avoid reallocations */
    m_data.resize(256);
}

IoHandler::~IoHandler()
{
}

void
IoHandler::readComplete(const boost::system::error_code& error,
			size_t bytesTransferred)
{
    size_t pos = 0;
    DebugStream& debug = Options::ioDebug();

    if (error) {
	doClose(error);
	return;
    }

    if (debug) {
	debug << "IO: Got bytes ";
	for (size_t i = 0; i < bytesTransferred; i++) {
	    debug << std::setfill('0') << std::setw(2)
		  << std::showbase << std::hex
		  << (unsigned int) m_recvBuffer[i] << " ";
	}
	debug << std::endl;
    }

    while (pos < bytesTransferred) {
	unsigned char dataByte = m_recvBuffer[pos++];

	switch (m_state) {
	    case Syncing:
		if (m_pos == 0 && dataByte == 0xaa) {
		    m_pos = 1;
		} else if (m_pos == 1 && dataByte == 0x55) {
		    m_state = Length;
		    m_pos = 0;
		} else {
		    m_pos = 0;
		}
		break;
	    case Length:
		m_state = Data;
		m_pos = 0;
		m_length = dataByte;
		m_checkSum = 0;
		break;
	    case Data:
		m_data[m_pos++] = dataByte;
		m_checkSum ^= dataByte;
		if (m_pos == m_length) {
		    m_state = Checksum;
		}
		break;
	    case Checksum:
		if (m_checkSum == dataByte) {
		    EmsMessage message(m_db, m_data);
		    message.handle();
		}
		m_state = Syncing;
		break;
	}
    }

    readStart();
}

void
IoHandler::doClose(const boost::system::error_code& error)
{
    if (error == boost::asio::error::operation_aborted) {
	/* if this call is the result of a timer cancel() */
	return; // ignore it because the connection cancelled the timer
    }

    if (error) {
	std::cerr << "Error: " << error.message() << std::endl;
    }

    doCloseImpl();
    m_active = false;
}
