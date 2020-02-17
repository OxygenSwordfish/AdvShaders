#include "Terrain.h"


//Terrain constructors
Terrain::Terrain(int widthIn, int heightIn, int stepSizeIn)
{
	width = widthIn;
	height = heightIn;
	stepSize = stepSizeIn;
	makeVertices(&vertices);

}

Terrain::Terrain() {
	width = 50;
	height = 50;
	stepSize = 10;
	makeVertices(&vertices);

}


std::vector<float> Terrain::getVertices() {
	return vertices;
}

void Terrain::makeVertices(std::vector<float> *vertices) {
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

	for (int y = 0; y < height - 1; y++) {
		float  offSetY = y * stepSize;
		for (int x = 0; x < width - 1; x++) {
			float offSetX = x * stepSize;
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