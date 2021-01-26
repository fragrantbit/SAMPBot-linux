#include "samp_encr.h"
#include <stdint.h>
#include <cstring>
#include <time.h>
#include <stdlib.h>
#include <cstdio>
#include "tables.h"

void kyretardizeDatagram(unsigned char *buf, int len, int port, int unk)
{
    unsigned char bChecksum = 0;
    for(int i = 0; i < len; i++)
    {
        unsigned char bData = buf[i];
        bChecksum ^= bData & 0xAA;
    }
    encrBuffer[0] = bChecksum;

    unsigned char *buf_nocrc = &encrBuffer[1];
    memcpy(buf_nocrc, buf, len);
    for(int i = 0; i < len; i++)
    {
        buf_nocrc[i] = sampEncrTable[buf_nocrc[i]];
        if ( unk )
            buf_nocrc[i] ^= (uint8_t)(port ^ 0xCC);
        unk ^= 1u;
    }
}
void BIG_NUM_MUL(uint32_t in[5], uint32_t out[6], uint32_t factor)
{
    /*
        Based on TTMath library by Tomasz Sowa.
    */

    uint32_t src[5] = {0};

    
    for (int i = 0; i < 5; i++)
        src[i] = ((in[4-i]>>24) | ((in[4-i]<<8) & 0x00FF0000) | ((in[4-i]>>8) & 0x0000FF00) | (in[4-i]<<24));

    unsigned long tmp = 0;

    tmp = (uint64_t)(src[0])*(uint64_t)(factor);
    out[0] = tmp&0xFFFFFFFF;
    out[1] = tmp>>32;
    tmp = (uint64_t)(src[1])*(uint64_t)(factor) + (uint64_t)(out[1]);
    out[1] = tmp&0xFFFFFFFF;
    out[2] = tmp>>32;
    tmp = (uint64_t)(src[2])*(uint64_t)(factor) + (uint64_t)(out[2]);
    out[2] = tmp&0xFFFFFFFF;
    out[3] = tmp>>32;
    tmp = (uint64_t)(src[3])*(uint64_t)(factor) + (uint64_t)(out[3]);
    out[3] = tmp&0xFFFFFFFF;
    out[4] = tmp>>32;
    tmp = (uint64_t)(src[4])*(uint64_t)(factor) + (uint64_t)(out[4]);
    out[4] = tmp&0xFFFFFFFF;
    out[5] = tmp>>32;
    

    for (int i = 0; i < 12; i++)
    {
        unsigned char temp = ((unsigned char*)out)[i];
        ((unsigned char*)out)[i] = ((unsigned char*)out)[23 - i];
        ((unsigned char*)out)[23 - i] = temp;
    }
}



// Updated by ion
/*
    unsigned char out[6*4] = {0};


    for (int i = 0; i < 6*4; ++i)
        out[i] = alphanum[rand() % (sizeof(alphanum) - 1)];

    out[6*4] = 0;

    BIG_NUM_MUL((unsigned long*)out, (unsigned long*)out, factor);

    When the char array is declared, it might have any alignment,
    including alignment that wouldn't work for unsigned long or uint32_t.
    Later it's being treated as an unsigned long/uint32_t in BIG_NUM_MUL().
    
    That's where the issues are coming from.

    The workaround would be declaring an uint32_t array and point `out` to it.
    So whenever `out` is used, it uses the space occupied by the array of uint32_t.
    That's how it guarantees it has the correct alignment for uint32_t.
*/

int gen_gpci(char buf[64], uint32_t factor) /* by bartekdvd */
{	
    uint32_t buffer[6] = { 0 };
    unsigned char *out = (unsigned char *)buffer;

    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    for (int i = 0; i < 6*4; ++i)
        out[i] = alphanum[rand() % (sizeof(alphanum) - 1)];

    out[6*4] = 0;

    BIG_NUM_MUL((uint32_t *)out, (uint32_t *)out, factor);

    unsigned int notzero = 0;
    buf[0] = '0'; buf[1] = '\0';

    if (factor == 0) return 1;

    int pos = 0;
    for (int i = 0; i < 24; i++)
    {
        unsigned char tmp = out[i] >> 4;
        unsigned char tmp2 = out[i]&0x0F;
        
        if (notzero || tmp)
        {
            buf[pos++] = (char)((tmp > 9)?(tmp + 55):(tmp + 48));
            if (!notzero) notzero = 1;
        }

        if (notzero || tmp2)
        {
            buf[pos++] = (char)((tmp2 > 9)?(tmp2 + 55):(tmp2 + 48));
            if (!notzero) notzero = 1;
        }
    }
    buf[pos] = 0;

    return pos;
}

// CAnimManager::AddAnimation has been hooked by kye, but resolved jmp address isn't in samp.dll
const static unsigned char code_from_CAnimManager_AddAnimation[20] = {
	0xFF, 0x25, 0x34, 0x39, // gta_sa.exe + 0x4D3AA0
	0x4D, 0x00, 0x90, 0x90, // gta_sa.exe + 0x4D3AA4
	0x90, 0x90, 0x56, 0x57, // gta_sa.exe + 0x4D3AAC
	0x50, 0x8B, 0x44, 0x24, // gta_sa.exe + 0x4D3AA8
	0x14, 0x8D, 0x0C, 0x80  // gta_sa.exe + 0x4D3AB0
};

void SHA1(char *message, uint32_t *out)
{	
	uint32_t h0 = 0x67452301;
	uint32_t h1 = 0xEFCDAB89;
	uint32_t h2 = 0x98BADCFE;
	uint32_t h3 = 0x10325476;
	uint32_t h4 = 0xC3D2E1F0;

	uint32_t len = 0;
	unsigned long long bitlen = 0;
	
	while (message[len])
	{
		len++;
		bitlen += 8;
	}
	
	uint32_t complement = (55 - (len%56)) + 8*(((len+8)/64));
	uint32_t newlen = len + complement + 8 + 1;
	char *pMessage = new char[newlen];
	if (!pMessage)
		return;

	memcpy(pMessage, message, len);
	pMessage[len] = -128;
	memset(pMessage+len+1, 0, complement);

	*(unsigned long long*)&pMessage[len + 1 + complement] = endian_swap64(bitlen);

	uint32_t chunks = newlen/64;
	uint32_t w[80];

	for (uint32_t x = 0; x < chunks; x++)
	{
		for (uint32_t i = 0; i < 16; i++)
			w[i] = endian_swap32(*(uint32_t*)(&pMessage[x*64 + i*4]));

		memset(&w[16], 0, 64*4);

		for (uint32_t i = 16; i <= 79; i++)
			w[i] = ROTL((w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16]), 1);

		uint32_t a = h0;
		uint32_t b = h1;
		uint32_t c = h2;
		uint32_t d = h3;
		uint32_t e = h4;

		for (uint32_t i = 0; i <= 79; i++)
		{
			uint32_t f;
			uint32_t k;

			if (0 <= i && i <= 19)
			{
				f = (b & c) | ((~b) & d);
				k = 0x5A827999;
			}
			else if (20 <= i && i <= 39)
			{
				f = b ^ c ^ d;
				k = 0x6ED9EBA1;
			}
			else if (40 <= i && i <= 59)
			{
				f = (b & c) | (b & d) | (c & d);
				k = 0x8F1BBCDC;
			}
			else if (60 <= i && i <= 79)
			{
				f = b ^ c ^ d;
				k = 0xCA62C1D6;
			}

			uint32_t temp = (ROTL(a, 5) + f + e + k + w[i])&0xFFFFFFFF;
			e = d;
			d = c;
			c = ROTL(b, 30);
			b = a;
			a = temp;
		}

		h0 = (h0 + a)&0xFFFFFFFF;
		h1 = (h1 + b)&0xFFFFFFFF;
		h2 = (h2 + c)&0xFFFFFFFF;
		h3 = (h3 + d)&0xFFFFFFFF;
		h4 = (h4 + e)&0xFFFFFFFF;
	}

	delete [] pMessage;

	out[0] = h0;
	out[1] = h1;
	out[2] = h2;
	out[3] = h3;
	out[4] = h4;
}

void SHA1(char *message, char buf[64])
{
	if (!buf) return;
	uint32_t out[5];
	SHA1(message, out);
	snprintf(buf, 64, "%.8X%.8X%.8X%.8X%.8X", out[0], out[1], out[2], out[3], out[4]);
}



// after version change, may need to dump this again, but wasn't changed yet afaik
const static unsigned char auth_hash_transform_table[100] = {
	0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D,
	0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80,
	0x08, 0x06, 0x00, 0x00, 0x00, 0xE4, 0xB5, 0xB7, 0x0A, 0x00, 0x00, 0x00,
	0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x0B, 0x13, 0x00, 0x00, 0x0B,
	0x13, 0x01, 0x00, 0x9A, 0x9C, 0x18, 0x00, 0x00, 0x00, 0x04, 0x67, 0x41,
	0x4D, 0x41, 0x00, 0x00, 0xB1, 0x8E, 0x7C, 0xFB, 0x51, 0x93, 0x00, 0x00,
	0x00, 0x20, 0x63, 0x48, 0x52, 0x4D, 0x00, 0x00, 0x7A, 0x25, 0x00, 0x00,
	0x80, 0x83, 0x00, 0x00, 0xF9, 0xFF, 0x00, 0x00, 0x80, 0xE9, 0x00, 0x00,
	0x75, 0x30, 0x00, 0x00
};

unsigned char transform_auth_sha1(unsigned char value, unsigned char _xor)
{
	unsigned char  result = value;

	for(unsigned char  i = 0; i < 100; i++)
	{
		result = result ^ auth_hash_transform_table[i] ^ _xor;
	}

	return result;
}

char samp_sub_100517E0(unsigned char a1)
{
	char result = a1 + '0';

	if(a1 + '0' > '9')
	{
		result = a1 + '7';
	}

	return result;
}

void auth_stringify(char *out, unsigned char *hash)
{ 
	unsigned char i = 0;
	unsigned char *j = hash;

	do
	{
		out[i] = samp_sub_100517E0(*j >> 4); i++;
		out[i] = samp_sub_100517E0(*j & 0xF); i++;

		j++;
	}
	while(i < 40);

	out[i] = '\0';
}

void gen_auth_key(char buf[260], char *auth_in)
{
	char message[260];
	if(!auth_in) return;
	snprintf(message, 260, "%s", auth_in);

	uint32_t out[5];
	unsigned char *pb_out = (unsigned char *)&out;

	SHA1(message, out);

	for(unsigned char i = 0; i < 5; i++)
        pb_out[i] = transform_auth_sha1(pb_out[i], 0x2F); 

	for(unsigned char i = 5; i < 10; i++)  
        pb_out[i] = transform_auth_sha1(pb_out[i], 0x45); 

	for(unsigned char i = 10; i < 15; i++) 
        pb_out[i] = transform_auth_sha1(pb_out[i], 0x6F); 

	for(unsigned char i = 15; i < 20; i++) 
        pb_out[i] = transform_auth_sha1(pb_out[i], 0xDB); 

	for(unsigned char i = 0; i < 20; i++)
        pb_out[i] ^= code_from_CAnimManager_AddAnimation[i]; 

	auth_stringify(buf, pb_out);
}
