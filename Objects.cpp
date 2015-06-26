#include "Objects.h"
using namespace std;

Objects::Objects(int id)
{
	_id = id;
	_angleX = 0;
	_angleY = 0;
	_depth = 0;
	_translateX = 0;
	_translateY = 0;
	_isSelected = false;
}

Objects::~Objects(void)
{}

void Objects::addFace(int eIndex)
{
	_faces.push_back(eIndex);
}
void Objects::setCenter(float x, float y, float z)
{
	_center.clear();
	_center.push_back(x);
	_center.push_back(y);
	_center.push_back(z);
}

void Objects::updateAngles(float x, float y)
{
	_angleX += x;
	_angleY += y;
	if(_angleX > 360 || _angleX < -360)
		_angleX = 0;
	if(_angleY > 360 || _angleY < -360)
		_angleY = 0;
}

void Objects::updateDepth(float depth)
{
	_depth += depth;
}

void Objects::updateTranslation(float x, float y)
{
	_translateX += x;
	_translateY += y;
}
