/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2020 SIGNET Lab, Department of Information Engineering, University of Padova
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
 * Authors: Federico Chiariotti <chiariotti.federico@gmail.com>
 *          Michele Polese <michele.polese@gmail.com>
 *          Umberto Paro <umberto.paro@me.com>
 *
 */

#ifndef QUICSOCKETTXSCHEDULER_H
#define QUICSOCKETTXSCHEDULER_H

#include "quic-socket.h"
#include <queue>
#include <vector>

namespace ns3 {

class QuicSocketTxItem;

/**
 * \ingroup quic
 *
 * \brief Tx item for QUIC with priority
 */
class QuicSocketTxScheduleItem : public Object
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  QuicSocketTxScheduleItem (uint64_t id, uint64_t off, double p, Ptr<QuicSocketTxItem> it);
  QuicSocketTxScheduleItem (const QuicSocketTxScheduleItem &other);

  /**
   *  Compare \p this to another QuicSocketTxScheduleItem
   *
   *  \param [in] o The other item
   *  \return -1,0,+1 if `this < o`, `this == o`, or `this > o`
   */
  int Compare (const QuicSocketTxScheduleItem & o) const;

  inline bool operator < (const QuicSocketTxScheduleItem& other) const
  {
    return Compare (other) < 0;
  }
  inline bool operator > (const QuicSocketTxScheduleItem& other) const
  {
    return Compare (other) > 0;
  }
  inline bool operator <= (const QuicSocketTxScheduleItem& other) const
  {
    return Compare (other) <= 0;
  }
  inline bool operator >= (const QuicSocketTxScheduleItem& other) const
  {
    return Compare (other) >= 0;
  }

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  Ptr<QuicSocketTxItem> GetItem () const;

  /**
   * \brief Get the ID of the stream the item belongs to.
   * \return the stream ID
   */
  uint64_t GetStreamId () const;

  /**
   * \brief Get the offset of the item in the stream.
   * \return the offset
   */
  uint64_t GetOffset () const;

  /**
   * \brief Get the priority.
   * \return the item priority
   */
  double GetPriority () const;

  /**
   * \brief Change the priority of the item.
   * \param priority the new priority
   */
  void SetPriority (double priority);

private:
  uint64_t m_streamId;                //!< ID of the stream the item belongs to
  uint64_t m_offset;                  //!< offset on the stream
  double m_priority;                  //!< Priority level of the item (lowest is sent first)
  Ptr<QuicSocketTxItem> m_item;       //!< TxItem containing the packet
};


class CompareScheduleItems
{
public:
  bool operator() (Ptr<QuicSocketTxScheduleItem> ita, Ptr<QuicSocketTxScheduleItem> itb)
  {
    return (*ita) > (*itb);
  }
};
/**
 * \ingroup quic
 *
 * \brief Tx socket buffer for QUIC
 */
class QuicSocketTxScheduler : public Object
{
public:

  QuicSocketTxScheduler ();
  QuicSocketTxScheduler (const QuicSocketTxScheduler &other);
  virtual ~QuicSocketTxScheduler (void);

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Add a tx item to the scheduling list (default behavior: FIFO scheduling)
   *
   * \param item a smart pointer to a transmission item
   * \param retx true if the transmission item is being retransmitted
   */
  virtual void Add (Ptr<QuicSocketTxItem> item, bool retx);

  /**
   * \brief Get the next scheduled packet with a specified size
   *
   * \param numBytes number of bytes of the QuicSocketTxItem requested
   * \return the item that contains the right packet
   */
  Ptr<QuicSocketTxItem> GetNewSegment (uint32_t numBytes);

  /**
   * Returns the total number of bytes in the application buffer
   *
   * \return the total number of bytes in the application buffer
   */
  uint32_t AppSize (void) const;
  /**
   * Add a schedule tx item to the scheduling list
   *
   * \param item a scheduling item with priority
   * \param retx true if the item is being retransmitted
   */
  void AddScheduleItem (Ptr<QuicSocketTxScheduleItem> item, bool retx);

private:
  typedef std::priority_queue<Ptr<QuicSocketTxScheduleItem>, std::vector<Ptr<QuicSocketTxScheduleItem> >, CompareScheduleItems> QuicTxPacketList;        //!< container for data stored in the buffer
  QuicTxPacketList m_appList;
  uint32_t m_appSize;
};

} // namespace ns-3

#endif /* QUIC_SOCKET_TX_SCHEDULER_H */
