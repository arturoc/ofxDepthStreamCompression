/*
 * ofxDepthOctree.cpp
 *
 *  Created on: Apr 16, 2014
 *      Author: arturo
 */

#include "ofxDepthOctree.h"


bool ofxDepthVoxel::inside(const ofVec3f & v){
	return v.x>x && v.x<x+side && v.y>y && v.y<y+side && v.z>z && v.z<z+side;
}

inline ofVec2f ofxDepthVoxel::worldToCamera(float x, float y, float z){
	double factor = z*parent->factor;
	return ofVec2f( x/factor+640/2, y/factor+480/2);
}

inline ofVec2f ofxDepthVoxel::worldToCamera(const ofVec3f & p){
	return worldToCamera(p.x,p.y,p.z);
}

void ofxDepthVoxel::createDivisions(){
	if(divisions.empty()){
		float half_side = side/2.0;
		divisions.resize(8);

		divisions[0].x = x;
		divisions[0].y = y;
		divisions[0].z = z;
		divisions[0].side =half_side;
		divisions[0].parent = parent;

		divisions[1].x = x+half_side;
		divisions[1].y = y;
		divisions[1].z = z;
		divisions[1].side =half_side;
		divisions[1].parent = parent;

		divisions[2].x = x+half_side;
		divisions[2].y = y;
		divisions[2].z = z+half_side;
		divisions[2].side =half_side;
		divisions[2].parent = parent;

		divisions[3].x = x;
		divisions[3].y = y;
		divisions[3].z = z+half_side;
		divisions[3].side =half_side;
		divisions[3].parent = parent;

		divisions[4].x = x;
		divisions[4].y = y+half_side;
		divisions[4].z = z;
		divisions[4].side =half_side;
		divisions[4].parent = parent;

		divisions[5].x = x+half_side;
		divisions[5].y = y+half_side;
		divisions[5].z = z;
		divisions[5].side =half_side;
		divisions[5].parent = parent;

		divisions[6].x = x+half_side;
		divisions[6].y = y+half_side;
		divisions[6].z = z+half_side;
		divisions[6].side =half_side;
		divisions[6].parent = parent;

		divisions[7].x = x;
		divisions[7].y = y+half_side;
		divisions[7].z = z+half_side;
		divisions[7].side =half_side;
		divisions[7].parent = parent;
	}else{
		for(size_t j=0;j<8;j++){
			divisions[j].indices.clear();
		}
	}
}

void ofxDepthVoxel::divide(int levels){
	if(indices.empty()) return;

	if((z<2000 && levels>0) || (levels>5)){
		createDivisions();

		for(size_t i=0;i<indices.size();i++){
			for(size_t j=0;j<8;j++){
				if(divisions[j].inside(parent->pc[indices[i]])){
					divisions[j].indices.push_back(indices[i]);
					break;
				}
			}
		}

		#pragma omp for schedule(static, 4)
		for(size_t i=0;i<8;i++){
			divisions[i].divide(levels-1);
		}
	}else{
		divisions.clear();
	}

}

void ofxDepthVoxel::fillMesh(ofMesh & mesh, int level){
	if(indices.empty()) return;
	if(level==0 || divisions.empty()){
		if(mesh.getMode()==OF_PRIMITIVE_POINTS){
			mesh.addVertex(ofVec3f(x+side*0.5,y+side*0.5,z+side*0.5));
			mesh.addTexCoord(worldToCamera(x+side*0.5,y+side*0.5,z+side*0.5));
		}else{
			int p = mesh.getNumVertices();
			mesh.addVertex(ofVec3f(x,y,z));
			mesh.addVertex(ofVec3f(x+side,y,z));
			mesh.addVertex(ofVec3f(x+side,y+side,z));
			mesh.addVertex(ofVec3f(x,y+side,z));
			mesh.addVertex(ofVec3f(x,y,z+side));
			mesh.addVertex(ofVec3f(x+side,y,z+side));
			mesh.addVertex(ofVec3f(x+side,y+side,z+side));
			mesh.addVertex(ofVec3f(x,y+side,z+side));

			mesh.addTriangle(p,p+1,p+2);
			mesh.addTriangle(p+2,p+3,p);
			mesh.addTriangle(p+1,p+5,p+2);
			mesh.addTriangle(p+2,p+6,p+5);
			mesh.addTriangle(p+5,p+4,p+6);
			mesh.addTriangle(p+6,p+7,p+4);
			mesh.addTriangle(p+4,p,p+3);
			mesh.addTriangle(p+3,p+7,p+4);
			mesh.addTriangle(p+3,p+2,p+6);
			mesh.addTriangle(p+6,p+7,p+3);
			mesh.addTriangle(p+0,p+1,p+5);
			mesh.addTriangle(p+5,p+4,p+0);

			mesh.addTexCoord(worldToCamera(mesh.getVertex(p)));
			mesh.addTexCoord(worldToCamera(mesh.getVertex(p+1)));
			mesh.addTexCoord(worldToCamera(mesh.getVertex(p+2)));
			mesh.addTexCoord(worldToCamera(mesh.getVertex(p+3)));
			mesh.addTexCoord(worldToCamera(mesh.getVertex(p+4)));
			mesh.addTexCoord(worldToCamera(mesh.getVertex(p+5)));
			mesh.addTexCoord(worldToCamera(mesh.getVertex(p+6)));
			mesh.addTexCoord(worldToCamera(mesh.getVertex(p+7)));
		}
	}else{
		for(size_t i=0;i<8;i++){
			divisions[i].fillMesh(mesh,level-1);
		}
	}
}


void ofxDepthVoxel::serialize(ofBuffer & repr, int level){
	if(level==0) return;
	unsigned char reprc = 0;
	for(size_t i= 0;i<divisions.size();i++){
		if(!divisions[i].indices.empty()) reprc |= (unsigned char)pow(2,i);
	}
	//if(reprc==255) cout << "error repr==255 level "<< level << endl;
	if(level!=0 && divisions.empty()) reprc = 255;
	repr.append((char*)&reprc,1);

	if(z>2000 && reprc==255) return;

	for(size_t i= 0;i<divisions.size();i++){
		if(!divisions[i].indices.empty()) divisions[i].serialize(repr,level-1);
	}
}


ExternalBuffer ofxDepthVoxel::deserialize(const ExternalBuffer & repr, int levels){
	if(levels==0) return repr;
	unsigned char reprc = (unsigned char)repr.buffer[0];
	ExternalBuffer rest = { repr.buffer + 1, repr.size - 1};
	if(rest.size==0) return rest;
	if(levels==0 || (reprc==255 && z>2000)){
	}else{
		createDivisions();
		for(size_t i=0;i<divisions.size();i++){
			if(reprc & (unsigned char)pow(2,i)){
				divisions[i].indices.push_back(0);
				rest = divisions[i].deserialize(rest,levels-1);
			}
		}
	}
	return rest;
}

bool ofxDepthVoxel::isLeaf() const{
	return divisions.empty() && !indices.empty();
}

bool ofxDepthVoxel::diff(const ofxDepthVoxel & in, ofxDepthVoxel & diff){
	if(isLeaf() && in.isLeaf()){
		diff.indices.clear();
		return false;
	}
	if(isLeaf() || in.isLeaf()){
		diff.indices.push_back(0);
		return true;
	}
	if(divisions.empty() != in.divisions.empty()){
		diff.indices.push_back(0);
		return true;
	}
	if(divisions.empty() && in.divisions.empty()){
		diff.indices.clear();
		return false;
	}
	bool childChanged = false;
	diff.createDivisions();
	for(size_t i=0;i<8;i++){
		childChanged |= divisions[i].diff(in.divisions[i],diff.divisions[i]);
	}
	if(childChanged){
		diff.indices.push_back(0);
	}else{
		diff.indices.clear();
	}

	return childChanged;
}

ofxDepthOctree::ofxDepthOctree() {
	// TODO Auto-generated constructor stub

}

ofxDepthOctree::~ofxDepthOctree() {
	// TODO Auto-generated destructor stub
}


void ofxDepthOctree::setRegistration(float pixel_size, float distance){
	this->pixel_size = pixel_size;
	this->distance = distance;
	factor = 2*pixel_size/distance;
}

void ofxDepthOctree::allocate(ofShortPixels & pixels, int levels){
	float min_x=std::numeric_limits<float>::max();
	float min_y=std::numeric_limits<float>::max();
	float min_z=std::numeric_limits<float>::max();
	float max_x = std::numeric_limits<float>::min();
	float max_y = std::numeric_limits<float>::min();
	float max_z = std::numeric_limits<float>::min();
	pc.resize(pixels.size()/4);
	octree.indices.resize(pixels.size()/4);
	for(int y=0, i=0, j=0;y<480;y+=2){
		for(int x=0;x<640;x+=2,i++,j+=2){
			ofVec3f world = cameraToWorld(x,y,pixels[j]);
			//if(world.z>500 && world.z<5000){
				min_x = std::min(min_x,world.x);
				min_y = std::min(min_y,world.y);
				min_z = std::min(min_z,world.z);
				max_x = std::max(max_x,world.x);
				max_y = std::max(max_y,world.y);
				max_z = std::max(max_z,world.z);
			//}
			pc[i] = world;
			octree.indices[i] = i;
		}
		j+=pixels.getWidth();
	}
	float w = max_x - min_x;
	float h = max_y - min_y;
	float d = max_z - min_z;
	octree.x = min_x;// + w*0.5;
	octree.y = min_y;// + h*0.5;
	octree.z = min_z;// + d*0.5;
	octree.side = std::max(std::max(w,h),d);
	octree.parent = this;

	octree.divide(levels);
}

void ofxDepthOctree::update(ofShortPixels & pixels, int levels){
	for(int y=0, i=0, j=0;y<480;y+=2){
		for(int x=0;x<640;x+=2,i++,j+=2){
			ofVec3f world = cameraToWorld(x,y,pixels[j]);
			pc[i] = world;
		}
		j+=pixels.getWidth();
	}
	octree.divide(levels);
}

bool ofxDepthOctree::isAllocated(){
	return !pc.empty();
}

inline ofVec3f ofxDepthOctree::cameraToWorld(int x, int y, int z){
	double factor = z*this->factor;
	return ofVec3f( (double)(x - 640/2) * factor,
					(double)(y - 480/2) * factor,
					z);
}

ofMesh ofxDepthOctree::mesh(int level){
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_POINTS);
	octree.fillMesh(mesh,level);
	return mesh;
}

ofBuffer ofxDepthOctree::serialize(int level){
	ofBuffer repr;
	/*stringstream sstr;
	sstr << octree.x << "," << octree.y << "," << octree.z << "," << octree.side;
	repr.append(sstr.str());*/
	octree.serialize(repr,level);
	return repr;
}

void ofxDepthOctree::deserialize(const ofBuffer & repr, int levels){
	ExternalBuffer ret = { repr.getBinaryBuffer(), repr.size()};
	octree.deserialize(ret,levels);
}

void ofxDepthOctree::diff(const ofxDepthOctree & in, ofxDepthOctree & diff){
	octree.diff(in.octree,diff.octree);
}
