/*
 * ofxCompress.h
 *
 *  Created on: Apr 17, 2014
 *      Author: arturo
 */

#ifndef OFXCOMPRESS_H_
#define OFXCOMPRESS_H_

#define USE_GZIP 1

#if USE_GZIP
#include <zlib.h>

#define windowBits -15
#define GZIP_ENCODING 16

static void strm_init_deflate (z_stream * strm)
{
    strm->zalloc = Z_NULL;
    strm->zfree  = Z_NULL;
    strm->opaque = Z_NULL;
    deflateInit2 (strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                             windowBits, 9,
                             Z_FILTERED);
}

static void strm_init_inflate (z_stream * strm)
{
    strm->zalloc = Z_NULL;
    strm->zfree  = Z_NULL;
    strm->opaque = Z_NULL;
    inflateInit2 (strm, windowBits);
}

static size_t ofx_compress(const unsigned char* in, size_t size_in, unsigned char* out){
	z_stream strm;
	strm_init_deflate (& strm);
	strm.next_in = (unsigned char*)in;
	strm.avail_in = size_in;
	strm.avail_out = size_in;
	strm.next_out = out;
	deflate (& strm, Z_FINISH);
	deflateEnd(&strm);
	return size_in - strm.avail_out;
}

template<typename T>
static void ofx_uncompress(const unsigned char* in, size_t size_in, vector<T> & out){
	z_stream strm;
	strm_init_inflate (& strm);
	strm.next_in = (unsigned char*)in;
	strm.avail_in = size_in;
	if(out.size()==0) out.resize(size_in);
	unsigned char* next_out = (unsigned char*)&out[0];
	size_t size_out = out.size()*sizeof(T);
	do{
		strm.avail_out = size_out;
		strm.next_out = next_out;
		inflate (&strm, Z_FINISH);
		if(strm.avail_out!=0) break;
		out.resize(out.size()*2);
		next_out=(unsigned char*)&out[out.size()/2];
		size_out = (out.size()/2)*sizeof(T);
	}while(true);
	inflateEnd(&strm);
	out.resize( out.size() - strm.avail_out/sizeof(T));
}

template<typename T>
static void ofx_uncompress(const unsigned char* in, size_t size_in, T* out, size_t size_out){
	z_stream strm;
	strm_init_inflate (& strm);
	strm.next_in = (unsigned char*)in;
	strm.avail_in = size_in;
	strm.avail_out = size_out;
	strm.next_out = (unsigned char*)out;
	inflate (& strm, Z_FINISH);
	inflateEnd(&strm);
}
#else

#include "snappy.h"
static size_t ofx_compress(const unsigned char* in, size_t size_in, unsigned char* out){
	size_t compressedBytes;
	snappy::RawCompress((char*)in,size_in,(char*)out,&compressedBytes);
	return compressedBytes;
}

template<typename T>
static void ofx_uncompress(const unsigned char* in, size_t size_in, vector<T> & out){
	size_t uncompressed_len;
	snappy::GetUncompressedLength((char*)in,size_in,&uncompressed_len);
	out.resize(uncompressed_len/sizeof(T));
	snappy::RawUncompress((char*)in,size_in,(char*)&out[0]);
}

template<typename T>
static void ofx_uncompress(const unsigned char* in, size_t size_in, T* out, size_t size_out){
	snappy::RawUncompress((char*)in,size_in,(char*)out);
}
#endif


#endif /* OFXCOMPRESS_H_ */
