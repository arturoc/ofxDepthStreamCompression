/*
 * ofxDepthCompressedFrame.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: arturo
 */

#include "ofxDepthCompressedFrame.h"

#define USE_GZIP 0

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

void ofxDepthCompressedFrame::allocate(int w, int h, bool isKeyFrame){
	pixels.allocate(w,h,1);
	compressed.resize(w*h*3+5);
	compressed[0] = isKeyFrame;
	compressed[1] = w;
	compressed[2] = h;
	compressed[3] = 1024;
	compressed[4] = 120;
	compressedDirty = true;
}

void ofxDepthCompressedFrame::setRegistration(float pixel_size, float distance){
	compressed[3] = CLAMP(pixel_size*10000,numeric_limits<short>::min(),numeric_limits<short>::max());
	compressed[4] = CLAMP(distance*10,numeric_limits<short>::min(),numeric_limits<short>::max());
}

float ofxDepthCompressedFrame::getPixelSize(){
	return compressed[3]/10000.0;
}

float ofxDepthCompressedFrame::getDistance(){
	return compressed[4]/10.0;
}

void ofxDepthCompressedFrame::fromCompressedData(const char* data, size_t len){
	const short * shortdata = (const short*)data;
	compressedDirty = true;
	compressed.resize(5);
	compressed[0] = shortdata[0];
	compressed[1] = shortdata[1];
	compressed[2] = shortdata[2];
	compressed[3] = shortdata[3];
	compressed[4] = shortdata[4];


	//FIXME: check that size is correct
	compressed[1] = shordata[1] = 640;
	compressed[2] = shordata[2] = 480;

	pixels.allocate(shortdata[1],shortdata[2],1);
	if(isKeyFrame()){
		ofx_uncompress((unsigned char*)data+10,len-10,(unsigned char*)pixels.getPixels(),pixels.size()*sizeof(short));
	}else{
		pixels.set(0);
		if(len>10){
			ofx_uncompress((unsigned char*)data+10,len-10,uncompressedDiff);
			int lastPos = 0;
			for(size_t i=0; i<uncompressedDiff.size(); i++){
				pixels[lastPos+uncompressedDiff[i].pos] = uncompressedDiff[i].value;
				lastPos += uncompressedDiff[i].pos;
			}
		}
	}
}

vector<ofxDepthCompressedFrame::DiffPixel> & ofxDepthCompressedFrame::getUncompressedDiff(){
	return uncompressedDiff;
}

const ofPixels_<short> & ofxDepthCompressedFrame::getPixels() const{
	return pixels;
}

ofPixels_<short> & ofxDepthCompressedFrame::getPixels(){
	compressedDirty = true;
	return pixels;
}

void ofxDepthCompressedFrame::setIsKeyFrame(bool isKeyFrame){
	compressed[0] = isKeyFrame;
}

bool ofxDepthCompressedFrame::isKeyFrame() const{
	return compressed[0];
}

const vector<short> & ofxDepthCompressedFrame::compressedData(){
	if(compressedDirty){
		compressed.resize(getPixels().size()*2+5);
		if(!isKeyFrame()){
			uncompressedDiff.clear();
			int lastPos = 0;
			for(int i=0;i<pixels.size();i++){
				int pos = i-lastPos;
				if(pixels[i]==0 && pos < std::numeric_limits<unsigned short>::max()) continue;
                DiffPixel diffPixel={(unsigned short)pos,pixels[i]};
				uncompressedDiff.push_back(diffPixel);
				lastPos = i;
			}
			if(uncompressedDiff.empty()){
				compressedBytes = 0;
			}else{
				compressedBytes = ofx_compress((unsigned char*)&uncompressedDiff[0],uncompressedDiff.size()*sizeof(DiffPixel),(unsigned char*)&compressed[5]);
			}
		}else{
			compressedBytes = ofx_compress((unsigned char*)pixels.getPixels(),pixels.size()*sizeof(short),(unsigned char*)&compressed[5]);
		}
		compressedDirty = false;
		compressed.resize(compressedBytes/2+5);
	}
	return compressed;
}
