#define is_down(b) input->buttons[b].is_down
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)


// 5x7 bitmap font for digits 0–9
// Each row is a bitmask (left → right)
static const u8 digit_font[10][7] = {
	{0x1E,0x21,0x23,0x25,0x29,0x31,0x1E}, // 0
	{0x08,0x18,0x08,0x08,0x08,0x08,0x1C}, // 1
	{0x1E,0x21,0x01,0x0E,0x10,0x20,0x3F}, // 2
	{0x1E,0x21,0x01,0x0E,0x01,0x21,0x1E}, // 3
	{0x02,0x06,0x0A,0x12,0x3F,0x02,0x02}, // 4
	{0x3F,0x20,0x3E,0x01,0x01,0x21,0x1E}, // 5
	{0x0E,0x10,0x20,0x3E,0x21,0x21,0x1E}, // 6
	{0x3F,0x01,0x02,0x04,0x08,0x10,0x10}, // 7
	{0x1E,0x21,0x21,0x1E,0x21,0x21,0x1E}, // 8
	{0x1E,0x21,0x21,0x1F,0x01,0x02,0x1C}, // 9
};


float player_1_p, player_1_dp, player_2_p, player_2_dp;
float arena_half_size_x = 85, arena_half_size_y = 45;
float player_half_size_x = 2.5, player_half_size_y = 12;
float ball_p_x, ball_p_y, ball_dp_x = 100, ball_dp_y, ball_half_size = 1;

int player_1_score = 0, player_2_score = 0;

internal void
draw_digit(int digit, float x, float y, float scale, u32 color) {
	if (digit < 0 || digit > 9) return;

	for (int row = 0; row < 7; row++) {
		u8 bits = digit_font[digit][row];

		for (int col = 0; col < 5; col++) {
			if (bits & (1 << (4 - col))) {
				draw_rect(
					x + col * scale,
					y + row * scale,
					scale * 0.5f,
					scale * 0.5f,
					color
				);
			}
		}
	}
}

internal void
draw_number(int value, float x, float y, float scale, u32 color)
{
	if (value == 0) {
		draw_digit(0, x, y, scale, color);
		return;
	}

	float offset = 0;
	while (value > 0) {
		int digit = value % 10;
		draw_digit(digit, x - offset, y, scale, color);
		value /= 10;
		offset += scale * 6;
	}
}

internal void
simulate_player(float *p, float *dp, float ddp, float dt) {
	ddp -= *dp * 10.f;

	*p = *p + *dp * dt + ddp * dt * dt * .5f;
	*dp = *dp + ddp * dt;

	if (*p - player_half_size_y < -arena_half_size_y) {
		*p = -arena_half_size_y + player_half_size_y;
		*dp = 0;
	}
	else if (*p + player_half_size_y > arena_half_size_y) {
		*p = arena_half_size_y - player_half_size_y;
		*dp = 0;
	}
}

internal bool
aabb_collide(float p1x, float p1y, float hs1x, float hs1y,
	float p2x, float p2y, float hs2x, float hs2y) {
	return (p1x + hs1x > p2x - hs2x &&
		p1x - hs1x < p2x + hs2x &&
		p1y + hs1y > p2y - hs2y &&
		p1y + hs1y < p2y + hs2y);
}

internal void
simulate_game(Input* input, float dt) {
		clear_screen(0xff5500);
		draw_rect(0, 0, arena_half_size_x, arena_half_size_y, 0xffaa33);

		float player_1_ddp = 0.f;
		if (is_down(BUTTON_UP)) player_1_ddp -= 2000;
		if (is_down(BUTTON_DOWN)) player_1_ddp += 2000;

		float player_2_ddp = 0.f;
		if (is_down(BUTTON_W)) player_2_ddp -= 2000;
		if (is_down(BUTTON_S)) player_2_ddp += 2000;

		simulate_player(&player_1_p, &player_1_dp, player_1_ddp, dt);
		simulate_player(&player_2_p, &player_2_dp, player_2_ddp, dt);


		// simulating ball
		{
			ball_p_x += ball_dp_x * dt;
			ball_p_y += ball_dp_y * dt;


			if (aabb_collide(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 80, player_1_p,
				player_half_size_x, player_half_size_y)) {
				ball_p_x = 80 - player_half_size_x - ball_half_size;
				ball_dp_x *= -1;
				ball_dp_y = (ball_dp_y - player_1_p) * 2 + player_1_dp * .75f;
			}
			else if (aabb_collide(ball_p_x, ball_p_y, ball_half_size, ball_half_size, -80, player_2_p,
				player_half_size_x, player_half_size_y)) {
				ball_p_x = -80 + player_half_size_x + ball_half_size;
				ball_dp_x *= -1;
				ball_dp_y = (ball_dp_y - player_2_p) * 2 + player_2_dp * .75f;
			}

			if (ball_p_y + ball_half_size > arena_half_size_y) {
				ball_p_y = arena_half_size_y - ball_half_size;
				ball_dp_y *= -.5;
			}
			else if (ball_p_y - ball_half_size < -arena_half_size_y) {
				ball_p_y = -arena_half_size_y + ball_half_size;
				ball_dp_y *= -.5;
			}

			if (ball_p_x - ball_half_size > arena_half_size_x) {
				ball_dp_x *= -1;
				ball_dp_y = 0;
				ball_p_x = 0;
				ball_p_y = 0;
				player_1_score++;
				InvalidateRect(g_hwnd, nullptr, TRUE);
			}
			else if (ball_p_x - ball_half_size < -arena_half_size_x) {
				ball_dp_x *= -1;
				ball_dp_y = 0;
				ball_p_x = 0;
				ball_p_y = 0;
				player_2_score++;
				InvalidateRect(g_hwnd, nullptr, TRUE);
			}
		}




		draw_rect(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 0xffffff);
		draw_rect(80, player_1_p, player_half_size_x, player_half_size_y, 0xff0000);
		draw_rect(-80, player_2_p, player_half_size_x, player_half_size_y, 0x00ff22);

		// Draw scores (top of arena)
		draw_number(player_1_score, -arena_half_size_x + 5, -40, 1.f, 0xffffff);
		draw_number(player_2_score, arena_half_size_x - 10, -40, 1.f, 0xffffff);



}