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

#ifndef QUICSOCKETTXPFIFOSCHEDULER_H
#define QUICSOCKETTXPFIFOSCHEDULER_H

#include "quic-socket-tx-scheduler.h"
#include <queue>
#include <vector>

namespace ns3
{

/**
 * \brief The PFIFO implementation
 *
 * This class is a Priority FIFO implementation of the socket scheduler, which prioritizes streams with a lower stream number
 */
class QuicSocketTxPFifoScheduler: public QuicSocketTxScheduler
{
public:
	/**
	 * \brief Get the type ID.
	 * \return the object TypeId
	 */
	static TypeId GetTypeId(void);

	QuicSocketTxPFifoScheduler();
	QuicSocketTxPFifoScheduler(const QuicSocketTxPFifoScheduler &other);
	~QuicSocketTxPFifoScheduler(void);

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
	struct PriorityTxItem
	{
		uint64_t streamId;
		uint64_t offset;
		Ptr<QuicSocketTxItem> item;
		PriorityTxItem(uint64_t id, uint64_t off, Ptr<QuicSocketTxItem> it) :
			streamId(id),
			offset(off),
			item(it)
			{
			}
	};
	struct ItemComp
	{
		bool operator()(PriorityTxItem const &it1, PriorityTxItem const &it2)
		{
			if (it1.streamId != it2.streamId)
				return it1.streamId > it2.streamId;
			else
				return it1.offset > it2.offset;
		}
	};
	typedef std::priority_queue<PriorityTxItem, std::vector<PriorityTxItem>, ItemComp> QuicTxPacketList; //!< container for data stored in the buffer

	QuicTxPacketList m_appList; //!< List of buffered application packets to be transmitted with additional info
	uint32_t m_appSize;            //!< Size of all data in the application list
	bool m_retxFirst;
};

} // namepsace ns3

#endif /* QUIC_SOCKET_TX_PFIFO_SCHEDULER_H */
