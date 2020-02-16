/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 SIGNET Lab, Department of Information Engineering, University of Padova
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Alvise De Biasio <alvise.debiasio@gmail.com>
 *          Federico Chiariotti <chiariotti.federico@gmail.com>
 *          Michele Polese <michele.polese@gmail.com>
 *          Davide Marcato <davidemarcato@outlook.com>
 *          
 */

#ifndef QUICSOCKETTXSCHEDULER_H
#define QUICSOCKETTXSCHEDULER_H

#include "quic-socket-tx-buffer.h"

namespace ns3
{

class QuicSocketTxItem;

/**
 * \ingroup quic
 *
 * \brief Tx socket buffer for QUIC
 */
class QuicSocketTxScheduler: public Object
{
public:
	/**
	 * \brief Get the type ID.
	 * \return the object TypeId
	 */
	static TypeId GetTypeId(void);

	QuicSocketTxScheduler();
	QuicSocketTxScheduler(const QuicSocketTxScheduler &other);
	~QuicSocketTxScheduler(void); /**
	 * Print the scheduler buffer information to a string
	 *
	 * \param os the std::ostream object
	 */
	virtual void Print(std::ostream &os) const
	{
		NS_UNUSED(os);
	}

	//friend std::ostream & operator<< (std::ostream & os, QuicSocketTxBuffer const & quicTxBuf);

	/**
	 * Add a tx item to the scheduling list
	 *
	 * \param item a smart pointer to a transmission item
	 */
	virtual void Add(Ptr<QuicSocketTxItem> item, bool retx)
	{
		NS_UNUSED(item);
		NS_UNUSED(retx);
	}

	/**
	 * \brief Get the next scheduled packet with a specified size
	 *
	 * \param numBytes number of bytes of the QuicSocketTxItem requested
	 * \return the item that contains the right packet
	 */
	virtual Ptr<QuicSocketTxItem> GetNewSegment(uint32_t numBytes)
	{
		NS_UNUSED(numBytes);
		return nullptr;
	}

	/**
	 * Returns the total number of bytes in the application buffer
	 *
	 * \return the total number of bytes in the application buffer
	 */
	virtual uint32_t AppSize(void) const = 0;

};

/**
 * \brief The FIFO implementation
 *
 * This class is a simple FIFO implementation of the socket scheduler, which treats all streams the same
 */
class QuicSocketTxFifoScheduler: public QuicSocketTxScheduler
{
public:
	/**
	 * \brief Get the type ID.
	 * \return the object TypeId
	 */
	static TypeId GetTypeId(void);

	QuicSocketTxFifoScheduler();
	QuicSocketTxFifoScheduler(const QuicSocketTxFifoScheduler &other);
	~QuicSocketTxFifoScheduler(void);

	/**
	 * Print the scheduler buffer information to a string
	 *
	 * \param os the std::ostream object
	 */
	void Print(std::ostream &os) const;

	void Add(Ptr<QuicSocketTxItem> item, bool retx);
	Ptr<QuicSocketTxItem> GetNewSegment(uint32_t numBytes);
	uint32_t AppSize(void) const;

private:
	typedef std::list<Ptr<QuicSocketTxItem>> QuicTxPacketList; //!< container for data stored in the buffer
	QuicTxPacketList m_appList; //!< List of buffered application packets to be transmitted with additional info
	uint32_t m_appSize;            //!< Size of all data in the application list
};

} // namepsace ns3

#endif /* QUIC_SOCKET_TX_SCHEDULER_H */
