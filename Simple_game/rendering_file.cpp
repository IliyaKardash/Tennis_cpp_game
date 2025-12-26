internal void render_background() {
	unsigned int* pixel = (unsigned int*)render_state.memory;
	for (int y = 0; y < render_state.height; y++) {
		for (int x = 0; x < render_state.width; x++) {
			*pixel++ = x - y;
		}
	}
}

internal void clear_screen(unsigned int color)
{
    if (!render_state.memory) return;

    unsigned int* pixel = (unsigned int*)render_state.memory;
    int count = render_state.width * render_state.height;

    for (int i = 0; i < count; i++) {
        pixel[i] = color;
    }
}

internal void draw_rect_in_pixels(int x0, int y0, int x1, int y1, u32 color)
{
    if (!render_state.memory) return;

    // Clamp
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > render_state.width)  x1 = render_state.width;
    if (y1 > render_state.height) y1 = render_state.height;

    if (x0 >= x1 || y0 >= y1) return;

    for (int y = y0; y < y1; y++)
    {
        u32* pixel =
            (u32*)render_state.memory + x0 + y * render_state.width;

        for (int x = x0; x < x1; x++) {
            *pixel++ = color;
        }
    }
}

global_variable float render_scale = 0.01f;


internal void draw_rect(float x, float y, float half_size_x, float half_size_y, u32 color) {

    x *= render_state.height*render_scale;
    y *= render_state.height*render_scale;
    half_size_y *= render_state.height* render_scale;
    half_size_x *= render_state.height* render_scale;

    x += render_state.width / 2.f;
    y += render_state.height / 2.f;


    // Change to pixels 
    int x0 = x - half_size_x;
    int x1 = x + half_size_x;
    int y0 = y - half_size_y;
    int y1 = y + half_size_y;
    draw_rect_in_pixels(x0, y0, x1, y1, color);
}
