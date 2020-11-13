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

#ifndef QUICSTREAMTXBUFFER_H
#define QUICSTREAMTXBUFFER_H

#include "ns3/object.h"
#include "ns3/traced-value.h"
#include "ns3/sequence-number.h"
#include "ns3/nstime.h"
#include "quic-subheader.h"

namespace ns3 {

/**
 * \ingroup quic
 *
 * \brief Item that encloses the frame packet and some flags for it
 */
class QuicStreamTxItem : public Object
{
public:
  QuicStreamTxItem ();
  QuicStreamTxItem (const QuicStreamTxItem &other);
  ~QuicStreamTxItem ();

  /**
   * \brief Print the Item
   * \param os ostream
   */
  void Print (std::ostream &os) const;

  SequenceNumber32 m_packetNumberSequence;  //!< Sequence number of the application packet associated with this frame
  Ptr<Packet> m_packet;                     //!< packet associated to this QuicStreamTxItem
  bool m_lost;                              //!< true if the frame is lost
  bool m_retrans;                           //!< true if it is a retx
  bool m_sacked;                            //!< true if already acknowledged
  Time m_lastSent;                          //!< time at which it was sent
  uint64_t m_id; // UNUSED !!?


};

/**
 * \ingroup quic
 *
 * \brief Tx stream buffer for QUIC
 */
class QuicStreamTxBuffer : public Object
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  QuicStreamTxBuffer ();
  virtual ~QuicStreamTxBuffer (void);

  /**
   * Print the buffer information to a string,
   * including the list of sent packets
   *
   * \param os the std::ostream object
   */
  void Print (std::ostream & os) const;
  //friend std::ostream & operator<< (std::ostream & os, QuicTxBuffer const & quicTxBuf);

  /**
   * Add a packet to the tx buffer
   *
   * \param p a smart pointer to a packet
   * \return true if the insertion was successful
   */
  bool Add (Ptr<Packet> p);

  /**
   * ReAdd a rejected packet from the socket tx buffer to the stream tx buffer
   *
   * \param p a smart pointer to a packet
   * \return true if the insertion was successful
   */
  bool Rejected (Ptr<Packet> p);

  /**
   * \brief Request the next frame to transmit
   *
   * \param numBytes the number of bytes of the next frame to transmit requested
   * \param seq the sequence number of the next frame to transmit
   * \return the next frame to transmit
   */
  Ptr<Packet> NextSequence (uint32_t numBytes, const SequenceNumber32 seq);

  /**
   * \brief Get a block of data not transmitted yet and move it into SentList
   *
   * \param numBytes number of bytes of the QuicSocketTxItem requested
   * \return the item that contains the right packet
   */
  Ptr<QuicStreamTxItem> GetNewSegment (uint32_t numBytes);

  /**
   * Process an acknowledgment, set the frames in the send buffer as acknowledged, mark
   * lost frames (according to the QUIC IETF draft)
   *
   * \brief Process an ACK
   *
   * \param largestAcknowledged The largest acknowledged sequence number
   * \param additionalAckBlocks The sequence numbers that were just acknowledged
   * \param gaps The gaps in the acknowledgment
   */
  void OnAckUpdate (const uint64_t largestAcknowledged, const std::vector<uint64_t> &additionalAckBlocks, const std::vector<uint64_t> &gaps);

  /**
   * Get the max size of the buffer
   *
   * \return the maximum buffer size in bytes
   */
  uint32_t GetMaxBufferSize (void) const;

  /**
   * Set the max size of the buffer
   *
   * \param n the maximum buffer size in bytes
   */
  void SetMaxBufferSize (uint32_t n);

  /**
   * Compute the available space in the buffer
   *
   * \return the available space in the buffer
   */
  uint32_t Available (void) const;

  /**
   * Returns the total number of bytes in the application buffer
   *
   * \return the total number of bytes in the application buffer
   */
  uint32_t AppSize (void) const;

  /**
   * \brief Return total bytes in flight
   *
   * \returns total bytes in flight
   */
  uint32_t BytesInFlight () const;

private:
  typedef std::list<Ptr<QuicStreamTxItem>> QuicTxPacketList;  //!< container for data stored in the buffer

  /**
   * \brief Merge two QuicStreamTxItem
   *
   * Merge t2 in t1. It consists in copying the lastSent field if t2 is more
   * recent than t1. Retransmitted field is copied only if it set in t2 but not
   * in t1. Sacked is copied only if it is true in both items.
   *
   * \param t1 first item
   * \param t2 second item
   */
  void MergeItems (QuicStreamTxItem &t1, QuicStreamTxItem &t2) const;

  /**
   * \brief Split one QuicStreamTxItem
   *
   * Move "size" bytes from t2 into t1, copying all the fields.
   *
   * \param t1 first item
   * \param t2 second item
   * \param size Size to split
   */
  void SplitItems (QuicStreamTxItem &t1, QuicStreamTxItem &t2, uint32_t size) const; // Available only for streams

  QuicTxPacketList m_appList;   //!< List of buffered application data to be transmitted with additional info
  QuicTxPacketList m_sentList;  //!< List of sent frame with additional info
  uint32_t m_maxBuffer;         //!< Max number of data bytes in buffer (SND.WND)
  uint32_t m_appSize;           //!< Size of all data in the application list
  uint32_t m_sentSize;          //!< Size of all data in the sent list

};


}

#endif /* QUIC_STREAM_TX_BUFFER_H */
