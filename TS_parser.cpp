#include "./tsCommon.h"
#include "tsTransportStream.h"
#include <cstdio>

//=============================================================================================================================================================================

int main(int argc, char *argv[], char *envp[])
{
  // TODO - open file
  FILE *fp = std::fopen("../example_new.ts", "rb");
  
  // TODO - check if file if opened
  if (!fp)
  {
    std::perror("File opening failed");
    return EXIT_FAILURE;
  }

  xTS_PacketHeader TS_PacketHeader;
  xTS TS;

  uint8_t TS_PacketBuffer[TS.TS_PacketLength];

  int32_t TS_PacketId = 0;
  while (!std::feof(fp))
  {
    if (TS_PacketId == 10)
      break;
    // TODO - read from file
    std::fread(TS_PacketBuffer, sizeof(uint8_t), TS.TS_PacketLength, fp);

    TS_PacketHeader.Reset();
    TS_PacketHeader.Parse(TS_PacketBuffer);

    printf("%010d ", TS_PacketId);
    TS_PacketHeader.Print();
    printf("\n");

    TS_PacketId++;
  }

  // TODO - close file
  std::fclose(fp);

  return EXIT_SUCCESS;
}

//=============================================================================================================================================================================
