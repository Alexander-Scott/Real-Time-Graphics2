#pragma once

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <dinput.h>

class Input
{
public:
	Input();
	~Input();

	bool Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
	void Destroy();
	bool Update();

	bool IsEscapePressed();
	void GetMouseLocation(int& mouseX, int& mouseY);

	bool IsLeftPressed();
	bool IsRightPressed();
	bool IsUpPressed();
	bool IsDownPressed();
	bool IsAPressed();
	bool IsZPressed();
	bool IsPgUpPressed();
	bool IsPgDownPressed();

	bool IsF1Toggled();
	bool IsF2Toggled();
	bool IsF3Toggled();

private:
	bool ReadKeyboard();
	bool ReadMouse();
	void ProcessInput();

private:
	IDirectInput8*			_directInput;
	IDirectInputDevice8*	_keyboard;
	IDirectInputDevice8*	_mouse;

	unsigned char			_keyboardState[256];
	DIMOUSESTATE			_mouseState;

	int						_screenWidth, _screenHeight;
	int						_mouseX, _mouseY;

	bool					_f1Released;
	bool					_f2Released;
	bool					_f3Released;
};