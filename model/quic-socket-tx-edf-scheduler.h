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

#ifndef QUICSOCKETTXEDFSCHED_H
#define QUICSOCKETTXEDFSCHED_H

#include "quic-socket-tx-scheduler.h"
#include <queue>
#include <map>
#include <vector>

namespace ns3
{

/**
 * \brief The EDF implementation
 *
 * This class is an Earliest Deadline First implementation of the socket scheduler, which prioritizes the packet with the earliest deadline
 */
class QuicSocketTxEdfScheduler: public QuicSocketTxScheduler
{
public:
	/**
	 * \brief Get the type ID.
	 * \return the object TypeId
	 */
	static TypeId GetTypeId(void);

	QuicSocketTxEdfScheduler();
	QuicSocketTxEdfScheduler(const QuicSocketTxEdfScheduler &other);
	~QuicSocketTxEdfScheduler(void);

	/**
	 * Print the scheduler buffer information to a string
	 *
	 * \param os the std::ostream object
	 */
	void Print(std::ostream &os) const;

	void Add(Ptr<QuicSocketTxItem> item, bool retx);
	Ptr<QuicSocketTxItem> GetNewSegment(uint32_t numBytes);
	uint32_t AppSize(void) const;

	/**
	 * Set the latency bound for a specified stream
	 *
	 * \param streamId The stream ID
	 * \param latency The stream's maximum latency
	 */
	void SetLatency(uint32_t streamId, Time latency);

	/**
	 * Get the latency bound for a specified stream
	 *
	 * \param streamId The stream ID
	 * \return The stream's maximum latency, or 0 if the stream is not registered
	 */
	Time GetLatency(uint32_t streamId);

	/**
	 * Set the default latency bound
	 *
	 * \param latency The default maximum latency
	 */
	void SetDefaultLatency(Time latency);

	/**
	 * Get the default latency bound
	 *
	 * \param streamId The stream ID
	 * \return The default maximum latency
	 */
	Time GetDefaultLatency();

private:

	struct PriorityTxItem
	{
		Time deadline;
		Ptr<QuicSocketTxItem> item;
		PriorityTxItem(Time dl, Ptr<QuicSocketTxItem> it) :
			deadline(dl),
			item(it)
			{
			}
	};
	struct ItemComp
	{
		bool operator()(PriorityTxItem const &it1, PriorityTxItem const &it2)
		{
			return it1.deadline < it2.deadline;
		}
	};
	typedef std::priority_queue<PriorityTxItem, std::vector<PriorityTxItem>, ItemComp> QuicTxPacketList; //!< container for data stored in the buffer

	/**
	 * Gets the deadline for a transmission item
	 * \param item The pointer to the item
	 */
	Time GetDeadline(Ptr<QuicSocketTxItem> item);

	QuicTxPacketList m_appList; //!< List of buffered application packets to be transmitted with additional info
	uint32_t m_appSize;            //!< Size of all data in the application list
	bool m_retxFirst;
	Time m_defaultLatency;
	std::map<uint32_t, Time> m_latencyMap;
};

} // namepsace ns3

#endif /* QUIC_SOCKET_TX_EDF_SCHEDULER_H */
