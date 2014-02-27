/*
 * ofxDepthStreamCompression.h
 *
 *  Created on: Feb 26, 2014
 *      Author: arturo
 */

#ifndef OFXDEPTHSTREAMCOMPRESSION_H_
#define OFXDEPTHSTREAMCOMPRESSION_H_

#include "ofPixels.h"
#include "ofxDepthCompressedFrame.h"

class ofxDepthStreamCompression {
public:
	void setup(int w, int h);
	ofxDepthCompressedFrame newFrame(ofShortPixels & depth, float pixel_size, float distance);

private:
	ofxDepthCompressedFrame lastKeyFrame;
	ofxDepthCompressedFrame lastFrame;
	unsigned long long timeLastKeyFrame;
};

#endif /* OFXDEPTHSTREAMCOMPRESSION_H_ */
