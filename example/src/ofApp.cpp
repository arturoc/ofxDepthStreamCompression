#include "ofApp.h"
#include "assert.h"

#define DEPTH_FINAL_W 160
#define DEPTH_FINAL_H 120
#define SCALE_UP 4

//--------------------------------------------------------------
void ofApp::setup(){
	kinect.setRegistration(true);
	kinect.init();
	kinect.open();
	compressor.setup(DEPTH_FINAL_W,DEPTH_FINAL_H);
	tex.allocate(DEPTH_FINAL_W,DEPTH_FINAL_H,GL_R8);
	tex.setRGToRGBASwizzles(true);
	texDiff.allocate(DEPTH_FINAL_W,DEPTH_FINAL_H,GL_R8);
	texDiff.setRGToRGBASwizzles(true);
	keyPlusDiff.allocate(DEPTH_FINAL_W,DEPTH_FINAL_H,1);
	resizedRawDepth.allocate(DEPTH_FINAL_W,DEPTH_FINAL_H,1);
	timeOneSec = ofGetElapsedTimeMillis();
	sizeOneSec = 0;
	mesh.getVertices().resize(DEPTH_FINAL_W*DEPTH_FINAL_H);
	mesh.getTexCoords().resize(DEPTH_FINAL_W*DEPTH_FINAL_H);
	for (int y=0,i=0;y<DEPTH_FINAL_H;y++){
		for (int x=0;x<DEPTH_FINAL_W;x++,i++){
			mesh.getVertices()[i].set(x,y);
			mesh.getTexCoords()[i].set(x*SCALE_UP,y*SCALE_UP);
		}
	}
	for (int y=0;y<DEPTH_FINAL_H;y++){
		for (int x=0;x<DEPTH_FINAL_W;x++){
			mesh.addIndex(y*DEPTH_FINAL_W+x+1);
			mesh.addIndex(y*DEPTH_FINAL_W+x);
			mesh.addIndex((y+1)*DEPTH_FINAL_W+x);

			mesh.addIndex((y+1)*DEPTH_FINAL_W+x);
			mesh.addIndex((y+1)*DEPTH_FINAL_W+x+1);
			mesh.addIndex(y*DEPTH_FINAL_W+x+1);
		}
	}
	//mesh.setMode(OF_PRIMITIVE_POINTS);
	ofBackground(0);
	camera.setTarget(ofVec3f(0.0,0.0,-1000.0));
	showPointCloud = false;
	ofEnableDepthTest();

	shader.load("shader.vert","shader.frag","shader.geom");
	shader.begin();
	shader.setUniform1f("ref_pix_size",kinect.getZeroPlanePixelSize());
	shader.setUniform1f("ref_distance",kinect.getZeroPlaneDistance());
	shader.setUniform1f("SCALE_UP",SCALE_UP);
	shader.setUniform1f("max_distance",500);
	shader.end();
}

template<typename T>
void meshFromDepthPixels(ofPixels_<T> & pixels, ofVboMesh & mesh, ofxKinect & kinect){
	for (int y=0,i=0;y<pixels.getHeight();y++){
		for (int x=0;x<pixels.getWidth();x++,i++){
			mesh.getVertices()[i].z = ((unsigned short*)pixels.getPixels())[i];
		}
	}
}

//--------------------------------------------------------------
void ofApp::update(){
	kinect.update();
	if(kinect.isFrameNewDepth()){
		kinect.getRawDepthPixelsRef().resizeTo(resizedRawDepth,OF_INTERPOLATE_NEAREST_NEIGHBOR);
		ofxDepthCompressedFrame frame = compressor.newFrame(resizedRawDepth,kinect.getZeroPlanePixelSize(),kinect.getZeroPlaneDistance());

		lastFrame.fromCompressedData((const char*)&frame.compressedData()[0],frame.compressedData().size()*2);
		if(lastFrame.isKeyFrame()){
			lastKeyFrame = lastFrame;
		}else{
			for(int i=0;i<keyPlusDiff.size();i++){
				keyPlusDiff[i] = ((unsigned short)lastKeyFrame.getPixels()[i]) + lastFrame.getPixels()[i];
			}
			meshFromDepthPixels(keyPlusDiff,mesh,kinect);
		}
		tex.loadData(keyPlusDiff.getPixels(),DEPTH_FINAL_W,DEPTH_FINAL_H,GL_RED);
		texDiff.loadData((unsigned short*)lastFrame.getPixels().getPixels(),DEPTH_FINAL_W,DEPTH_FINAL_H,GL_RED);
		sizeOneSec += frame.compressedData().size()*2;
		uncompressedOneSec += kinect.getRawDepthPixelsRef().size()*2;
		framesOneSec++;
		unsigned long long now = ofGetElapsedTimeMillis();
		if(now - timeOneSec>=1000){
			cout << uncompressedOneSec/(float(now-timeOneSec)/1000.0)/1024.0 << "Kb/sec (uncompressed)" << endl;
			cout << sizeOneSec/(float(now-timeOneSec)/1000.0)/1024.0 << "Kb/sec" << endl;
			cout << framesOneSec/(float(now-timeOneSec)/1000.0) << "fps" << endl;
			cout << "ratio: " << float(sizeOneSec)/float(uncompressedOneSec) << endl;
			timeOneSec = now;
			sizeOneSec = 0;
			uncompressedOneSec = 0;
			framesOneSec = 0;
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	if(showPointCloud){
		glEnable(GL_CULL_FACE);
		shader.begin();
		camera.begin();
		kinect.getTextureReference().bind();
		mesh.drawWireframe();
		kinect.getTextureReference().unbind();
		camera.end();
		shader.end();
		glDisable(GL_CULL_FACE);
	}else{
		tex.draw(0,0);
		texDiff.draw(DEPTH_FINAL_W,0);
	}
	ofDrawBitmapString(ofToString(ofGetFrameRate()),20,20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	showPointCloud = !showPointCloud;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
