#ifndef SAMP_ENCR_H
#define SAMP_ENCR_H

#include <stdint.h>

#define endian_swap8(x) (x)
#define endian_swap16(x) ((x>>8) | (x<<8))
#define endian_swap32(x) ((x>>24) | ((x<<8) & 0x00FF0000) | ((x>>8) & 0x0000FF00) | (x<<24))
#define endian_swap64(x) ((x>>56) | ((x<<40) & 0x00FF000000000000) | \
        ((x<<24) & 0x0000FF0000000000) | ((x<<8)  & 0x000000FF00000000) | \
        ((x>>8)  & 0x00000000FF000000) | ((x>>24) & 0x0000000000FF0000) | \
        ((x>>40) & 0x000000000000FF00) | (x<<56))

#define ROTL(value, shift) ((value << shift) | (value >> (sizeof(value)*8 - shift)))
#define ROTR(value, shift) ((value >> shift) | (value << (sizeof(value)*8 - shift)))
#define swap(x,y,T) {T tmp = x; x = y; y = tmp;}


void kyretardizeDatagram(unsigned char *buf, int len, int port, int unk);
int gen_gpci(char buf[64], uint32_t factor);
void gen_auth_key(char buf[260], char* auth_in);

#endif