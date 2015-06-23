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

int Objects::getId(void)
{
	return _id;
}

void Objects::addFace(int eIndex)
{
	_faces.push_back(eIndex);
}

vector<int> Objects::getFaces(void)
{
	return _faces;
}

void Objects::setCenter(float x, float y, float z)
{
	_center.clear();
	_center.push_back(x);
	_center.push_back(y);
	_center.push_back(z);
}

vector<float> Objects::getCenter(void)
{
	return _center;
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

float Objects::getAngleX(void)
{
	return _angleX;
}

float Objects::getAngleY(void)
{
	return _angleY;
}

void Objects::updateDepth(float depth)
{
	_depth += depth;
}

float Objects::getDepth(void)
{
	return _depth;
}

void Objects::updateTranslation(float x, float y)
{
	_translateX += x;
	_translateY += y;
}

float Objects::getTranslationX(void)
{
	return _translateX;
}

float Objects::getTranslationY(void)
{
	return _translateY;
}
