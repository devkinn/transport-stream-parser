#include "tsTransportStream.h"

//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================

/// @brief Reset - reset all TS packet header fields
void xTS_PacketHeader::Reset() {
  m_SB = 0;
  m_E = 0;
  m_S = 0;
  m_T = 0;
  m_PID = 0;
  m_TSC = 0;
  m_AFC = 0;
  m_CC = 0;
}

/**
  @brief Parse all TS packet header fields
  @param Input is pointer to buffer containing TS packet
  @return Number of parsed bytes (4 on success, -1 on failure)
 */
int32_t xTS_PacketHeader::Parse(const uint8_t *Input) {
  uint32_t headerData = xSwapBytes32(*((uint32_t *)Input));
  m_SB = (headerData & 0b11111111000000000000000000000000) >> 24;
  m_E = (headerData & 0b00000000100000000000000000000000) >> 23;
  m_S = (headerData & 0b00000000010000000000000000000000) >> 22;
  m_T = (headerData & 0b00000000001000000000000000000000) >> 21;
  m_PID = (headerData & 0b00000000000111111111111100000000) >> 8;
  m_TSC = (headerData & 0b00000000000000000000000011000000) >> 6;
  m_AFC = (headerData & 0b00000000000000000000000000110000) >> 4;
  m_CC = (headerData & 0b00000000000000000000000000001111);

  return 4;
}

/// @brief Print all TS packet header fields
void xTS_PacketHeader::Print() const {
  printf("TS: SB=%*d", -3, m_SB);
  printf("E=%*d", -3, m_E);
  printf("S=%*d", -3, m_S);
  printf("T=%*d", -3, m_T);
  printf("PID=%*d", -5, m_PID);
  printf("TSC=%*d", -3, m_TSC);
  printf("AFC=%*d", -3, m_AFC);
  printf("CC=%*d", -3, m_CC);
}

//=============================================================================================================================================================================
// xTS_AdaptationField
//=============================================================================================================================================================================

/// @brief Reset - reset all TS packet header fields
void xTS_AdaptationField::Reset() {
  m_AdaptationFieldControl = 0;
  m_AdaptationFieldLength = 0;
  m_DC = 0;
  m_RA = 0;
  m_SP = 0;
  m_PR = 0;
  m_OR = 0;
  m_SF = 0;
  m_TP = 0;
  m_EX = 0;
}

/**
  @brief Parse adaptation field
  @param PacketBuffer is pointer to buffer containing TS packet
  @param AdaptationFieldControl is value of Adaptation Field Control field of corresponding TS packet header
  @return Number of parsed bytes (length of AF or -1 on failure)
*/
int32_t xTS_AdaptationField::Parse(const uint8_t *PacketBuffer, uint8_t AdaptationFieldControl) {
  m_AdaptationFieldControl = AdaptationFieldControl;
  m_AdaptationFieldLength = *(PacketBuffer + 4);

  uint8_t mandatoryFlags = *(PacketBuffer + 5);
  m_DC = (mandatoryFlags & 0b10000000) >> 7;
  m_RA = (mandatoryFlags & 0b01000000) >> 6;
  m_SP = (mandatoryFlags & 0b00100000) >> 5;
  m_PR = (mandatoryFlags & 0b00010000) >> 4;
  m_OR = (mandatoryFlags & 0b00001000) >> 3;
  m_SF = (mandatoryFlags & 0b00000100) >> 2;
  m_TP = (mandatoryFlags & 0b00000010) >> 1;
  m_EX = (mandatoryFlags & 0b00000001);

  return m_AdaptationFieldLength;
}

/// @brief Print all TS packet header fields
void xTS_AdaptationField::Print() const {
  printf("AF: L=%*d", -3, m_AdaptationFieldLength);
  printf("DC=%*d", -3, m_DC);
  printf("RA=%*d", -3, m_RA);
  printf("SP=%*d", -3, m_SP);
  printf("PR=%*d", -3, m_PR);
  printf("OR=%*d", -3, m_OR);
  printf("SF=%*d", -3, m_SF);
  printf("TP=%*d", -3, m_TP);
  printf("EX=%*d", -3, m_EX);
}

//=============================================================================================================================================================================
// xPES_Assembler
//=============================================================================================================================================================================

xPES_Assembler::eResult xPES_Assembler::AbsorbPacket(const uint8_t *TransportStreamPacket, const xTS_PacketHeader *PacketHeader, const xTS_AdaptationField *AdaptationField) {
  const uint8_t *pes_pointer = TransportStreamPacket + xTS::TS_HeaderLength;
  if (PacketHeader->hasAdaptationField()) {
    pes_pointer += AdaptationField->getNumBytes();
  }

  if (PacketHeader->getPayloadUnitStartInd() == 1) {
    m_Started = true;
    m_PESH.Reset();
    m_PESH.Parse(pes_pointer);
    m_LastContinuityCounter = PacketHeader->getContinuityCounter();
    m_DataOffset = xTS::TS_PacketLength - xTS::TS_HeaderLength;
    if (PacketHeader->hasAdaptationField()) {
      m_DataOffset -= AdaptationField->getNumBytes();
    }
    return eResult::AssemblingStarted;
  }

  if ((1 + m_LastContinuityCounter) != PacketHeader->getContinuityCounter()) {
    return eResult::StreamPackedLost;
  }

  m_DataOffset += xTS::TS_PacketLength - xTS::TS_HeaderLength;
  if (PacketHeader->hasAdaptationField()) {
    m_DataOffset -= AdaptationField->getNumBytes();
  }

  if (m_DataOffset >= m_PESH.getPacketLength()) {
    return eResult::AssemblingFinished;
  }

  m_LastContinuityCounter = PacketHeader->getContinuityCounter();
  return eResult::AssemblingContinue;
}

//=============================================================================================================================================================================
// xPES_PacketHeader
//=============================================================================================================================================================================

int32_t xPES_PacketHeader::Parse(const uint8_t *Input) {
  uint64_t headerData = xSwapBytes64(*((uint64_t *)Input));
  m_PacketStartCodePrefix = (headerData & 0xFFFFFF0000000000) >> 40;
  m_StreamId = (headerData & 0x000000FF00000000) >> 32;
  m_PacketLength = (headerData & 0x00000000FFFF0000) >> 16;

  if (m_StreamId != eStreamId::eStreamId_program_stream_map && m_StreamId != eStreamId::eStreamId_padding_stream && m_StreamId != eStreamId::eStreamId_private_stream_2 && m_StreamId != eStreamId::eStreamId_ECM && m_StreamId != eStreamId::eStreamId_EMM && m_StreamId != eStreamId::eStreamId_program_stream_directory && m_StreamId != eStreamId::eStreamId_DSMCC_stream && m_StreamId != eStreamId::eStreamId_ITUT_H222_1_type_E) {
    uint8_t extensionData = *(Input + 8);
    return 9 + extensionData;
  }
  return 6;
}

void xPES_PacketHeader::Reset() {
  m_PacketStartCodePrefix = 0;
  m_StreamId = 0;
  m_PacketLength = 0;
}

void xPES_PacketHeader::Print() const {
  printf("PES: PSCP=%*d", -3, m_PacketStartCodePrefix);
  printf("SID=%*d", -5, m_StreamId);
  printf("L=%*d", -5, m_PacketLength);
}