/*
 * ofxDepthStreamCompression.cpp
 *
 *  Created on: Feb 26, 2014
 *      Author: arturo
 */

#include "ofxDepthStreamCompression.h"

void ofxDepthStreamCompression::setup(int w, int h){
	lastKeyFrame.allocate(w,h,false);
	lastFrame.allocate(w,h,false);
	timeLastKeyFrame = 0;
}

ofxDepthCompressedFrame ofxDepthStreamCompression::newFrame(ofShortPixels & depth, float pixel_size, float distance){
	if(depth.getWidth()!=lastKeyFrame.getPixels().getWidth() || depth.getHeight()!=lastKeyFrame.getPixels().getHeight()){
		ofLogError() << "trying to compress frame of different size than allocated";
		return lastKeyFrame;
	}
	if(depth.getNumChannels()!=1){
		ofLogError() << "depth with more than 1 channel not supporded";
		return lastKeyFrame;
	}


	if(!lastKeyFrame.isKeyFrame() || ofGetElapsedTimeMillis()-timeLastKeyFrame>1000){
		memcpy(lastKeyFrame.getPixels().getPixels(),depth.getPixels(),depth.size()*sizeof(short));
		lastKeyFrame.setIsKeyFrame(true);
		lastKeyFrame.setRegistration(pixel_size,distance);
		timeLastKeyFrame = ofGetElapsedTimeMillis();
		return lastKeyFrame;
	}

	lastFrame.allocate(depth.getWidth(), depth.getHeight(), false);
	lastFrame.setRegistration(pixel_size,distance);
	int diffCount = 0;
	for(int i=0; i<depth.size(); i++){
		int diff = int(depth[i]) - int(lastKeyFrame.getPixels()[i]);
		float weightedDiff = abs(diff)/float(depth[i]+1);
		if(((depth[i]<2000 && weightedDiff>0.01) || weightedDiff>0.03) && abs(diff)<numeric_limits<short>::max()){
			diffCount ++;
			/*if(diffCount>depth.size()/10){
				memcpy(lastKeyFrame.getPixels().getPixels(),depth.getPixels(),depth.size()*sizeof(short));
				lastKeyFrame.setIsKeyFrame(true);
				lastKeyFrame.setRegistration(pixel_size,distance);
				timeLastKeyFrame = ofGetElapsedTimeMillis();
				return lastKeyFrame;
			}*/
			lastFrame.getPixels()[i] = diff;
		}else{
			lastFrame.getPixels()[i] = 0;
		}

	}
	return lastFrame;
}
