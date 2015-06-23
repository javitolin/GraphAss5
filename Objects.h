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
	void addFace(int);
	int getId(void);
	std::vector<int> getFaces(void);
	void setCenter(float, float, float);
	std::vector<float> getCenter(void);
	void updateAngles(float, float);
	float getAngleX(void);
	float getAngleY(void);
	void updateDepth(float);
	float getDepth(void);
	void updateTranslation(float, float);
	float getTranslationX(void);
	float getTranslationY(void);
	void selected(bool s){ _isSelected = s;};
	bool isSelected(){ return _isSelected;};
};

