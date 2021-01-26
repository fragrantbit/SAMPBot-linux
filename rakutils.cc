#include "rakutils.h"

InternalPacketPool internalPacketPool;
unsigned sendPacketCount=0;
DataStructures::Queue<InternalPacket*> sendPacketSet[ 4 ];
DataStructures::RangeList<unsigned short> acknowlegements;
RakNetStatisticsStruct statistics;



unsigned histogramPlossCount, histogramAckCount;
long long unreliableTimeout;

int RakUtils::getBSHeaderLen(const InternalPacket *const internalPacket)
{
    int bitLength;

    bitLength=sizeof(MessageNumberType)*2*8;

    // Write the PacketReliability.  This is encoded in 3 bits
    //bitStream->WriteBits((unsigned char*)&(internalPacket->reliability), 3, true);
    bitLength += 4;


    /* (!)
    // Packet is of RELIABLE reliability.
    

    // If the reliability requires an ordering channel and ordering index, we Write those.
    /*if ( internalPacket->reliability == UNRELIABLE_SEQUENCED || internalPacket->reliability == RELIABLE_SEQUENCED || internalPacket->reliability == RELIABLE_ORDERED )
    {
        // ordering channel encoded in 5 bits (from 0 to 31)
        //bitStream->WriteBits((unsigned char*)&(internalPacket->orderingChannel), 5, true);
        bitLength+=5;

        // ordering index is one byte
        //bitStream->WriteCompressed(internalPacket->orderingIndex);
        bitLength+=sizeof(OrderingIndexType)*8;
    }*/

    // Write if this is a split packet (1 bit)
    //bool isSplitPacket = internalPacket->splitPacketCount > 0;

    //bitStream->Write(isSplitPacket);
    bitLength += 1;

    /*if ( isSplitPacket )
    {
        // split packet indices are two bytes (so one packet can be split up to 65535
        // times - maximum packet size would be about 500 * 65535)
        //bitStream->Write(internalPacket->splitPacketId);
        //bitStream->WriteCompressed(internalPacket->splitPacketIndex);
        //bitStream->WriteCompressed(internalPacket->splitPacketCount);
        bitLength += (sizeof(SplitPacketIdType) + sizeof(SplitPacketIndexType) * 2) * 8;
    }*/

    // Write how many bits the packet data is. Stored in an unsigned short and
    // read from 16 bits
    //bitStream->WriteBits((unsigned char*)&(internalPacket->dataBitLength), 16, true);

    // Read how many bits the packet data is.  Stored in 16 bits
    bitLength += 16;

    // Byte alignment
    //bitLength += 8 - ((bitLength -1) %8 + 1);

    return bitLength;
}


int RakUtils::signBSFromIP(RakNet::BitStream *bs, 
        const InternalPacket *const internalPacket)
{

    int start = bs->GetNumberOfBitsUsed();
    const unsigned char c = (unsigned char) internalPacket->reliability;

    // testing
    //if (internalPacket->reliability==UNRELIABLE)
    //  printf("Sending unreliable packet %i\n", internalPacket->messageNumber);
    //else if (internalPacket->reliability==RELIABLE_SEQUENCED || internalPacket->reliability==RELIABLE_ORDERED || internalPacket->reliability==RELIABLE)
    //	  printf("Sending reliable packet number %i\n", internalPacket->messageNumber);

    //bitStream->AlignWriteToByteBoundary();

    // Write the message number (2 bytes)
    bs->Write( internalPacket->messageNumber );

    // Acknowledgment packets have no more data than the messageNumber and whether it is anacknowledgment

    // Write the PacketReliability.  This is encoded in 3 bits
    bs->WriteBits( (const unsigned char *)&c, 4, true );

    // If the reliability requires an ordering channel and ordering index, we Write those.
    if ( internalPacket->reliability == UNRELIABLE_SEQUENCED || internalPacket->reliability == RELIABLE_SEQUENCED || internalPacket->reliability == RELIABLE_ORDERED )
    {
        // ordering channel encoded in 5 bits (from 0 to 31)
        bs->WriteBits( ( unsigned char* ) & ( internalPacket->orderingChannel ), 5, true );

        // One or two bytes
        bs->Write( internalPacket->orderingIndex );
    }

    // Write if this is a split packet (1 bit)
    bool isSplitPacket = internalPacket->splitPacketCount > 0;

    bs->Write( isSplitPacket );

    if ( isSplitPacket )
    {
        bs->Write( internalPacket->splitPacketId );
        bs->WriteCompressed( internalPacket->splitPacketIndex );
        bs->WriteCompressed( internalPacket->splitPacketCount );
    }

    // Write how many bits the packet data is. Stored in 13 bits

    unsigned short length = ( unsigned short ) internalPacket->dataBitLength; // Ignore the 2 high bytes for WriteBits

    bs->WriteCompressed( length );

    // Write the actual data.
    bs->WriteAlignedBytes( ( unsigned char* ) internalPacket->data, BITS_TO_BYTES( internalPacket->dataBitLength ) );

    //bitStream->WriteBits((unsigned char*)internalPacket->data, internalPacket->dataBitLength);

    return bs->GetNumberOfBitsUsed() - start;
}


void RakUtils::insertPacket(
        InternalPacket *internalPacket, 
        RakNetTimeNS time, 
        bool makeCopyOfInternalPacket, bool firstResend)
{
    // lastAckTime is the time we last got an acknowledgment - however we also initialize the value if this is the first resend and
    // either we never got an ack before or we are inserting into an empty resend queue

    if (makeCopyOfInternalPacket)
    {
        InternalPacket *pool=internalPacketPool.GetPointer();
        //printf("Adding %i\n", internalPacket->data);
        memcpy(pool, internalPacket, sizeof(InternalPacket));
    }
    else
    {
        RakAssert(internalPacket->nextActionTime!=0);

    }

}

//-------------------------------------------------------------------------------------------------------
// Generates a datagram (coalesced packets)
// Is taken from ReliabilityLayer.cpp
// Modified 
//-------------------------------------------------------------------------------------------------------
unsigned RakUtils::GenerateDatagram(
        RakNet::BitStream *output, 
        const InternalPacket &IPs,
        int MTUSize, 
        RakNetTimeNS time)
{
    InternalPacket * internalPacket;
    int maxDataBitSize;
    int reliableBits = 0;
    int nextPacketBitLength;
    unsigned i, messageHandlerIndex;
    bool isReliable, onlySendUnreliable;
    bool writeFalseToHeader;
    unsigned messagesSent=0;

    maxDataBitSize = MTUSize - 28;
    

    // Added.
    // Reset the output stream before writing to it.
    output->Reset();

    maxDataBitSize <<= 3;

    if(time > nextAckTime){
        if(acknowlegements.Size()>0){
            output->Write(true);
            messagesSent++;
            statistics.acknowlegementBitsSent += acknowlegements.Serialize(output, (MTUSize-28)*8-1, true);
            if(acknowlegements.Size()==0)
                nextAckTime=time+(RakNetTimeNS)((RakNetTime)(3.0/4.0f));

            writeFalseToHeader=false;
        } else
        {
            writeFalseToHeader=true;
            nextAckTime=time+(RakNetTimeNS)((RakNetTime)(3.0/4.0f));
        }
    }
    else
        writeFalseToHeader=true;


    onlySendUnreliable = false;
        
    internalPacket = (InternalPacket *)&IPs;

    nextPacketBitLength = getBSHeaderLen(internalPacket) + internalPacket->dataBitLength;
    if(unreliableTimeout!=0 &&
        (internalPacket->reliability==UNRELIABLE || internalPacket->reliability==UNRELIABLE_SEQUENCED) &&
        time > internalPacket->creationTime+(RakNetTimeNS)unreliableTimeout)
    {
        // Unreliable packets are deleted
        delete [] internalPacket->data;
        internalPacketPool.ReleasePointer(internalPacket);
        goto END_OF_GENERATE_FRAME;
    }


    if(internalPacket->reliability == RELIABLE || internalPacket->reliability == RELIABLE_SEQUENCED || internalPacket->reliability == RELIABLE_ORDERED)
        isReliable = true;
    else
        isReliable = false;

    // Write to the output bitstream	
    statistics.messagesSent[0]++;
    statistics.messageDataBitsSent[0] += internalPacket->dataBitLength;
    if(writeFalseToHeader)
    {
        output->Write(false);
        writeFalseToHeader=false;
    }
    statistics.messageTotalBitsSent[0] += signBSFromIP(
        output, internalPacket);

    //output->PrintBits();
    internalPacket->packetNumber=sendPacketCount;
    messagesSent++;


END_OF_GENERATE_FRAME:
    ;


    if(output->GetNumberOfBitsUsed()>0)
        sendPacketCount++;

    return messagesSent;
}


// Is taken from ReliabilityLayer.cpp
InternalPacket *RakUtils::getIPFromBS(
        RakNet::BitStream *bs, 
        RakNetTimeNS time) 
{

    bool bitStreamSucceeded;
    InternalPacket* internalPacket;

    if ( bs->GetNumberOfUnreadBits() < (int) sizeof( internalPacket->messageNumber ) * 8 )
        return 0; // leftover bits

    internalPacket = internalPacketPool.GetPointer();

    internalPacket->creationTime = time;

    //bitStream->AlignReadToByteBoundary();

    // Read the packet number (2 bytes)
    bitStreamSucceeded = bs->Read( internalPacket->messageNumber );



    if ( bitStreamSucceeded == false )
    {
        internalPacketPool.ReleasePointer( internalPacket );
        return 0;
    }


    if ( bitStreamSucceeded == false )
    {
        internalPacketPool.ReleasePointer( internalPacket );
        return 0;
    }

    // Read the PacketReliability. This is encoded in 3 bits
    unsigned char reliability;

    bitStreamSucceeded = bs->ReadBits( ( unsigned char* ) ( &( reliability ) ), 4 );

    internalPacket->reliability = ( const PacketReliability ) reliability;



    if ( bitStreamSucceeded == false )
    {
        internalPacketPool.ReleasePointer( internalPacket );
        return 0;
    }

    // If the reliability requires an ordering channel and ordering index, we read those.
    if ( internalPacket->reliability == UNRELIABLE_SEQUENCED || internalPacket->reliability == RELIABLE_SEQUENCED || internalPacket->reliability == RELIABLE_ORDERED )
    {
        // ordering channel encoded in 5 bits (from 0 to 31)
        bitStreamSucceeded = bs->ReadBits( ( unsigned char* ) & ( internalPacket->orderingChannel ), 5 );

        if ( bitStreamSucceeded == false )
        {
            internalPacketPool.ReleasePointer( internalPacket );
            return 0;
        }

        bitStreamSucceeded = bs->Read( internalPacket->orderingIndex );


        if ( bitStreamSucceeded == false )
        {
            internalPacketPool.ReleasePointer( internalPacket );
            return 0;
        }
    }

    // Read if this is a split packet (1 bit)
    bool isSplitPacket;

    bitStreamSucceeded = bs->Read( isSplitPacket );



    if ( bitStreamSucceeded == false )
    {
        internalPacketPool.ReleasePointer( internalPacket );
        return 0;
    }

    if ( isSplitPacket )
    {
        bitStreamSucceeded = bs->Read( internalPacket->splitPacketId );


        if ( bitStreamSucceeded == false )
        {
            internalPacketPool.ReleasePointer( internalPacket );
            return 0;
        }

        bitStreamSucceeded = bs->ReadCompressed( internalPacket->splitPacketIndex );
        if ( bitStreamSucceeded == false )
        {
            internalPacketPool.ReleasePointer( internalPacket );
            return 0;
        }

        bitStreamSucceeded = bs->ReadCompressed( internalPacket->splitPacketCount );

        if ( bitStreamSucceeded == false )
        {
            internalPacketPool.ReleasePointer( internalPacket );
            return 0;
        }
    }

    else
        internalPacket->splitPacketIndex = internalPacket->splitPacketCount = 0;

    unsigned short length;

    bitStreamSucceeded = bs->ReadCompressed( length );

    // Read into an unsigned short.  Otherwise the data would be offset too high by two bytes

    if ( bitStreamSucceeded == false )
    {
        internalPacketPool.ReleasePointer( internalPacket );
        return 0;
    }

    internalPacket->dataBitLength = length;
    if ( ! ( internalPacket->dataBitLength > 0 && BITS_TO_BYTES( internalPacket->dataBitLength ) < MAXIMUM_MTU_SIZE ) )
    {
        // 10/08/05 - internalPacket->data wasn't allocated yet
        //	delete [] inconvert indentation to spacesternalPacket->data;
        internalPacketPool.ReleasePointer( internalPacket );
        return 0;
    }

    // Allocate memory to hold our data
    internalPacket->data = new unsigned char [ BITS_TO_BYTES( internalPacket->dataBitLength ) ];
    //printf("Allocating %i\n",  internalPacket->data);

    // Set the last byte to 0 so if ReadBits does not read a multiple of 8 the last bits are 0'ed out
    internalPacket->data[ BITS_TO_BYTES( internalPacket->dataBitLength ) - 1 ] = 0;
    // Read the data the packet holds
    bitStreamSucceeded = bs->ReadAlignedBytes( ( unsigned char* ) internalPacket->data, BITS_TO_BYTES( internalPacket->dataBitLength ) );
    //bitStreamSucceeded = bitStream->ReadBits((unsigned char*)internalPacket->data, internalPacket->dataBitLength);
#ifdef _DEBUG

    // 10/08/05 - Disabled assert since this hits from offline packets
    //assert( bitStreamSucceeded );
#endif

    if ( bitStreamSucceeded == false )
    {
        delete [] internalPacket->data;
        internalPacketPool.ReleasePointer( internalPacket );
        return 0;
    }
    return internalPacket;
}