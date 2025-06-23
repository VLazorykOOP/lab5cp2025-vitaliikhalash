#include <Windows.h>
#include <string>
#include <array>
#include <tchar.h>

const std::wstring szWindowClass = L"DesktopApp";
const std::wstring szTitle = L"Sierpinski's Carpet";
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HWND hGroupBox;
HWND hLabelLevel;
HWND hEditLevel;
HWND hButtonDraw;

static HWND CreateControlElement(LPCTSTR className, LPCTSTR text, DWORD style, HWND parent, HINSTANCE hInst, HMENU id = nullptr)
{
	return CreateWindow(className, text, WS_VISIBLE | WS_CHILD | style,
		0, 0, 0, 0, parent, id, hInst, nullptr);
}

static void DrawGasket(HDC hdc, int level, float x, float y, float side)
{
	if (level <= 0)
		return;

	float sub = side / 3;

	RECT box = {
		(LONG)(x + sub),
		(LONG)(y + sub),
		(LONG)(x + 2 * sub),
		(LONG)(y + 2 * sub) };
	FillRect(hdc, &box, (HBRUSH)GetStockObject(WHITE_BRUSH));

	level--;
	DrawGasket(hdc, level, x, y, sub);
	DrawGasket(hdc, level, x + sub, y, sub);
	DrawGasket(hdc, level, x + 2 * sub, y, sub);
	DrawGasket(hdc, level, x, y + sub, sub);
	DrawGasket(hdc, level, x + 2 * sub, y + sub, sub);
	DrawGasket(hdc, level, x, y + 2 * sub, sub);
	DrawGasket(hdc, level, x + sub, y + 2 * sub, sub);
	DrawGasket(hdc, level, x + 2 * sub, y + 2 * sub, sub);
}

static bool IsValidInput(const TCHAR* str)
{
	if (!str || !_tcslen(str))
	{
		return false;
	}

	for (int i = 0; str[i] != _T('\0'); ++i)
	{
		if (!_istdigit(str[i]))
		{
			return false;
		}
	}

	return true;
}

static int GetLevelFromInput()
{
	std::array<wchar_t, 32> buffer{};

	GetWindowText(hEditLevel, buffer.data(), 16);

	if (wcslen(buffer.data()) == 0)
	{
		SetWindowText(hEditLevel, _T("0"));
		MessageBox(nullptr, _T("Error: Input is empty."), szTitle.c_str(), MB_OK | MB_ICONERROR);
		return 0;
	}
	else if (!IsValidInput(buffer.data()))
	{
		SetWindowText(hEditLevel, _T("0"));
		MessageBox(nullptr, _T("Error: Invalid input. Please enter a valid positive integer."), szTitle.c_str(), MB_OK | MB_ICONERROR);
		return 0;
	}

	int level = _ttoi(buffer.data());

	if (level > 8)
	{
		int result = MessageBox(nullptr,
			_T("Recursion level over 8 may cause performance issues.\nDo you want to continue?"),
			szTitle.c_str(),
			MB_OKCANCEL | MB_ICONWARNING);

		if (result == IDCANCEL)
		{
			SetWindowText(hEditLevel, _T("0"));
			return 0;
		}
	}

	return level;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex = {
		sizeof(WNDCLASSEX),
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		LoadIcon(hInstance, IDI_APPLICATION),
		LoadCursor(nullptr, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		nullptr,
		szWindowClass.data(),
		LoadIcon(hInstance, IDI_APPLICATION) };

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(nullptr, _T("Call to RegisterClassEx failed!"), szTitle.c_str(), NULL);
		return 1;
	}

	HWND hWnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		szWindowClass.c_str(),
		szTitle.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		800, 800,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	if (!hWnd)
	{
		MessageBox(nullptr, _T("Call to CreateWindow failed!"), szTitle.c_str(), NULL);
		return 1;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
		hGroupBox = CreateControlElement(TEXT("button"), TEXT("Recursion Levels"), BS_GROUPBOX, hWnd, hInst);
		hLabelLevel = CreateControlElement(TEXT("static"), TEXT("Level:"), 0, hWnd, hInst);
		hEditLevel = CreateControlElement(TEXT("edit"), TEXT("3"), WS_BORDER, hWnd, hInst);
		hButtonDraw = CreateControlElement(TEXT("button"), TEXT("Draw Fractal"), 0, hWnd, hInst, (HMENU)1);
		break;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == 1)
		{
			InvalidateRect(hWnd, nullptr, TRUE);
		}
		break;
	case WM_SIZE:
	{
		int width = LOWORD(lParam);
		MoveWindow(hGroupBox, width - 160, 20, 140, 100, TRUE);
		MoveWindow(hLabelLevel, width - 135, 50, 50, 20, TRUE);
		MoveWindow(hEditLevel, width - 85, 50, 40, 20, TRUE);
		MoveWindow(hButtonDraw, width - 140, 80, 100, 30, TRUE);
		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);

		auto centerX = static_cast<float>((clientRect.right - clientRect.left) / 2);
		auto centerY = static_cast<float>((clientRect.bottom - clientRect.top) / 2);
		float boxSize = 600;

		HDC hdc = BeginPaint(hWnd, &ps);

		RECT box = {
			static_cast<long>(centerX - boxSize / 2),
			static_cast<long>(centerY - boxSize / 2),
			static_cast<long>(centerX + boxSize / 2),
			static_cast<long>(centerY + boxSize / 2) };
		FillRect(hdc, &box, (HBRUSH)GetStockObject(BLACK_BRUSH));

		int level = GetLevelFromInput();
		DrawGasket(hdc, level,
			centerX - boxSize / 2,
			centerY - boxSize / 2,
			boxSize);

		EndPaint(hWnd, &ps);

		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
