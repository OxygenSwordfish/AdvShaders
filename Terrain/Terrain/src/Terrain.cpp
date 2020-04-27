#include "Terrain.h"


//Terrain constructors
Terrain::Terrain(int widthIn, int heightIn, int stepSizeIn)
{
	width = widthIn;
	height = heightIn;
	stepSize = stepSizeIn;
	//makeVertices(&vertices, 0.0f, 0.0f);

}

Terrain::Terrain() {
	width = 50;
	height = 50;
	stepSize = 10;
	makeVertices(&vertices, 0.0f, 0.0f); //Modified for infinite generation, defaults to 0,0 coordinates

}


std::vector<float> Terrain::getVertices() {
	return vertices;
}

void Terrain::makeVertices(std::vector<float> *vertices, float xPos, float yPos) {
	/* triangle a b c
		   b
		   | \
		   a _ c


		 triangle d f e
		   f _ e
			 \ |
			   d

		 c == d
		 b == f
		 Duplicate vertices but easier in long run! (tesselation and LOD)

		a = (x,y,z)
		b = (x, y+1)
		c = (x+1,y)

		d = (x+1,y)
		e = (x, y+1)
		f = (x+1,y+1)

		 each vertex a, b,c, etc. will have 5 data:
		 x y z u v
		  */
	vertices->clear();

	for (int y = 0; y < height - 1; y++) {
		float  offSetY = yPos + (y * stepSize);
		for (int x = 0; x < width - 1; x++) {
			float offSetX = xPos + (x * stepSize);
			makeVertex(offSetX, offSetY, vertices);  // a
			makeVertex(offSetX, offSetY + stepSize, vertices);  // b
			makeVertex(offSetX + stepSize, offSetY, vertices);   // c
			makeVertex(offSetX + stepSize, offSetY, vertices);  //d
			makeVertex(offSetX, offSetY + stepSize, vertices);  //e
			makeVertex(offSetX + stepSize, offSetY + stepSize, vertices);  //f
		}
	}

}

void Terrain::makeVertex(int x, int y, std::vector<float> *vertices) {

	//x y z position
	vertices->push_back((float)x); //xPos
	vertices->push_back(0.0f); //yPos - always 0 for now. Going to calculate this on GPU - can change to calclaue it here.
	vertices->push_back((float)y); //zPos

   // add texture coords
	vertices->push_back((float)x / (width*stepSize));
	vertices->push_back((float)y / (height*stepSize));


}

bool Terrain::bounds(float x, float z, float dist)
{
	float posX = vertices.at(vertices.size() / 2); //X position
	float posZ = vertices.at((vertices.size() / 2)+ 2); //Z position

	if ((x > (posX + dist)) || (x < (posX - dist)))
		return true;
	if ((z > (posZ + dist)) || (z < (posZ - dist)))
		return true;

	return false;

}