/*
 * Buderus EMS data collector
 *
 * Copyright (C) 2011 Danny Baumann <dannybaumann@web.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __INCOMINGMESSAGEHANDLER_H__
#define __INCOMINGMESSAGEHANDLER_H__

#include <list>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/function.hpp>
#include "EmsMessage.h"
#include "ValueCache.h"

class IncomingMessageHandler
{
    public:
	typedef std::function<void (const EmsValue& value)> ValueCallback;

    public:
	IncomingMessageHandler(ValueCache& cache);

	void addValueCallback(ValueCallback& cb) {
	    m_valueCallbacks.push_back(cb);
	}

	void handleIncomingMessage(const std::vector<uint8_t>& data);
	virtual void onPcMessageReceived(const EmsMessage& /* message */) {}

    private:
	void handleValue(const EmsValue& value);

	std::list<ValueCallback> m_valueCallbacks;
	EmsMessage::ValueHandler m_valueCb;
	EmsMessage::CacheAccessor m_cacheCb;
};

#endif /* __INCOMINGMESSAGEANDLER_H__ */
