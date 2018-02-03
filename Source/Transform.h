#ifndef _POSITIONCLASS_H_
#define _POSITIONCLASS_H_

#include <math.h>
#include <directxmath.h>

using namespace DirectX;

class Transform
{
public:
	Transform();
	Transform(const Transform&);
	~Transform();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	void SetPosition(XMFLOAT3);
	void SetRotation(XMFLOAT3);

	void GetPosition(float&, float&, float&);
	void GetRotation(float&, float&, float&);

	void GetPosition(XMFLOAT3&);
	void GetRotation(XMFLOAT3&);

	void SetFrameTime(float);

	void MoveForward(bool);
	void MoveBackward(bool);
	void MoveUpward(bool);
	void MoveDownward(bool);
	void TurnLeft(bool);
	void TurnRight(bool);
	void LookUpward(bool);
	void LookDownward(bool);

private:
	XMFLOAT3 _position;
	XMFLOAT3 _rotation;

	float _frameTime;

	float _forwardSpeed, _backwardSpeed;
	float _upwardSpeed, _downwardSpeed;
	float _leftTurnSpeed, _rightTurnSpeed;
	float _lookUpSpeed, _lookDownSpeed;
};

#endif