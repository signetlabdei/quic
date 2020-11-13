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

#ifndef QUICSTREAMRXBUFFER_H
#define QUICSTREAMRXBUFFER_H

#include <map>
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/sequence-number.h"
#include "ns3/ptr.h"
#include "quic-subheader.h"
#include "ns3/object.h"

namespace ns3 {

/**
 * \ingroup quic
 *
 * \brief Item that encloses the received Quic Stream frame
 */
class QuicStreamRxItem
{
public:
  QuicStreamRxItem ();
  QuicStreamRxItem (const QuicStreamRxItem &other);

  /**
   * \brief Print the Item
   * \param os ostream
   */
  void Print (std::ostream &os) const;

  /**
   * Comparison operator
   * \param other rhs operand
   * \return true if the rhs operand offset is greater than the lhs one
   */
  bool operator< (const QuicStreamRxItem& other)
  {
    return m_offset < other.m_offset;
  }

  /**
   * Equal operator
   * \param other rhs operand
   * \return true if the operands are equal
   */
  bool operator== (const QuicStreamRxItem& other)
  {
    return (this->m_offset == other.m_offset) and (this->m_fin == other.m_fin) and (this->m_packet == other.m_packet);
  }

  Ptr<Packet> m_packet;  //!< Stream Frame
  uint64_t m_offset;     //!< Offset of the Stream Frame
  bool m_fin;            //!< FIN bit of the Stream Frame

};

/**
 * \ingroup quic
 *
 * \brief Rx stream buffer for QUIC
 */
class QuicStreamRxBuffer : public Object
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  QuicStreamRxBuffer ();
  virtual ~QuicStreamRxBuffer ();

  /**
   * Print the buffer information to a string,
   * including the list of received packets
   *
   * \param os the std::ostream object
   */
  void Print (std::ostream & os) const;

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
  uint32_t Available () const;

  /**
   * Check how many bytes can be released from the buffer (i.e., how many in-order bytes
   * are present from a certain offset)
   *
   * \param currRecvOffset the current offset in the stream sequence
   * \return a pair with the offset of the last packet to extract and the total number of bytes to extract
   */
  std::pair<uint64_t, uint64_t> GetDeliverable (uint64_t currRecvOffset);

  /**
   * Add a packet to the receive buffer
   *
   * \param p a smart pointer to a packet
   * \param sub the QuicSubheader of the packet
   * \return true if the insertion was successful
   */
  bool Add (Ptr<Packet> p, const QuicSubheader& sub);

  /**
   * Extract maxSize bytes from the buffer
   *
   * \param maxSize the number of bytes to be extracted
   * \return a smart pointer to the extracted packet
   */
  Ptr<Packet> Extract (uint32_t maxSize);

  /**
   * Get the total amount of data received in a stream
   * which has received a frame with the FIN bit set
   *
   * \return the final size of the stream
   */
  uint32_t GetFinalSize () const;

  /**
   * Return the number of bytes in the buffer
   *
   * \return the buffer occupancy in bytes
   */
  uint32_t Size (void) const;

private:
  // TODO consider replacing std::vector with a ordered data structure
  typedef std::vector<QuicStreamRxItem*> QuicStreamRxPacketList;  //!< container for data stored in the buffer

  QuicStreamRxPacketList m_streamRecvList;  //!< List of received packets with additional info
  uint32_t m_numBytesInBuffer;              //!< Current buffer occupancy
  uint32_t m_finalSize;                     //!< Final buffer size
  uint32_t m_maxBuffer;                     //!< Maximum buffer size
  bool m_recvFin;                           //!< FIN bit reception flag

};

} //namepsace ns3

#endif /* QUIC_SREAM_RX_BUFFER_H */
