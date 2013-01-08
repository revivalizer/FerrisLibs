#include <Fgl/Win32Window.h>

using namespace Fsl;

namespace Fgl
{
	Window *Window::Create(const String& title, int desiredWidth, int desiredHeight, bool fullscreen, Style style)
	{
		return new Win32Window(title, desiredWidth, desiredHeight, fullscreen, style);
	}

	int Win32Window::classRefCount = 0;
	String Win32Window::className = "Class";

	Win32Window::Win32Window(const String& title, int desiredWidth, int desiredHeight, bool fullscreen, Style style)
		: Window(title, desiredWidth, desiredHeight, fullscreen, style)
	{
		nativeStyle = 0;
		if ((int)style & (int)Style::Caption)nativeStyle |= WS_CAPTION;
		if ((int)style & (int)Style::SystemMenu)nativeStyle |= WS_SYSMENU;
		if ((int)style & (int)Style::MinimizeBox)nativeStyle |= WS_MINIMIZEBOX;
		instance = GetModuleHandle(nullptr);
		menu = nullptr;

		if (!classRefCount)
		{
			WNDCLASSEX wndClass =
			{
				sizeof(WNDCLASSEX),
				0,
				wndProcRouter,
				0, 0,
				instance,
				LoadIcon(NULL, IDI_APPLICATION),
				LoadCursor(NULL, IDC_ARROW),
				(HBRUSH) COLOR_WINDOW,
				NULL,
				className.GetData(),
				NULL
			};
			if (!RegisterClassEx(&wndClass)) throw FSL_EXCEPTION("Couldn't register window class");
		}
		classRefCount++;

		if (!fullscreen)
		{
			RECT rect = { 0, 0, desiredWidth, desiredHeight };
			AdjustWindowRect(&rect, nativeStyle, true);
			handle = CreateWindow(className.GetData(), title.GetData(), nativeStyle, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, instance, this);
		}
		else
		{
			// TODO
		}
		if (!handle) throw FSL_EXCEPTION("Couldn't create window");

		ShowWindow(handle, SW_SHOWNORMAL);
		SetSize(desiredWidth, desiredHeight);
		SetForegroundWindow(handle);
		SetFocus(handle);
	}

	Win32Window::~Win32Window()
	{
		::SetMenu(handle, nullptr);
		DestroyWindow(handle);
		classRefCount--;
		if (!classRefCount) UnregisterClass(className.GetData(), instance);
	}

	void Win32Window::Update()
	{
		MSG msg;
		while (PeekMessage(&msg, handle, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void Win32Window::SetSize(int desiredWidth, int desiredHeight)
	{
		this->desiredWidth = desiredWidth;
		this->desiredHeight = desiredHeight;
		RECT rect = { 0, 0, desiredWidth, desiredHeight };
		AdjustWindowRect(&rect, nativeStyle, menu != nullptr);
		SetWindowPos(handle, 0, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
	}

	void Win32Window::SetMenu(Menu *menu)
	{
		if (this->menu) this->menu->SetWindow(nullptr);
		this->menu = (Win32Menu *)menu;
		if (this->menu)
		{
			if (!::SetMenu(handle, this->menu->GetHandle())) throw FSL_EXCEPTION("Could not attach menu to window");
			this->menu->SetWindow(this);
		}
		else
		{
			if (!::SetMenu(handle, nullptr)) throw FSL_EXCEPTION("Could not detach menu from window");
		}
		SetSize(desiredWidth, desiredHeight);
	}

	HWND Win32Window::GetHandle() const
	{
		return handle;
	}

	LRESULT CALLBACK Win32Window::wndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		switch (Msg)
		{
		case WM_SIZE:
			width = LOWORD(lParam);
			height = HIWORD(lParam);
			OnResize(width, height);
			break;

		case WM_KEYDOWN:
			OnKeyDown(wParamToKey(wParam));
			break;

		case WM_KEYUP:
			OnKeyUp(wParamToKey(wParam));
			break;

		case WM_CLOSE:
			OnClose();
			break;

		case WM_COMMAND:
			if (menu) menu->HandleCommandMessage(wParam);
			break;

		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
		}
		return 0;
	}

	LRESULT CALLBACK Win32Window::wndProcRouter(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		Win32Window *window = nullptr;
		switch (Msg)
		{
		case WM_GETMINMAXINFO:
			return DefWindowProc(hWnd, Msg, wParam, lParam);

		case WM_NCCREATE:
			window = (Win32Window *)((LPCREATESTRUCT)lParam)->lpCreateParams;
			window->handle = hWnd;
			SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)window);
			break;

		default:
			window = (Win32Window *)GetWindowLong(hWnd, GWL_USERDATA);
		}
		if (!window) throw FSL_EXCEPTION("Window proc router invoked without a window handle");

		return window->wndProc(hWnd, Msg, wParam, lParam);
	}

	Key Win32Window::wParamToKey(WPARAM wParam)
	{
		if (wParam == VK_ESCAPE) return Key::Escape;
		if (wParam == VK_SPACE) return Key::Space;
		if (wParam >= 'A' && wParam <= 'Z') return (Key)(wParam - 'A' + (int)Key::A);
		if (wParam >= '0' && wParam <= '9') return (Key)(wParam - '0' + (int)Key::Number0);
		if (wParam >= VK_F1 && wParam <= VK_F12) return (Key)(wParam - VK_F1 + (int)Key::F1);
		if (wParam == VK_UP) return Key::Up;
		if (wParam == VK_DOWN) return Key::Down;
		if (wParam == VK_LEFT) return Key::Left;
		if (wParam == VK_RIGHT) return Key::Right;
		return Key::Unknown;
	}
}
