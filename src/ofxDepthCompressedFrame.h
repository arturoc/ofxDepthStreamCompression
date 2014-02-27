/*
 * ofxDepthCompressedFrame.h
 *
 *  Created on: Feb 26, 2014
 *      Author: arturo
 */

#ifndef OFXDEPTHCOMPRESSEDFRAME_H_
#define OFXDEPTHCOMPRESSEDFRAME_H_

#include "ofPixels.h"

class ofxDepthCompressedFrame {
public:
	void allocate(int w, int h, bool isKeyFrame);
	void setRegistration(float pixel_size, float distance);
	ofPixels_<short> & getPixels();
	const ofPixels_<short> & getPixels() const;
	float getPixelSize();
	float getDistance();
	void setIsKeyFrame(bool isKeyFrame);
	bool isKeyFrame() const;

	const vector<short> & compressedData();
	void fromCompressedData(const char* data, size_t len);

private:
	struct DiffPixel{
		unsigned short pos;
		short value;
	};
	ofPixels_<short> pixels;
	vector<short> compressed;
	vector<DiffPixel> uncompressedDiff;
	bool compressedDirty;
	size_t compressedBytes;
};

#endif /* OFXDEPTHCOMPRESSEDFRAME_H_ */
