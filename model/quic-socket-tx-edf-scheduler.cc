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

#include "quic-socket-tx-edf-scheduler.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include "ns3/simulator.h"

#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/abort.h"
#include "quic-subheader.h"
#include "quic-socket-base.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("QuicSocketTxEdfScheduler");

NS_OBJECT_ENSURE_REGISTERED(QuicSocketTxEdfScheduler);

TypeId QuicSocketTxEdfScheduler::GetTypeId(void) {
	static TypeId tid = TypeId("ns3::QuicSocketTxEdfScheduler").SetParent<
			QuicSocketTxScheduler>().SetGroupName("Internet").AddConstructor<
			QuicSocketTxEdfScheduler>().AddAttribute("RetxFirst",
			"Prioritize retransmissions regardless of stream",
			BooleanValue(false),
			MakeBooleanAccessor(&QuicSocketTxEdfScheduler::m_retxFirst),
			MakeBooleanChecker());
	return tid;
}

QuicSocketTxEdfScheduler::QuicSocketTxEdfScheduler() :
		QuicSocketTxScheduler(), m_appSize(0), m_retxFirst(false) {
	m_appList = QuicTxPacketList();
	m_defaultLatency = Seconds(0.1);
}

QuicSocketTxEdfScheduler::QuicSocketTxEdfScheduler(
		const QuicSocketTxEdfScheduler &other) :
		QuicSocketTxScheduler(other), m_appSize(other.m_appSize), m_retxFirst(
				other.m_retxFirst) {
	m_appList = other.m_appList;
	m_defaultLatency = other.m_defaultLatency;
	m_latencyMap = other.m_latencyMap;
}

QuicSocketTxEdfScheduler::~QuicSocketTxEdfScheduler(void) {
	m_appList = QuicTxPacketList();
	m_appSize = 0;
}

void QuicSocketTxEdfScheduler::Print(std::ostream &os) const {
	NS_LOG_FUNCTION(this);
	QuicTxPacketList temp = QuicTxPacketList(m_appList);
	std::stringstream as;

	while (temp.size() > 0) {
		PriorityTxItem pItem = temp.top();
		pItem.item->Print(as);
		temp.pop();
	}

	os << Simulator::Now().GetSeconds() << "\nApp list: \n" << as.str()
			<< "\n\nCurrent Status: \nApplication Size = " << m_appSize;
}

void QuicSocketTxEdfScheduler::Add(Ptr<QuicSocketTxItem> item, bool retx) {
	NS_LOG_FUNCTION(this << item);

	if (retx) {
		if (m_retxFirst) {
			NS_LOG_INFO("Adding retransmitted packet with highest priority");
			m_appList.push(PriorityTxItem(Seconds(0), item));
			m_appSize += item->m_packet->GetSize();
		} else {
			uint32_t dataSizeByte = item->m_packet->GetSize();
			QuicSubheader sub;
			item->m_packet->PeekHeader(sub);
			if (sub.GetSerializedSize() + sub.GetLength()
					< item->m_packet->GetSize()) {

				NS_LOG_INFO(
						"Disgregate packet to be retransmitted" << dataSizeByte);
				//data->Print(std::cout);

				// the packet could contain multiple frames
				// each of them starts with a subheader
				// cycle through the data packet and extract the frames
				for (uint32_t start = 0; start < dataSizeByte;) {
					item->m_packet->RemoveHeader(sub);
					NS_LOG_INFO(
							"subheader " << sub << " dataSizeByte " << dataSizeByte << " remaining " << item->m_packet->GetSize () << " frame size " << sub.GetLength ());
					Ptr<Packet> nextFragment = item->m_packet->Copy();
					nextFragment->RemoveAtEnd(
							nextFragment->GetSize() - sub.GetLength());
					NS_LOG_INFO("fragment size " << nextFragment->GetSize());

					// remove the first portion of the packet

					item->m_packet->RemoveAtStart(sub.GetLength());
					nextFragment->AddHeader(sub);
					start += nextFragment->GetSize();
					Ptr<QuicSocketTxItem> it = CreateObject<QuicSocketTxItem>(
							*item);
					uint64_t streamId = sub.GetStreamId();
					uint64_t offset = sub.GetOffset();
					it->m_packet = nextFragment;
					NS_LOG_INFO(
							"Added retx fragment on stream " << streamId << " with offset " << offset << " and length " << it->m_packet->GetSize() << ", pointer " << GetPointer(it->m_packet));
					PriorityTxItem pItem = PriorityTxItem(GetDeadline(it), it);
					m_appList.push(pItem);
					m_appSize += it->m_packet->GetSize();
				}
			} else {
				NS_LOG_INFO(
						"Added retx packet on stream " << sub.GetStreamId() << " with offset " << sub.GetOffset());
				m_appList.push(PriorityTxItem(GetDeadline(item), item));
				m_appSize += item->m_packet->GetSize();
			}
		}
	} else {
		QuicSubheader sub;
		item->m_packet->PeekHeader(sub);
		NS_LOG_INFO(
				"Added packet on stream " << sub.GetStreamId() << " with offset " << sub.GetOffset());
		m_appList.push(PriorityTxItem(GetDeadline(item), item));
		m_appSize += item->m_packet->GetSize();
	}
}

Ptr<QuicSocketTxItem> QuicSocketTxEdfScheduler::GetNewSegment(
		uint32_t numBytes) {
	NS_LOG_FUNCTION(this << numBytes);

	bool firstSegment = true;
	Ptr<Packet> currentPacket = 0;
	Ptr<QuicSocketTxItem> currentItem = 0;
	Ptr<QuicSocketTxItem> outItem = CreateObject<QuicSocketTxItem>();
	outItem->m_isStream = true; // Packets sent with this method are always stream packets
	outItem->m_isStream0 = false;
	outItem->m_packet = Create<Packet>();
	uint32_t outItemSize = 0;

	while (m_appList.size() > 0 && outItemSize < numBytes) {
		PriorityTxItem currentPriorityItem = m_appList.top();
		currentItem = currentPriorityItem.item;
		currentPacket = currentItem->m_packet;

		NS_LOG_INFO(currentItem << " "<<GetPointer(currentPacket));

		if (outItemSize + currentItem->m_packet->GetSize() /*- subheaderSize*/
		<= numBytes)       // Merge
				{
			NS_LOG_LOGIC(
					"Add complete frame to the outItem - size " << currentItem->m_packet->GetSize () << " m_appSize " << m_appSize);
			QuicSubheader qsb;
			currentPacket->PeekHeader(qsb);
			NS_LOG_INFO(
					"Packet: stream " <<qsb.GetStreamId() << ", offset "<<qsb.GetOffset());
			QuicSocketTxItem::MergeItems(*outItem, *currentItem);
			outItemSize += currentItem->m_packet->GetSize();

			m_appList.pop();
			m_appSize -= currentItem->m_packet->GetSize();

			NS_LOG_LOGIC("Updating application buffer size: " << m_appSize);
			continue;
		} else if (firstSegment) // we cannot transmit a full packet, so let's split it and update the subheaders
		{
			firstSegment = false;

			// subtract the whole packet from m_appSize, then add the remaining fragment (need to account for headers)
			uint32_t removed = currentItem->m_packet->GetSize();
			m_appSize -= removed;

			// get the currentPacket subheader
			QuicSubheader qsb;
			currentPacket->PeekHeader(qsb);

			// new packet size
			int newPacketSizeInt = (int) numBytes - outItemSize
					- qsb.GetSerializedSize();
			if (newPacketSizeInt <= 0) {
				NS_LOG_INFO("Not enough bytes even for the header");
				m_appSize += removed;
				break;
			}
			NS_LOG_INFO(
					"Split packet on stream " <<qsb.GetStreamId() << ", sending " << newPacketSizeInt << " bytes from offset "<<qsb.GetOffset());

			currentPacket->RemoveHeader(qsb);
			uint32_t newPacketSize = (uint32_t) newPacketSizeInt;

			NS_LOG_LOGIC("Add incomplete frame to the outItem");
			uint32_t totPacketSize = currentItem->m_packet->GetSize();
			NS_LOG_LOGIC("Extracted " << outItemSize << " bytes");

			uint32_t oldOffset = qsb.GetOffset();
			uint32_t newOffset = oldOffset + newPacketSize;
			bool oldOffBit = !(oldOffset == 0);
			bool newOffBit = true;
			uint32_t oldLength = qsb.GetLength();
			uint32_t newLength = 0;
			bool newLengthBit = true;
			newLength = totPacketSize - newPacketSize;
			if (oldLength == 0) {
				newLengthBit = false;
			}
			bool lengthBit = true;
			bool oldFinBit = qsb.IsStreamFin();
			bool newFinBit = false;

			QuicSubheader newQsbToTx = QuicSubheader::CreateStreamSubHeader(
					qsb.GetStreamId(), oldOffset, newPacketSize, oldOffBit,
					lengthBit, newFinBit);
			QuicSubheader newQsbToBuffer = QuicSubheader::CreateStreamSubHeader(
					qsb.GetStreamId(), newOffset, newLength, newOffBit,
					newLengthBit, oldFinBit);

			Ptr<Packet> firstPartPacket = currentItem->m_packet->CreateFragment(
					0, newPacketSize);
			NS_ASSERT_MSG(firstPartPacket->GetSize() == newPacketSize,
					"Wrong size " << firstPartPacket->GetSize ());
			firstPartPacket->AddHeader(newQsbToTx);
			firstPartPacket->Print(std::cerr);

			NS_LOG_INFO(
					"Split packet, putting second part back in application buffer - stream " <<newQsbToBuffer.GetStreamId() << ", storing from offset "<<newQsbToBuffer.GetOffset());

			Ptr<Packet> secondPartPacket =
					currentItem->m_packet->CreateFragment(newPacketSize,
							newLength);
			secondPartPacket->AddHeader(newQsbToBuffer);

			QuicSocketTxItem *toBeBuffered = new QuicSocketTxItem(*currentItem);
			toBeBuffered->m_packet = secondPartPacket;
			currentItem->m_packet = firstPartPacket;

			QuicSocketTxItem::MergeItems(*outItem, *currentItem);
			outItemSize += currentItem->m_packet->GetSize();

			m_appList.pop();
			PriorityTxItem bufferedPriorityItem = PriorityTxItem(
					currentPriorityItem.deadline, toBeBuffered);
			m_appList.push(bufferedPriorityItem);
			m_appSize += toBeBuffered->m_packet->GetSize();
			// check correctness of application size
			uint32_t check = m_appSize;
			QuicTxPacketList temp = QuicTxPacketList(m_appList);
			while (temp.size() > 0) {
				PriorityTxItem pItem = temp.top();
				check -= pItem.item->m_packet->GetSize();
				temp.pop();
			}
			NS_ASSERT(check == 0);

			NS_LOG_LOGIC(
					"Buffer size: " << m_appSize << " (put back " << toBeBuffered->m_packet->GetSize () << " bytes)");
			break; // at most one segment
		}
	}

	NS_LOG_INFO(
			"Update: remaining App Size " << m_appSize << ", object size " << outItemSize);

	//Print(std::cout);

	return outItem;
}

void QuicSocketTxEdfScheduler::SetLatency(uint32_t streamId, Time latency) {
	m_latencyMap[streamId] = latency;
}

Time QuicSocketTxEdfScheduler::GetLatency(uint32_t streamId) {
	Time latency = m_defaultLatency;
	try {
		latency = m_latencyMap.at(streamId);
	} catch (int e) {
		NS_LOG_INFO(
				"Stream " << streamId <<" does not have a pre-specified latency, using default");
	}
	return latency;
}

void QuicSocketTxEdfScheduler::SetDefaultLatency(Time latency) {
	m_defaultLatency = latency;
}

Time QuicSocketTxEdfScheduler::GetDefaultLatency() {
	return m_defaultLatency;
}

uint32_t QuicSocketTxEdfScheduler::AppSize(void) const {
	return m_appSize;
}

Time QuicSocketTxEdfScheduler::GetDeadline(Ptr<QuicSocketTxItem> item) {
	Ptr<Packet> packet = item->m_packet;
	QuicSubheader sub;
	packet->PeekHeader(sub);
	return item->m_generated + GetLatency(sub.GetStreamId());
}

}
