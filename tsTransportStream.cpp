#include "tsTransportStream.h"

//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================

/// @brief Reset - reset all TS packet header fields
void xTS_PacketHeader::Reset()
{
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
int32_t xTS_PacketHeader::Parse(const uint8_t *Input)
{
  uint32_t *headerPtr = (uint32_t *)Input;
  uint32_t headerData = xSwapBytes32(*headerPtr);
  m_SB = (decltype(m_SB))((headerData & 0b11111111000000000000000000000000) >> 24);
  m_E = (decltype(m_E))((headerData & 0b00000000100000000000000000000000) >> 23);
  m_S = (decltype(m_S))((headerData & 0b00000000010000000000000000000000) >> 22);
  m_T = (decltype(m_T))((headerData & 0b00000000001000000000000000000000) >> 21);
  m_PID = (decltype(m_PID))((headerData & 0b00000000000111111111111100000000) >> 8);
  m_TSC = (decltype(m_TSC))((headerData & 0b00000000000000000000000011000000) >> 6);
  m_AFC = (decltype(m_AFC))((headerData & 0b00000000000000000000000000110000) >> 4);
  m_CC = (decltype(m_CC))((headerData & 0b00000000000000000000000000001111));

  return 4;
}

/// @brief Print all TS packet header fields
void xTS_PacketHeader::Print() const
{
  printf("TS: SB=%*d", -5, m_SB);
  printf("E=%*d", -5, m_E);
  printf("S=%*d", -5, m_S);
  printf("T=%*d", -5, m_T);
  printf("PID=%*d", -5, m_PID);
  printf("TSC=%*d", -5, m_TSC);
  printf("AFC=%*d", -5, m_AFC);
  printf("CC=%*d", -5, m_CC);
}

//=============================================================================================================================================================================
