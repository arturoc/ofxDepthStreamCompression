/*
 * ofxDepthOctree.h
 *
 *  Created on: Apr 16, 2014
 *      Author: arturo
 */

#ifndef OFXDEPTHOCTREE_H_
#define OFXDEPTHOCTREE_H_

#include "ofConstants.h"
#include "ofVec3f.h"
#include "ofPixels.h"
#include "ofMesh.h"

struct ExternalBuffer{
	const char * buffer;
	size_t size;
};

class ofxDepthOctree;

class ofxDepthVoxel{
public:
	float x, y, z, side;
	vector<size_t> indices;
	ofxDepthOctree * parent;
	void allocate(int levels);
	bool inside(const ofVec3f & v);
	void divide(int levels);
	void fillMesh(ofMesh & mesh, int level);
	vector<ofxDepthVoxel> divisions;
	void serialize(ofBuffer & repr,int level);
	ExternalBuffer deserialize(const ExternalBuffer & repr, int levels);

	bool diff(const ofxDepthVoxel & in, ofxDepthVoxel & diff);
	bool isLeaf() const;

private:
	void createDivisions();
	ofVec2f worldToCamera(float x, float y, float z);
	ofVec2f worldToCamera(const ofVec3f & p);
};

class ofxDepthOctree {
public:
	ofxDepthOctree();
	virtual ~ofxDepthOctree();

	void setRegistration(float pixel_size, float distance);
	void allocate(ofShortPixels & depth, int levels);
	void update(ofShortPixels & depth, int levels);
	bool isAllocated();
	ofMesh mesh(int level);
	ofBuffer serialize(int level);
	void deserialize(const ofBuffer & repr, int levels);

	void diff(const ofxDepthOctree & in, ofxDepthOctree & diff);
private:
	ofVec3f cameraToWorld(int x, int y, int z);
	ofxDepthVoxel octree;
	float pixel_size;
	float distance;
	vector<ofVec3f> pc;
	float factor;
	friend class ofxDepthVoxel;
};

#endif /* OFXDEPTHOCTREE_H_ */
