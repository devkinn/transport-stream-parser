#include "./tsCommon.h"
#include "tsTransportStream.h"

//=============================================================================================================================================================================

int main(int argc, char *argv[], char *envp[])
{
	FILE *TransportStreamFile = std::fopen("../example_new.ts", "rb");

	if (!TransportStreamFile)
	{
		std::perror("File opening failed.");
		return EXIT_FAILURE;
	}

	xTS_PacketHeader TS_PacketHeader;
	xTS_AdaptationField TS_PacketAdaptationField;
	xPES_Assembler PES_Assembler;

	uint8_t TS_PacketBuffer[xTS::TS_PacketLength];

	int32_t TS_PacketId = 0;
	while (!std::feof(TransportStreamFile))
	{

		size_t NumRead = std::fread(TS_PacketBuffer, 1, xTS::TS_PacketLength, TransportStreamFile);
		if (NumRead != xTS::TS_PacketLength)
			break;

		TS_PacketHeader.Reset();
		TS_PacketHeader.Parse(TS_PacketBuffer);

		TS_PacketAdaptationField.Reset();

		if (TS_PacketHeader.getSyncByte() == 'G' && TS_PacketHeader.getPacketIdentifier() == 136)
		{
			if (TS_PacketHeader.hasAdaptationField())
			{
				TS_PacketAdaptationField.Parse(TS_PacketBuffer, TS_PacketHeader.getAdaptationFieldControl());
			}

			printf("%010d ", TS_PacketId);
			TS_PacketHeader.Print();
			if (TS_PacketHeader.hasAdaptationField())
			{
				printf("\n%10s ", " ");
				TS_PacketAdaptationField.Print();
			}

			xPES_Assembler::eResult Result = PES_Assembler.AbsorbPacket(TS_PacketBuffer, &TS_PacketHeader, &TS_PacketAdaptationField);

			switch (Result)
			{
			case xPES_Assembler::eResult::StreamPackedLost:
				printf("PcktLost");
				break;
			case xPES_Assembler::eResult::AssemblingStarted:
				printf("\n%10s ", " ");
				printf("Assembling Started");
				printf("\n%10s ", " ");
				PES_Assembler.PrintPESH();
				break;
			case xPES_Assembler::eResult::AssemblingContinue:
				printf("Assembling Continue");
				break;
			case xPES_Assembler::eResult::AssemblingFinished:
				printf("\n%10s ", " ");
				printf("Assembling Finished");
				printf("\n%10s ", " ");
				printf("PES: Len=%d", PES_Assembler.getNumPacketBytes());
				break;
			default:
				break;
			}
			printf("\n");
		}

		TS_PacketId++;
	}

	std::fclose(TransportStreamFile);

	return EXIT_SUCCESS;
}

//=============================================================================================================================================================================
