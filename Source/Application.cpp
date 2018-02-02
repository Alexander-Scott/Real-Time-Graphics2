#include "Application.h"

Application::Application()
{
	_input = 0;
	_dx11Instance = 0;
	_timer = 0;
	_shaderManager = 0;
	_scene = 0;
}

Application::Application(const Application& other)
{
}

Application::~Application()
{
}

bool Application::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	bool result;
	
	// Create the input object.
	_input = new Input;
	if (!_input)
	{
		return false;
	}

	// Initialize the input object.
	result = _input->Initialize(hinstance, hwnd, screenWidth, screenHeight);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the input object.", L"Error", MB_OK);
		return false;
	}

	// Create the Direct3D object.
	_dx11Instance = new DX11Instance;
	if(!_dx11Instance)
	{
		return false;
	}

	// Initialize the Direct3D object.
	result = _dx11Instance->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	// Create the shader manager object.
	_shaderManager = new ShaderManager;
	if(!_shaderManager)
	{
		return false;
	}

	// Initialize the shader manager object.
	result = _shaderManager->Initialize(_dx11Instance->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the shader manager object.", L"Error", MB_OK);
		return false;
	}

	// Create the timer object.
	_timer = new Timer;
	if(!_timer)
	{
		return false;
	}

	// Initialize the timer object.
	result = _timer->Initialize();
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the timer object.", L"Error", MB_OK);
		return false;
	}

	// Create the scene object.
	_scene = new Scene;
	if(!_scene)
	{
		return false;
	}

	// Initialize the scene object.
	result = _scene->Initialize(_dx11Instance, hwnd, screenWidth, screenHeight, SCREEN_DEPTH);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the zone object.", L"Error", MB_OK);
		return false;
	}

	return true;
}

void Application::Destroy()
{
	// Release the scene object.
	if(_scene)
	{
		_scene->Destroy();
		delete _scene;
		_scene = 0;
	}

	// Release the timer object.
	if(_timer)
	{
		delete _timer;
		_timer = 0;
	}

	// Release the shader manager object.
	if(_shaderManager)
	{
		_shaderManager->Destroy();
		delete _shaderManager;
		_shaderManager = 0;
	}

	// Release the dx11 object.
	if(_dx11Instance)
	{
		_dx11Instance->Destroy();
		delete _dx11Instance;
		_dx11Instance = 0;
	}

	// Release the input object.
	if(_input)
	{
		_input->Destroy();
		delete _input;
		_input = 0;
	}

	return;
}

bool Application::Update()
{
	bool result;

	// Update the system stats.
	_timer->Update();

	// Do the input frame processing.
	result = _input->Update();
	if(!result)
	{
		return false;
	}

	// Check if the user pressed escape and wants to exit the application.
	if(_input->IsEscapePressed() == true)
	{
		return false;
	}

	// Do the scene frame processing.
	result = _scene->Update(_dx11Instance, _input, _shaderManager, _timer->GetTime());
	if (!result)
	{
		return false;
	}

	return result;
}