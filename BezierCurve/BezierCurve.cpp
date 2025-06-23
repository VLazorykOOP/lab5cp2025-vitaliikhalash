#include <Windows.h>
#include <string>
#include <array>
#include <tchar.h>

const std::wstring szWindowClass = L"DesktopApp";
const std::wstring szTitle = L"Cubic Bezier Curve";
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HWND hGroupBox;
std::array<std::array<HWND, 2>, 4> hLabelPoints{};
std::array<std::array<HWND, 2>, 4> hEditPoints{};
HWND hButtonDraw;

static HWND CreateControlElement(LPCTSTR className, LPCTSTR text, DWORD style, HWND parent, HINSTANCE hInst, HMENU id = nullptr)
{
	return CreateWindow(className, text, WS_VISIBLE | WS_CHILD | style,
		0, 0, 0, 0, parent, id, hInst, nullptr);
}

struct Point
{
	float x;
	float y;
	Point(float x = 0, float y = 0) : x(x), y(y) {}
};

static Point CalculateCubicBezier(float t, const Point& p0, const Point& p1, const Point& p2, const Point& p3)
{
	float u = 1 - t;
	float tt = t * t;
	float uu = u * u;
	float uuu = uu * u;
	float ttt = tt * t;

	return Point(
		uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x,
		uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y);
}

static void DrawBezierCurve(HDC hdc, const Point& p0, const Point& p1, const Point& p2, const Point& p3)
{
	Point prev = p0;

	for (int i = 0; i <= 100; ++i)
	{
		float t = static_cast<float>(i) / 100;
		Point current = CalculateCubicBezier(t, p0, p1, p2, p3);
		MoveToEx(hdc, static_cast<int>(prev.x), static_cast<int>(prev.y), nullptr);
		LineTo(hdc, static_cast<int>(current.x), static_cast<int>(current.y));
		prev = current;
	}

	MoveToEx(hdc, static_cast<int>(p0.x), static_cast<int>(p0.y), nullptr);
	LineTo(hdc, static_cast<int>(p1.x), static_cast<int>(p1.y));
	LineTo(hdc, static_cast<int>(p2.x), static_cast<int>(p2.y));
	LineTo(hdc, static_cast<int>(p3.x), static_cast<int>(p3.y));

	auto hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
	auto hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	std::array<Point, 4> points{ p0, p1, p2, p3 };
	for (int i = 0; i < 4; ++i)
	{
		Ellipse(hdc,
			static_cast<int>(points[i].x) - 5, static_cast<int>(points[i].y) - 5,
			static_cast<int>(points[i].x) + 5, static_cast<int>(points[i].y) + 5);
	}
	SelectObject(hdc, hOldBrush);
	DeleteObject(hBrush);
}

static bool IsValidInput(const TCHAR* str)
{
	bool hasDecimalPoint = false;

	if (!str || !_tcslen(str))
	{
		return false;
	}

	for (int i = 0; str[i] != _T('\0'); ++i)
	{
		if (_istdigit(str[i]))
		{
			continue;
		}

		if (str[i] == _T('.') && !hasDecimalPoint)
		{
			continue;
		}

		return false;
	}

	return true;
}

static void GetPointsFromInput(Point points[4])
{
	std::array<wchar_t, 32> buffer{};

	for (int i = 0; i < 4; ++i)
	{
		GetWindowText(hEditPoints[i][0], buffer.data(), 16);

		if (wcslen(buffer.data()) == 0)
		{
			points[i].x = 0;
			SetWindowText(hEditPoints[i][0], _T("0"));
			MessageBox(nullptr, _T("Error: Input is empty."), szTitle.c_str(), MB_OK | MB_ICONERROR);
		}
		else if (!IsValidInput(buffer.data()))
		{
			points[i].x = 0;
			SetWindowText(hEditPoints[i][0], _T("0"));
			MessageBox(nullptr, _T("Error: Invalid input. Please enter a valid number."), szTitle.c_str(), MB_OK | MB_ICONERROR);
		}
		else
		{
			points[i].x = static_cast<float>(_ttof(buffer.data()));
		}

		GetWindowText(hEditPoints[i][1], buffer.data(), 16);

		if (wcslen(buffer.data()) == 0)
		{
			points[i].y = 0;
			SetWindowText(hEditPoints[i][1], _T("0"));
			MessageBox(nullptr, _T("Error: Input is empty."), szTitle.c_str(), MB_OK | MB_ICONERROR);
		}
		else if (!IsValidInput(buffer.data()))
		{
			points[i].y = 0;
			SetWindowText(hEditPoints[i][1], _T("0"));
			MessageBox(nullptr, _T("Error: Invalid input. Please enter a valid number."), szTitle.c_str(), MB_OK | MB_ICONERROR);
		}
		else
		{
			points[i].y = static_cast<float>(_ttof(buffer.data()));
		}
	}
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
		nullptr,
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

		hGroupBox = CreateControlElement(TEXT("button"), TEXT("Control Points"), BS_GROUPBOX, hWnd, hInst);

		std::array<std::array<LPCTSTR, 2>, 4> labels = {
			{{TEXT("X0:"), TEXT("Y0:")},
			 {TEXT("X1:"), TEXT("Y1:")},
			 {TEXT("X2:"), TEXT("Y2:")},
			 {TEXT("X3:"), TEXT("Y3:")}} };
		std::array<std::array<LPCTSTR, 2>, 4> defaults = {
			{{TEXT("100"), TEXT("400")},
			 {TEXT("400"), TEXT("100")},
			 {TEXT("400"), TEXT("600")},
			 {TEXT("700"), TEXT("400")}} };

		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 2; ++j)
			{
				hLabelPoints[i][j] = CreateControlElement(TEXT("static"), labels[i][j], 0, hWnd, hInst);
				hEditPoints[i][j] = CreateControlElement(TEXT("edit"), defaults[i][j], WS_BORDER, hWnd, hInst);
			}
		}

		hButtonDraw = CreateControlElement(TEXT("button"), TEXT("Draw Curve"), 0, hWnd, hInst, (HMENU)1);

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
		int y = 50;

		MoveWindow(hGroupBox, width - 250, 20, 230, 190, TRUE);

		for (int i = 0; i < 4; ++i)
		{
			MoveWindow(hLabelPoints[i][0], width - 240, y, 40, 20, TRUE);
			MoveWindow(hEditPoints[i][0], width - 200, y, 60, 20, TRUE);
			MoveWindow(hLabelPoints[i][1], width - 130, y, 40, 20, TRUE);
			MoveWindow(hEditPoints[i][1], width - 90, y, 60, 20, TRUE);
			y += 30;
		}

		MoveWindow(hButtonDraw, width - 185, y, 100, 30, TRUE);

		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		std::array<Point, 4> pts{};
		GetPointsFromInput(pts.data());
		DrawBezierCurve(hdc, pts[0], pts[1], pts[2], pts[3]);
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
