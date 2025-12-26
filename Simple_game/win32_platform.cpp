#include <windows.h>
#include "utils.cpp"


global_variable bool running = true;
global_variable bool is_resizing = false;

HWND g_hwnd = nullptr;

struct Render_State {
	int height, width;
	void* memory;

	BITMAPINFO bitmap_info;
};
Render_State render_state = {};

#include "rendering_file.cpp"
#include "platform_common.cpp"
#include "game.cpp"


void resize_render_state(int width, int height)
{
	// Guard against minimized window
	if (width <= 0 || height <= 0)
		return;

	render_state.width = width;
	render_state.height = height;

	if (render_state.memory) {
		VirtualFree(render_state.memory, 0, MEM_RELEASE);
	}

	int size = width * height * sizeof(unsigned int);

	render_state.memory = VirtualAlloc(
		0,
		size,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_READWRITE
	);

	// Bitmap info
	render_state.bitmap_info.bmiHeader = {};
	render_state.bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	render_state.bitmap_info.bmiHeader.biWidth = width;
	render_state.bitmap_info.bmiHeader.biHeight = -height; // top-down bitmap
	render_state.bitmap_info.bmiHeader.biPlanes = 1;
	render_state.bitmap_info.bmiHeader.biBitCount = 32;
	render_state.bitmap_info.bmiHeader.biCompression = BI_RGB;
}

LRESULT CALLBACK window_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
		switch (uMsg)
		{
		case WM_CLOSE:
		case WM_DESTROY:
			running = false;
			return 0;

		case WM_ENTERSIZEMOVE:
			is_resizing = true;
			return 0;

		case WM_EXITSIZEMOVE:
			is_resizing = false;
			return 0;

		case WM_SIZE:
		{
			RECT rect;
			GetClientRect(hwnd, &rect);
			int width = rect.right - rect.left;
			int height = rect.bottom - rect.top;
			resize_render_state(width, height);
			return 0;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	// create a window class
	WNDCLASS window_class = {};

	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = window_callback;
	window_class.hInstance = hInstance;
	window_class.lpszClassName = L"GameWindowClass";
	// register class

	RegisterClass(&window_class);

	// create window

	g_hwnd = CreateWindow(window_class.lpszClassName, L"My First Game", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, 0, 0, hInstance, 0);
	HDC hdc = GetDC(g_hwnd);

	Input input = {};

	float delta_time = 0.016666f;
	LARGE_INTEGER frame_begin_time;
	QueryPerformanceCounter(&frame_begin_time);

	float performance_frequency;
	{
		LARGE_INTEGER perf;
		QueryPerformanceFrequency(&perf);
		performance_frequency = (float)perf.QuadPart;
	}


	while (running) {
		// input
		MSG message;

		for (int i = 0; i < BUTTON_COUNT; i++) {
			input.buttons[i].changed = false;
		}
		while (PeekMessage(&message, g_hwnd, 0, 0, PM_REMOVE)) {

			switch (message.message) {
				case WM_KEYUP:
				case WM_KEYDOWN: {
					u32 vk_code = (u32)message.wParam;
					bool is_down = ((message.lParam & (1 << 31)) == 0);


#define process_button(b, vk)\
case vk: {\
input.buttons[b].changed = is_down != input.buttons[b].is_down;\
input.buttons[b].is_down = is_down;\
} break;
					switch (vk_code) {
						process_button(BUTTON_UP, VK_UP);
						process_button(BUTTON_DOWN, VK_DOWN);
						process_button(BUTTON_W, 'W');
						process_button(BUTTON_S, 'S');
					}
				} break;
					
				default: {
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
			}

		}
		// simulate

		if (!is_resizing && render_state.memory)
		{
			simulate_game(&input, delta_time);
			StretchDIBits(
				hdc,
				0, 0,
				render_state.width,
				render_state.height,
				0, 0,
				render_state.width,
				render_state.height,
				render_state.memory,
				&render_state.bitmap_info,
				DIB_RGB_COLORS,
				SRCCOPY
			);

			LARGE_INTEGER frame_end_time;
			QueryPerformanceCounter(&frame_end_time);
			delta_time = (float)(frame_end_time.QuadPart - frame_begin_time.QuadPart) / performance_frequency;
			frame_begin_time = frame_end_time;
		}

	}

	return 0;

}

