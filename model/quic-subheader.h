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

#ifndef QUICSUBHEADER_H
#define QUICSUBHEADER_H

#include <stdint.h>
#include "ns3/header.h"
#include "ns3/buffer.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/sequence-number.h"

namespace ns3 {

/**
 * \ingroup quic
 * \brief SubHeader for the QUIC Protocol
 *
 * This class has fields corresponding to those in a QUIC subheader
 * (stream id, connection id, error code, offset, flags, etc) as well
 * as methods for serialization to and deserialization from a buffer.
 *
 * Frames and Frame Types [Quic IETF Draft 13 Transport - sec. 5]
 * --------------------------------------------------------------
 *
 * The payload of all packets, after removing packet protection, consists
 * of a sequence of frames. Version Negotiation and Stateless Reset do not
 * contain frames. Payloads MUST contain at least one frame, and MAY
 * contain multiple frames and multiple frame types.
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                          Frame 1 (*)                        ...
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                          Frame 2 (*)                        ...
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *                                  ...
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                          Frame N (*)                        ...
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Frames MUST fit within a single QUIC packet and MUST NOT span a QUIC
 * packet boundary. Each frame begins  with a Frame Type, indicating
 * its type, followed by additional type-dependent fields.
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                           Type (i)                          ...
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                   Type-Dependent Fields (*)                 ...
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Variable-Length Integer Encoding [Quic IETF Draft 13 Transport - sec. 7.1]
 * --------------------------------------------------------------------------
 *
 * QUIC frames commonly use a variable-length encoding for non-negative
 * integer values. This encoding ensures that smaller integer values
 * need fewer octets to encode. The QUIC variable-length integer encoding
 * reserves the two most significant bits of the first octet to encode the
 * base 2 logarithm of the integer encoding length in octets. The integer
 * value is encoded on the remaining bits, in network byte order. This
 * means that integers are encoded on 1, 2, 4, or 8 octets and can encode
 * 6, 14, 30, or 62 bit values respectively.
 *
 *   +------+--------+-------------+-----------------------+
 *   | 2Bit | Length | Usable Bits | Range                 |
 *   +------+--------+-------------+-----------------------+
 *   | 00   | 1      | 6           | 0-63                  |
 *   |      |        |             |                       |
 *   | 01   | 2      | 14          | 0-16383               |
 *   |      |        |             |                       |
 *   | 10   | 4      | 30          | 0-1073741823          |
 *   |      |        |             |                       |
 *   | 11   | 8      | 62          | 0-4611686018427387903 |
 *   +------+--------+-------------+-----------------------+
 *
 */
class QuicSubheader : public Header
{
public:
  /**
   * \brief Quic subheader type frame values
   */
  typedef enum
  {
    PADDING = 0x00,            //!< Padding
    RST_STREAM = 0x01,         //!< Rst Stream
    CONNECTION_CLOSE = 0x02,   //!< Connection Close
    APPLICATION_CLOSE = 0x03,  //!< Application Close
    MAX_DATA = 0x04,           //!< Max Data
    MAX_STREAM_DATA = 0x05,    //!< Max Stream Data
    MAX_STREAM_ID = 0x06,      //!< Max Stream Id
    PING = 0x07,               //!< Ping
    BLOCKED = 0x08,            //!< Blocked
    STREAM_BLOCKED = 0x09,     //!< Stream Blocked
    STREAM_ID_BLOCKED = 0x0A,  //!< Stream Id Blocked
    NEW_CONNECTION_ID = 0x0B,  //!< New Connection Id
    STOP_SENDING = 0x0C,       //!< Stop Sending
    ACK = 0x0D,                //!< Ack
    PATH_CHALLENGE = 0x0E,     //!< Path Challenge
    PATH_RESPONSE = 0x0F,      //!< Path Response
    STREAM000 = 0x10,          //!< Stream (offset=0, length=0, fin=0)
    STREAM001 = 0x11,          //!< Stream (offset=0, length=0, fin=1)
    STREAM010 = 0x12,          //!< Stream (offset=0, length=1, fin=0)
    STREAM011 = 0x13,          //!< Stream (offset=0, length=1, fin=1)
    STREAM100 = 0x14,          //!< Stream (offset=1, length=0, fin=0)
    STREAM101 = 0x15,          //!< Stream (offset=1, length=0, fin=1)
    STREAM110 = 0x16,          //!< Stream (offset=1, length=1, fin=0)
    STREAM111 = 0x17           //!< Stream (offset=1, length=1, fin=1)
  } TypeFrame_t;

  /**
   * \brief Quic subheader transport error codes values
   */
  typedef enum
  {
    NO_ERROR = 0x00,                   // No error
    INTERNAL_ERROR = 0x01,             // Implementation error
    SERVER_BUSY = 0x02,                // Server currently busy
    FLOW_CONTROL_ERROR = 0x03,         // Flow control error
    STREAM_ID_ERROR = 0x04,            // Invalid stream ID
    STREAM_STATE_ERROR = 0x05,         // Frame received in invalid stream state
    FINAL_OFFSET_ERROR = 0x06,         // Change to final stream offset
    FRAME_FORMAT_ERROR = 0x07,         // Generic frame format error
    TRANSPORT_PARAMETER_ERROR = 0x08,  // Error in transport parameters
    VERSION_NEGOTIATION_ERROR = 0x09,  // Version negotiation failure
    PROTOCOL_VIOLATION = 0x0A,         // Generic protocol violation
    UNSOLICITED_PATH_ERROR = 0x0B,     // Unsolicited PATH_RESPONSE frame
    FRAME_ERROR = 0x100                // Specific frame format error [0x100-0x1FF] -> will simply use Frame Error 0x100 as a mask and summing specific TypeFrame_t
  } TransportErrorCodes_t;

  QuicSubheader ();
  virtual ~QuicSubheader ();

  // Inherited from Header
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  /**
   * \brief Print a Quic subheader into an output stream
   *
   * \param os output stream
   * \param tc Quic subheader to print
   * \return The ostream passed as first argument
   */
  friend std ::ostream& operator<< (std::ostream& os, QuicSubheader & tc);

  /**
   * \brief Converts the frame type flag into a human readable string description
   *
   * \return the generated string
   **/
  std::string FrameTypeToString () const;

  /**
   * \brief Converts the transport error code into a human readable string description
   *
   * \return the generated string
   **/
  std::string TransportErrorCodeToString () const;

  /**
   * \brief Write a variable-length 64-bit integer on a buffer according to the encoding standards
   *
   * \param i the buffer iterator
   * \param varInt64 the variable-length 64-bit integer
   **/
  void WriteVarInt64 (Buffer::Iterator& i, uint64_t varInt64) const;

  /**
   * \brief Read a variable-length 64-bit integer from a buffer according to the encoding standards
   *
   * \param i the buffer iterator
   * \return the variable-length 64-bit integer
   **/
  uint64_t ReadVarInt64 (Buffer::Iterator& i);

  /**
   * \brief Get the variable-length 64-bit integer size according to the encoding standards (in bits)
   *
   * \param varInt64 the variable-length 64-bit integer
   * \return the size of the variable-length 64-bit integer (in bits)
   **/
  static uint32_t GetVarInt64Size (uint64_t varInt64);

  /**
   * Create a Padding subheader
   *
   * \return the generated QuicSubheader
   */
  static QuicSubheader CreatePadding (void);

  /**
   * Create a Max Stream Data subheader
   *
   * \param streamId the stream id of the stream being terminated
   * \param applicationErrorCode the error code that indicates why the stream is being closed
   * \param finalOffset the absolute byte offset of the end of data written on this stream by the RST_STREAM sender
   * \return the generated QuicSubheader
   */
  static QuicSubheader CreateRstStream (uint64_t streamId, uint16_t applicationErrorCode, uint64_t finalOffset);

  /**
   * Create a Connection Close subheader
   *
   * \param errorCode the error code that indicates the reason for closing this connection
   * \param reasonPhrase the human-readable explanation for why the connection has been closed
   * \return the generated QuicSubheader
   */
  static QuicSubheader CreateConnectionClose (uint16_t errorCode, const char* reasonPhrase);

  /**
   * Create a Application Close subheader
   *
   * \param errorCode the error code that indicates the reason for closing this connection
   * \param reasonPhrase the human-readable explanation for why the connection has been closed
   * \return the generated QuicSubheader
   */
  static QuicSubheader CreateApplicationClose (uint16_t errorCode, const char* reasonPhrase);

  /**
   * Create a Max Data subheader
   *
   * \param maxData the maximum amount of data that can be sent on the entire connection
   * \return the generated QuicSubheader
   */
  static QuicSubheader CreateMaxData (uint64_t maxData);

  /**
   * Create a Max Stream Data subheader
   *
   * \param streamId The stream id of the stream that is affected
   * \param maxStreamData the maximum amount of data that can be sent on the identified stream
   * \return the generated QuicSubheader
   */
  static QuicSubheader CreateMaxStreamData (uint64_t streamId, uint64_t maxStreamData);

  /**
   * Create a Max Stream Id subheader
   *
   * \param maxStreamId the id of the maximum unidirectional or bidirectional peer-initiated stream id for the connection
   * \return the generated QuicSubheader
   */
  static QuicSubheader CreateMaxStreamId (uint64_t maxStreamId);

  /**
   * Create a Ping subheader
   *
   * \return the generated QuicSubheader
   */
  static QuicSubheader CreatePing (void);

  /**
   * Create a Blocked subheader
   *
   * \param offset the connection-level offset at which the blocking occurred
   * \return the generated QuicSubheader
   */
  static QuicSubheader CreateBlocked (uint64_t offset);

  /**
   * Create a Stream Blocked subheader
   *
   * \param streamId the stream which is flow control blocked
   * \param offset the offset of the stream at which the blocking occurred
   * \return the generated QuicSubheader
   */
  static QuicSubheader CreateStreamBlocked (uint64_t streamId, uint64_t offset);

  /**
   * Create a Stream Id Blocked subheader
   *
   * \param streamId the highest stream id that the sender was permitted to open
   * \return the generated QuicSubheader
   */
  static QuicSubheader CreateStreamIdBlocked (uint64_t streamId);

  /**
   * Create a New Connection Id subheader
   *
   * \param sequence this value starts at 0 and increases by 1 for each connection ID that is provided by the server
   * \param connectionId the new connection id
   * \param statelessResetToken the 128-bit value that will be used to for a stateless reset when the associated connection ID is used
   * \return the generated QuicSubheader
   */
  static QuicSubheader CreateNewConnectionId (uint64_t sequence, uint64_t connectionId);     //uint128_t statelessResetToken);

  /**
   * Create a Stop Sending subheader
   *
   * \param streamId the stream id of the stream being ignored
   * \param applicationErrorCode the application-specified reason the sender is ignoring the stream
   * \return the generated QuicSubheader
   */
  static QuicSubheader CreateStopSending (uint64_t streamId, uint16_t applicationErrorCode);

  /**
   * Create a Ack subheader
   *
   * \param largestAcknowledged the largest packet number the peer is acknowledging
   * \param ackDelay the time in microseconds that the largest acknowledged packet, was received by this peer to when this ACK was sent
   * \param firstAckBlock the number of contiguous packets preceding the Largest Acknowledged that are being acknowledged
   * \param gaps the vector where each field contains the number of contiguous unacknowledged packets preceding the packet number one lower than the smallest in the preceding ack block
   * \param additionalAckBlocks the vector where each field contains the number of contiguous acknowledged packets preceding the largest packet number
   * \return the generated QuicSubheader
   */
  static QuicSubheader CreateAck (uint32_t largestAcknowledged, uint64_t ackDelay, uint32_t firstAckBlock, std::vector<uint32_t>& gaps, std::vector<uint32_t>& additionalAckBlocks);

  /**
   * Create a Path Response subheader
   *
   * \param data the data word of the Path Challenge subheader
   * \return the generated QuicSubheader
   */
  static QuicSubheader CreatePathChallenge (uint8_t data);

  /**
   * Create a Path Response subheader
   *
   * \param data the data word of the Path Response subheader
   * \return the generated QuicSubheader
   */
  static QuicSubheader CreatePathResponse (uint8_t data);

  /**
   * Create a Stream subheader
   *
   * \param streamId the ID of the stream
   * \param offset the offset of the first byte of the frame in the stream
   * \param length the packet size
   * \param offBit a flag to indicate whether the offset is carried or not
   * \param lengthBit a flag to indicate whether the length is carried or not
   * \param finBit a flag to indicate the FIN bit
   * \return the generated QuicSubheader
   */
  static QuicSubheader CreateStreamSubHeader (uint64_t streamId, uint64_t offset, uint64_t length, bool offBit = false, bool lengthBit = false, bool finBit = false);

  // Getters, Setters and Controls

  /**
   * \brief Get the ack block count
   * \return The ack block count for this QuicSubheader
   */
  uint32_t GetAckBlockCount () const;

  /**
   * \brief Set the ack block count
   * \param ackBlockCount the ack block count for this QuicSubheader
   */
  void SetAckBlockCount (uint32_t ackBlockCount);

  /**
   * \brief Get the additional ack block vector
   * \return The additional ack block vector for this QuicSubheader
   */
  const std::vector<uint32_t>& GetAdditionalAckBlocks () const;

  /**
   * \brief Set the additional ack block vector
   * \param ackBlocks the additional ack block vector for this QuicSubheader
   */
  void SetAdditionalAckBlocks (const std::vector<uint32_t>& ackBlocks);

  /**
   * \brief Get the ack delay
   * \return The ack delay for this QuicSubheader
   */
  uint64_t GetAckDelay () const;

  /**
   * \brief Set the ack delay
   * \param ackDelay the ack delay for this QuicSubheader
   */
  void SetAckDelay (uint64_t ackDelay);

  /**
   * \brief Get the connection id
   * \return The connection id for this QuicSubheader
   */
  uint64_t GetConnectionId () const;

  /**
   * \brief Set the connection id
   * \param connectionId the connection id for this QuicSubheader
   */
  void SetConnectionId (uint64_t connectionId);

  /**
   * \brief Get the data word
   * \return The data word for this QuicSubheader
   */
  uint8_t GetData () const;

  /**
   * \brief Set the data word
   * \param data the data word for this QuicSubheader
   */
  void SetData (uint8_t data);

  /**
   * \brief Get the error code
   * \return The data word for this QuicSubheader
   */
  uint16_t GetErrorCode () const;

  /**
   * \brief Set the error code
   * \param errorCode the error code for this QuicSubheader
   */
  void SetErrorCode (uint16_t errorCode);

  /**
   * \brief Get the frame type
   * \return The frame type for this QuicSubheader
   */
  uint8_t GetFrameType () const;

  /**
   * \brief Set the frame type
   * \param frameType the frame type for this QuicSubheader
   */
  void SetFrameType (uint8_t frameType);

  /**
   * \brief Get the gap vector
   * \return The gap vector for this QuicSubheader
   */
  const std::vector<uint32_t>& GetGaps () const;

  /**
   * \brief Set the gap vector
   * \param gaps the gap for this QuicSubheader
   */
  void SetGaps (const std::vector<uint32_t>& gaps);

  /**
   * \brief Get the largest acknowledged
   * \return The largest acknowledged for this QuicSubheader
   */
  uint32_t GetLargestAcknowledged () const;

  /**
   * \brief Set the largest acknowledged
   * \param largestAcknowledged the largest acknowledged for this QuicSubheader
   */
  void SetLargestAcknowledged (uint32_t largestAcknowledged);

  /**
   * \brief Get the length
   * \return The length for this QuicSubheader
   */
  uint64_t GetLength () const;

  /**
   * \brief Set the length
   * \param length the length for this QuicSubheader
   */
  void SetLength (uint64_t length);

  /**
   * \brief Get the max data
   * \return The max data for this QuicSubheader
   */
  uint64_t GetMaxData () const;

  /**
   * \brief Set the max data
   * \param maxData the max data for this QuicSubheader
   */
  void SetMaxData (uint64_t maxData);

  /**
   * \brief Get the max stream data
   * \return The max stream data for this QuicSubheader
   */
  uint64_t GetMaxStreamData () const;

  /**
   * \brief Set the max stream data
   * \param maxStreamData the max stream data for this QuicSubheader
   */
  void SetMaxStreamData (uint64_t maxStreamData);

  /**
   * \brief Get the max stream id
   * \return The max stream id for this QuicSubheader
   */
  uint64_t GetMaxStreamId () const;

  /**
   * \brief Set the max stream id
   * \param maxStreamId the max stream id for this QuicSubheader
   */
  void SetMaxStreamId (uint64_t maxStreamId);

  /**
   * \brief Get the offset
   * \return The offset for this QuicSubheader
   */
  uint64_t GetOffset () const;

  /**
   * \brief Set the offset
   * \param offset the offset for this QuicSubheader
   */
  void SetOffset (uint64_t offset);

  /**
   * \brief Get the reason phrase vector
   * \return The reason phrase vector for this QuicSubheader
   */
  const std::vector<uint8_t>& GetReasonPhrase () const;

  /**
   * \brief Set the reason phrase vector
   * \param reasonPhrase the reason phrase vector for this QuicSubheader
   */
  void SetReasonPhrase (const std::vector<uint8_t>& reasonPhrase);

  /**
   * \brief Get the reason phrase length
   * \return The reason phrase length for this QuicSubheader
   */
  uint64_t GetReasonPhraseLength () const;

  /**
   * \brief Set the reason phrase length
   * \param reasonPhraseLength the reason phrase length for this QuicSubheader
   */
  void SetReasonPhraseLength (uint64_t reasonPhraseLength);

  /**
   * \brief Get the sequence
   * \return The sequence for this QuicSubheader
   */
  uint64_t GetSequence () const;

  /**
   * \brief Set the sequence
   * \param sequence the sequence for this QuicSubheader
   */
  void SetSequence (uint64_t sequence);

  /**
   * \brief Get the stream Id
   * \return The stream Id for this QuicSubheader
   */
  uint64_t GetStreamId () const;

  /**
   * \brief Set the stream Id
   * \param streamId the stream Id for this QuicSubheader
   */
  void SetStreamId (uint64_t streamId);

  /**
   * \brief Get the first ack block
   * \return The first ack block for this QuicSubheader
   */
  uint64_t GetFirstAckBlock () const;

  /**
   * \brief Set the first ack block
   * \param firstAckBlock the first ack block for this QuicSubheader
   */
  void SetFirstAckBlock (uint64_t firstAckBlock);

  // TODO: Implement Stateless Reset Token functionality
  // uint128_t getStatelessResetToken() const;
  // void SetStatelessResetToken(uint128_t statelessResetToken);

  /**
   * \brief Check if the subheader is Padding
   * \return true if the subheader is Padding, false otherwise
   */
  bool IsPadding () const;

  /**
   * \brief Check if the subheader is Rst Stream
   * \return true if the subheader is Rst Stream, false otherwise
   */
  bool IsRstStream () const;

  /**
   * \brief Check if the subheader is Connection Close
   * \return true if the subheader is Connection Close, false otherwise
   */
  bool IsConnectionClose () const;

  /**
   * \brief Check if the subheader is Application Close
   * \return true if the subheader is Application Close, false otherwise
   */
  bool IsApplicationClose () const;

  /**
   * \brief Check if the subheader is Max Data
   * \return true if the subheader is Max Data, false otherwise
   */
  bool IsMaxData () const;

  /**
   * \brief Check if the subheader is Max Stream Data
   * \return true if the subheader is Max Stream Data, false otherwise
   */
  bool IsMaxStreamData () const;

  /**
   * \brief Check if the subheader is Max Stream Id
   * \return true if the subheader is Max Stream Id, false otherwise
   */
  bool IsMaxStreamId () const;

  /**
   * \brief Check if the subheader is Ping
   * \return true if the subheader is Ping, false otherwise
   */
  bool IsPing () const;

  /**
   * \brief Check if the subheader is Blocked
   * \return true if the subheader is Blocked, false otherwise
   */
  bool IsBlocked () const;

  /**
   * \brief Check if the subheader is Stream Blocked
   * \return true if the subheader is Stream Blocked, false otherwise
   */
  bool IsStreamBlocked () const;

  /**
   * \brief Check if the subheader is Stream Id Blocked
   * \return true if the subheader is Stream Id Blocked, false otherwise
   */
  bool IsStreamIdBlocked () const;

  /**
   * \brief Check if the subheader is New Connection Id
   * \return true if the subheader is New Connection Id, false otherwise
   */
  bool IsNewConnectionId () const;

  /**
   * \brief Check if the subheader is Stop Sending
   * \return true if the subheader is Stop Sending, false otherwise
   */
  bool IsStopSending () const;

  /**
   * \brief Check if the subheader is Ack
   * \return true if the subheader is Ack, false otherwise
   */
  bool IsAck () const;

  /**
   * \brief Check if the subheader is Path Challenge
   * \return true if the subheader is Path Challenge, false otherwise
   */
  bool IsPathChallenge () const;

  /**
   * \brief Check if the subheader is Path Response
   * \return true if the subheader is Path Response, false otherwise
   */
  bool IsPathResponse () const;

  /**
   * \brief Check if the subheader is Stream
   * \return true if the subheader is Stream, false otherwise
   */
  bool IsStream () const;

  /**
   * \brief Check if the subheader is Stream and the FIN bit is true
   * \return true if the subheader is Stream and the FIN bit is true, false otherwise
   */
  bool IsStreamFin () const;

  /**
   * Comparison operator
   * \param lhs left operand
   * \param rhs right operand
   * \return true if the operands are equal
   */
  friend bool operator== (const QuicSubheader &lhs, const QuicSubheader &rhs);

  /**
   * \brief Calculates the subheader length (in words)
   *
   * Given the standard size of the subheader, the method checks for options
   * and calculates the real length (in words).
   *
   * \return subheader length in 4-byte words
   */
  uint32_t CalculateSubHeaderLength () const;

private:
  uint8_t m_frameType;                          //!< Frame type
  uint64_t m_streamId;                          //!< Stream id
  uint16_t m_errorCode;                         //!< Error code
  uint64_t m_offset;                            //!< Offset
  uint64_t m_reasonPhraseLength;                //!< Reason phrase length
  std::vector<uint8_t> m_reasonPhrase;          //!< Reason phrase
  uint64_t m_maxData;                           //!< Max data limit
  uint64_t m_maxStreamData;                     //!< Max stream data limit
  uint64_t m_maxStreamId;                       //!< Max stream id limit
  uint64_t m_sequence;                          //!< Sequence
  uint64_t m_connectionId;                      //!< Connection id
  //uint128_t statelessResetToken;              //!< Stateless reset token
  uint32_t m_largestAcknowledged;               //!< Largest acknowledged
  uint32_t m_ackDelay;                          //!< Ack delay
  uint32_t m_ackBlockCount;                     //!< Ack block count
  uint32_t m_firstAckBlock;                     //!< First Ack block
  std::vector<uint32_t> m_additionalAckBlocks;  //!< Additional ack blocks vector
  std::vector<uint32_t> m_gaps;                 //!< Gaps vector
  uint8_t m_data;                               //!< Data word
  uint64_t m_length;                            //!< Length
};

} // namespace ns3

#endif /* QUIC_SUBHEADER_H_ */
