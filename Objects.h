#pragma once
#include <vector>

using namespace std;
class Objects
{
private:
	float _id;
	float _angleX, _angleY;
	float _depth;
	float _translateX, _translateY;
	vector<float> _colors;
	vector<int> _faces;
	vector<float> _center;
	bool _isSelected;

public:
	Objects(int);
	~Objects(void);
	int getId(void){return _id;};
	vector<int> getFaces(void){ return _faces;};
	float getTranslationX(void){return _translateX;};
	float getTranslationY(void){return _translateY;};
	void selected(bool s){ _isSelected = s;};
	bool isSelected(){ return _isSelected;};
	vector<float> getCenter(void){return _center;};
	float getAngleX(void){return _angleX;};
	float getAngleY(void){return _angleY;};
	float getDepth(void){return _depth;};

	void addFace(int);
	void setCenter(float, float, float);
	void updateAngles(float, float);
	void updateDepth(float);
	void updateTranslation(float, float);

};

