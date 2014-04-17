/*
 * ofxDepthCompressedFrame.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: arturo
 */

#include "ofxDepthCompressedFrame.h"
#include "ofxCompress.h"


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
	compressed[1] = 160;
	compressed[2] = 120;

	pixels.allocate(compressed[1],compressed[2],1);
	if(isKeyFrame()){
		ofx_uncompress((unsigned char*)data+10,len-10,(unsigned char*)pixels.getPixels(),pixels.size()*sizeof(short));
	}else{
		pixels.set(0);
		if(len>10){
			ofx_uncompress((unsigned char*)data+10,len-10,uncompressedDiff);
			int lastPos = 0;
			for(size_t i=0; i<uncompressedDiff.size(); i++){
				int nextPos = lastPos+uncompressedDiff[i].pos;
				if(nextPos>=pixels.size()) break;
				pixels[nextPos] = uncompressedDiff[i].value;
				lastPos = nextPos;
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
