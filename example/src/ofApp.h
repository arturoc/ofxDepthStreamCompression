#pragma once

#include "ofMain.h"
#include "ofxDepthStreamCompression.h"
#include "ofxKinect.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		ofxKinect kinect;
		ofxDepthStreamCompression compressor;
		ofxDepthCompressedFrame lastKeyFrame;
		ofxDepthCompressedFrame lastFrame;
		ofShortPixels keyPlusDiff;
		ofShortPixels resizedRawDepth;
		ofTexture tex,texDiff;
		unsigned long long timeOneSec;
		size_t sizeOneSec, uncompressedOneSec, framesOneSec;
		ofEasyCam camera;
		ofVboMesh mesh;
		bool showPointCloud;

		ofShader shader;
};
