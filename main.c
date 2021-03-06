
#include "raylib.h"
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>


#define ORIGIN_OFFSET     3.0f
#define STACKLEN            10
#define CAMERA_START_POS  6.0f
#define CAMERA_TARGET_POS 0.0f

#define SATUR 0.99f
#define VALUE 0.70f

typedef enum { MENU, PLAY, OVER, RESET, QUIT } state_t;
state_t state = MENU;

typedef struct {
	Vector3 position;
	float   width;
	float   height;
	float   length;
	Color   c_color;
	Color   l_color;
} cube_t;

typedef struct {
	cube_t  cube;
	Vector2 vec;
	float   ttl;
} ghost_t;

typedef enum { LEFT, RIGHT } direction_t;

Color block_colors[] = {
	GRAY   ,
	BROWN  ,
	LIME   ,
	BLUE   ,
	VIOLET ,
	RED    ,
};

char * color_names[] = {
	"GRAY  " ,
	"BROWN " ,
	"LIME  " ,
	"BLUE  " ,
	"VIOLET" ,
	"RED   " ,
};

ghost_t * destroying = NULL;
int des_count = 0;

bool check_cube_collision(cube_t top, cube_t last) {
	
	Rectangle rtop = {
		.x      = top.position.x,
		.y      = top.position.z,
		.width  = top.width,
		.height = top.length
	};
	
	Rectangle rlast = {
		.x      = last.position.x,
		.y      = last.position.z,
		.width  = last.width,
		.height = last.length
	};
	
	return CheckCollisionRecs(rtop, rlast);	
}

void draw_destroying(){
	
	if(des_count == 0)
		return;
	
	float speed = 1.0f;
	if (state == RESET)
		speed = 3.2f;
	
	for(int i = 0; i < des_count; i++){
		
		if(destroying[i].ttl > 3.0f)
			continue;
		
		DrawCube(
			destroying[i].cube.position,
			destroying[i].cube.width,
			destroying[i].cube.height,
			destroying[i].cube.length,
			destroying[i].cube.c_color
		);
		DrawCubeWires(
			destroying[i].cube.position,
			destroying[i].cube.width,
			destroying[i].cube.height,
			destroying[i].cube.length,
			destroying[i].cube.l_color
		);
		
		destroying[i].cube.position.x += destroying[i].vec.x * 0.002f * GetFrameTime() / 0.01667f;
		destroying[i].cube.position.z += destroying[i].vec.y * 0.002f * GetFrameTime() / 0.01667f;
		
		
		destroying[i].ttl += GetFrameTime() * speed;
		destroying[i].cube.c_color.a = (3.0f - destroying[i].ttl) / .05f;
		destroying[i].cube.l_color.a = destroying[i].cube.c_color.a * 4;
	}
	
	if(destroying[0].ttl > 3.0f) {
		des_count--;
		
		if(des_count > 0)
			memmove(destroying, &destroying[1], sizeof(destroying[0]) * (des_count) );
		
		if(des_count == 0){
			free(destroying);
			destroying = NULL;
		} else {
			destroying = realloc(destroying, sizeof(destroying[0]) * des_count);
		}
	}
	
}

void push_to_destroying(ghost_t d){
	des_count++;
	destroying = realloc(destroying, sizeof(d) * des_count);
	destroying[des_count - 1] = d;
}

cube_t chop_cubes(cube_t curr, cube_t base, direction_t origin) {
	
	Rectangle curr_rect = {
		.x      = curr.position.x - curr.width  / 2,
		.y      = curr.position.z - curr.length / 2,
		.width  = curr.width,
		.height = curr.length
	};
	
	Rectangle base_rect = {
		.x      = base.position.x - base.width  / 2,
		.y      = base.position.z - base.length / 2,
		.width  = base.width,
		.height = base.length
	};
	
	// check for quick return on perfect
	float diffx = base_rect.x - curr_rect.x;
	float diffy = base_rect.y - curr_rect.y;
	if (base_rect.x < curr_rect.x)
		diffx = curr_rect.x - base_rect.x;
	if (base_rect.y < curr_rect.y)
		diffy = curr_rect.y - base_rect.y;
	
	if (diffx < 0.05f && diffy < 0.05f) {
		printf("perfect!\n");
		// pretend they nailed it to the pixel
		curr.position.x = base.position.x;
		curr.position.z = base.position.z;
		return curr;
	}
	
	Rectangle newr = GetCollisionRec(curr_rect, base_rect);

	cube_t newc = {
		.position = { 
			.x = (base.position.x + curr.position.x) / 2,
			.y = curr.position.y,
			.z = (base.position.z + curr.position.z) / 2,
		},
		.width  = newr.width,
		.height = 0.5f,
		.length = newr.height,
		.c_color  = curr.c_color,
		.l_color  = curr.l_color,
	};
	
	// the next 40 lines could be compacted, but I find this more readable
	Vector2 c_ltmid = { curr_rect.x,                       curr_rect.y + curr_rect.height / 2};
	Vector2 c_rtmid = { curr_rect.x + curr_rect.width,     curr_rect.y + curr_rect.height / 2};
	Vector2 c_upmid = { curr_rect.x + curr_rect.width / 2, curr_rect.y };
	Vector2 c_dnmid = { curr_rect.x + curr_rect.width / 2, curr_rect.y + curr_rect.height};
	
	Vector2 b_ltmid = { base_rect.x,                       base_rect.y + base_rect.height / 2};
	Vector2 b_rtmid = { base_rect.x + base_rect.width,     base_rect.y + base_rect.height / 2};
	Vector2 b_upmid = { base_rect.x + base_rect.width / 2, base_rect.y };
	Vector2 b_dnmid = { base_rect.x + base_rect.width / 2, base_rect.y + base_rect.height};
	
	bool ltcol = CheckCollisionPointRec(b_ltmid, curr_rect);
	bool rtcol = CheckCollisionPointRec(b_rtmid, curr_rect);
	bool upcol = CheckCollisionPointRec(b_upmid, curr_rect);
	bool dncol = CheckCollisionPointRec(b_dnmid, curr_rect);
	
	Rectangle desr;
	if(origin == RIGHT && upcol){
		desr.x = b_upmid.x;
		desr.y = (b_upmid.y + c_upmid.y) / 2;
		desr.width = curr.width;
		desr.height = c_upmid.y - b_upmid.y;
	}
	if(origin == RIGHT && dncol){
		desr.x = b_dnmid.x;
		desr.y = (b_dnmid.y + c_dnmid.y) / 2;
		desr.width = curr.width;
		desr.height = c_dnmid.y - b_dnmid.y;
	}
	if(origin == LEFT  && ltcol){
		desr.x = (b_ltmid.x + c_ltmid.x) / 2;
		desr.y = c_ltmid.y;
		desr.width = c_ltmid.x - b_ltmid.x;
		desr.height = curr.length;
	}
	if(origin == LEFT  && rtcol){
		desr.x = (b_rtmid.x + c_rtmid.x) / 2;
		desr.y = c_rtmid.y;
		desr.width = c_rtmid.x - b_rtmid.x;
		desr.height = curr.length;
	}
	
	// vector for the ghost drift
	// ( .x = 1.0f || -1.0f, .y = 1.0f || -1.0f)
	Vector2 drift = {(ltcol * -1.0f) + (rtcol * 1.0f), (upcol * -1.0f) + (dncol * 1.0f)};
	
	ghost_t g = {
		.cube = {
			.position = {
				.x = desr.x,
				.y = curr.position.y,
				.z = desr.y,
			},
			.width   = desr.width,
			.height  = 0.5f,
			.length  = desr.height,
			.c_color.r = RAYWHITE.r,
			.c_color.g = RAYWHITE.g,
			.c_color.b = RAYWHITE.b,
			.c_color.a = 60,
			.l_color   = LIGHTGRAY
		},
		.ttl = 0,
		.vec = drift,
	};
	
	push_to_destroying(g);
	
	return newc;
}

void window_init(){

	const int screenWidth = 640;
	const int screenHeight = 480;
	
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(screenWidth, screenHeight, "STAX");
	
	// unset exit key
	SetExitKey(KEY_NULL);
	SetTargetFPS(60);
	SetRandomSeed(time(0));
	
}


// Define the camera to look into our 3d world
Camera3D camera;
float camera_blend_height;
float cam_radius;
Vector2 cam_angle;
unsigned int colorsize;
cube_t topblock;
cube_t stack[STACKLEN];
direction_t origin;
float direction;
int  points;
char point_txt[13];

Model   cyl;
Image   cyl_img;
Texture cyl_tex;
Vector3 cyl_pos;

void new_stack(){
	
	topblock = (cube_t){
		.position = { 0.0f, 0.25f, 0.0f },
		.width = 2.0f,
		.height = 0.5f,
		.length = 2.0f,
		//~ .c_color = block_colors[GetRandomValue(0, colorsize)],
		.c_color = ColorFromHSV(GetRandomValue(0, 360), SATUR, VALUE),
		.l_color = LIGHTGRAY,
	};

	origin = (origin + 1) % 2;
	if (origin == LEFT)
		topblock.position.x = -1.0f * ORIGIN_OFFSET;
	else
		topblock.position.z = -1.0f * ORIGIN_OFFSET;
	
	for(int i = 0; i < STACKLEN; i++){
		stack[i] = (cube_t){ // starting cube
			.position = (Vector3){0.0f, (float)(i) * -0.5f + -0.25f, 0.0f},
			.width =     2.0f,
			.height =    0.5f,
			.length =    2.0f,
			.c_color =   topblock.c_color,
			.l_color =   topblock.l_color,
		};
		if(i > 4)
			stack[i].c_color = BLACK;
		else {
			stack[i].c_color.r -= stack[i].c_color.r / 5 * (i + 1);
			stack[i].c_color.g -= stack[i].c_color.g / 5 * (i + 1);
			stack[i].c_color.b -= stack[i].c_color.b / 5 * (i + 1);
		}
	}
	
}

#define CYL_START_Y -13.0f

void round_init(){
	
	camera.position = (Vector3){
		CAMERA_START_POS, 
		CAMERA_START_POS,
		CAMERA_START_POS
	};  // Camera position
	camera.target = (Vector3){
		CAMERA_TARGET_POS,
		CAMERA_TARGET_POS,
		CAMERA_TARGET_POS
	};  // Camera looking at point
	camera.up = (Vector3){ 0.0f,1.0f, 0.0f };          // Camera up vector (rotation towards target)
	camera.fovy = 20.0f;                                // Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type
	
	camera_blend_height = camera.position.y;
	
	cam_radius = sqrt(
		pow(camera.position.x - camera.target.x, 2) +
		pow(camera.position.z - camera.target.z, 2)
	);
	cam_angle = (Vector2){ .x  = 45.0f, .y = 45.0f };
	
	SetCameraMode(camera, CAMERA_CUSTOM); // Set a free camera mode
	
	// TODO -- organize magic numbers
	cyl = LoadModelFromMesh(GenMeshCylinder(9, 20, 36));
	cyl_img = GenImageGradientH(640, 480, BLACK, WHITE);
	cyl_tex = LoadTextureFromImage(cyl_img);
	cyl.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = cyl_tex;
	cyl_pos = (Vector3){ 0.0f, CYL_START_Y, 0.0f };
	
	/* TODO -- try inverting shape itself instead of culling hack
	for(int i = 0; i < m.meshes[0].triangleCount; i++){
		int offset = i * 3;
		float v0 = m.meshes[0].vertices[offset + 0];
		float v1 = m.meshes[0].vertices[offset + 1];
		float v2 = m.meshes[0].vertices[offset + 2];
		m.meshes[0].vertices[offset + 0] = v2;
		m.meshes[0].vertices[offset + 1] = v1;
		m.meshes[0].vertices[offset + 2] = v0;
	}*/
	
	colorsize = sizeof(block_colors)/sizeof(block_colors[0]) - 1;
	
	for(int i = 0; i < colorsize + 1; i++){
		printf("%s --\n", color_names[i]);
		printf("\tRGB: { %03d, %03d, %03d }\n", block_colors[i].r, block_colors[i].g, block_colors[i].b);
		Vector3 hsv = ColorToHSV(block_colors[i]);
		printf("\tHSV: { %03f, %03f, %03f }\n", hsv.x, hsv.y, hsv.z);
	}
	
	origin = GetRandomValue(LEFT, RIGHT);
	direction = 1.0; // toward bottom
	
	new_stack();
	
	points = 0;
	
	sprintf(point_txt, "Points: %d", points);
	
	free(destroying);
	destroying = NULL;
	des_count = 0;
	
}

typedef enum { CLEAR, SCROLL } reset_state_t;
reset_state_t reset_s = CLEAR;
float clearttl = 2.55f;

bool update_reset(){
	
	// clearing
	if (reset_s == CLEAR && stack[0].c_color.a != 0){
		clearttl -= GetFrameTime() * 3;
		int reduceto  = (int)((clearttl) * 100.0f);
		if(reduceto < 1)
			reduceto = 0;
		
		for(int i = 0; i < STACKLEN; i++){
			stack[i].c_color.a = reduceto;
			stack[i].l_color.a = reduceto;
		}
		
		return false;
	} else if ( reset_s == CLEAR && stack[0].c_color.a == 0){
		// twice as high as start
		camera.position.x = CAMERA_START_POS;
		camera.position.y = CAMERA_START_POS + CAMERA_START_POS;
		camera.position.z = CAMERA_START_POS;
		camera.target.x   =                    CAMERA_TARGET_POS;
		camera.target.y   = CAMERA_START_POS + CAMERA_TARGET_POS;
		camera.target.z   =                    CAMERA_TARGET_POS;
		
		cam_radius = sqrt(
			pow(camera.position.x - camera.target.x, 2) +
			pow(camera.position.z - camera.target.z, 2)
		);
		cam_angle = (Vector2){ .x  = 45.0f, .y = 45.0f };
		
		//~ bg_pos.x          = -1.0f * CAMERA_START_POS;
		//~ bg_pos.y          = camera.position.y -  2 * CAMERA_START_POS;
		//~ bg_pos.z          = -1.0f * CAMERA_START_POS;
		cyl_pos.y         = camera.target.y + CYL_START_Y;
		new_stack();
		topblock.c_color.a = 0;
		topblock.l_color.a = 0;
		free(destroying);
		destroying = NULL;
		des_count = 0;
		clearttl = 2.55f;
		reset_s = SCROLL;
	}
	
	if (reset_s == SCROLL && camera.position.y > CAMERA_START_POS){
		camera.position.y -= GetFrameTime() / 0.1667f;
		camera.target.y   -= GetFrameTime() / 0.1667f;
		//~ bg_pos.y          -= GetFrameTime() / 0.1667f;
		cyl_pos.y          = camera.target.y + CYL_START_Y;
		
		if (points > 0)
			points *= (camera.position.y - CAMERA_START_POS + .01f) / CAMERA_START_POS;
		sprintf(point_txt, "Points: %d", points);
	} else if (reset_s == SCROLL && camera.position.y <= CAMERA_START_POS ){
		points = 0;
		camera.position.y = CAMERA_START_POS;
		camera.target.y   = CAMERA_TARGET_POS;
		//~ bg_pos.y  = -1.0f * CAMERA_START_POS;
		cyl_pos.y         = CYL_START_Y;
		camera_blend_height = camera.position.y;
		topblock.c_color.a = 255;
		topblock.l_color.a = 255;
		return true;
	}
	
	return false;
}

void render_reset(){
	BeginDrawing();

		ClearBackground(WHITE);
		
		BeginMode3D(camera);

		draw_stack();
		draw_destroying();
		draw_topblock();
		draw_skybox();
		
		//~ DrawGrid(10, 0.5f);
		
		EndMode3D();
		
		DrawText(point_txt, 10, 40, 20, BLACK);
		DrawFPS(10, 10);

	EndDrawing();
}

bool update_play(){
	// read stop input before here
	int keypress = -1;
	bool stop = false;
	while((keypress = GetKeyPressed())){
		if (keypress == KEY_ENTER) {
			stop = true;
		}
	}
	
	// L camera
	if (IsKeyDown(KEY_LEFT)){
		cam_angle.x += 1.0f * (GetFrameTime() / 0.01667f);
		if (cam_angle.x >= 360.0f)
			cam_angle.x -= 360.0f;
		
		camera.position.x = camera.target.x + cam_radius * cos(cam_angle.x * PI / 180);
		camera.position.z = camera.target.z + cam_radius * sin(cam_angle.x * PI / 180);
	}
	
	// R camera
	if (IsKeyDown(KEY_RIGHT)){
		cam_angle.x -= 1.0f * (GetFrameTime() / 0.01667f);
		if (cam_angle.x <= 0.0f)
			cam_angle.x = 360.f - cam_angle.x;
		
		camera.position.x = camera.target.x + cam_radius * cos(cam_angle.x * PI / 180);
		camera.position.z = camera.target.z + cam_radius * sin(cam_angle.x * PI / 180);
	}
	
	if(camera.position.y < camera_blend_height){
		camera.position.y += 0.01f * GetFrameTime() / 0.01667f;
		camera.target.y   += 0.01f * GetFrameTime() / 0.01667f;
		//~ bg_pos.y          += 0.01f * GetFrameTime() / 0.01667f;
		cyl_pos.y          = camera.target.y + CYL_START_Y;
	}
	
	// drop the block, add to stack
	if (stop) {
		
		if (check_cube_collision(topblock, stack[0])){
			
			topblock = chop_cubes(topblock, stack[0], origin);
			
		} else {			
			
			topblock.c_color = RAYWHITE;
			topblock.c_color.a = 0;
			topblock.l_color.a = 0;
			push_to_destroying((ghost_t){.cube = topblock, .ttl = 0, .vec = (Vector2){0, 0}});
			
			
			// Game Over
			//~ round_init();
			return true;
		}
		
		points++;
		sprintf(point_txt, "Points: %d", points);
		
		for(int i = 9; i > 0; i--){
			stack[i] = stack[i - 1];
		}
		stack[0] = topblock;
		
		camera_blend_height += 0.5f;
		
		topblock.position.y += 0.5f;
		
		// TODO -- as function
		origin = (origin + 1) % 2;
		if (origin == LEFT)
			topblock.position.x = -1.0f * ORIGIN_OFFSET;
		else
			topblock.position.z = -1.0f * ORIGIN_OFFSET;
		
		//~ topblock.c_color = block_colors[GetRandomValue(0, colorsize)];
		topblock.c_color = ColorFromHSV(GetRandomValue(0, 360), SATUR, VALUE);
	}
	
	// move along axis
	float * axis;
	if (origin == LEFT)
		axis = &topblock.position.x;
	else
		axis = &topblock.position.z;
	
	// move in respect to frame delta
	*axis += direction * 0.05f * GetFrameTime() / 0.01667f;
	
	// bounce off wall
	if ( *axis < (-1.0f * ORIGIN_OFFSET)  || *axis > ORIGIN_OFFSET ){
		*axis = direction * ORIGIN_OFFSET;
		direction *= -1.0f;
	}
	
	return false;
}

void draw_stack(){
	// stack
	for(int i = 0; i < 10; i++){
		DrawCube     (stack[i].position, stack[i].width, stack[i].height, stack[i].length, stack[i].c_color);
		DrawCubeWires(stack[i].position, stack[i].width, stack[i].height, stack[i].length, stack[i].l_color);
	}
}

void draw_topblock(){
	// topblock
	DrawCube     (topblock.position, topblock.width, topblock.height, topblock.length, topblock.c_color);
	DrawCubeWires(topblock.position, topblock.width, topblock.height, topblock.length, topblock.l_color);
}

void draw_skybox(){	
	// non-culled cylinder showed faster than billboard
	// and even flat texture draw to render surface
	// which seems weird, but k.
	rlDisableBackfaceCulling();
		DrawModel(cyl, cyl_pos, 1.0f, WHITE);
	rlEnableBackfaceCulling();
}

void render_play(){
	
	BeginDrawing();

		ClearBackground(WHITE);
		
		BeginMode3D(camera);
		
		draw_stack();
		draw_destroying();
		draw_topblock();
		draw_skybox();
		
		EndMode3D();
		
		DrawText(point_txt, 10, 40, 20, BLACK);
		DrawFPS(10, 10);

	EndDrawing();
}

bool update_over(){
	
	int keypress = -1;
	while((keypress = GetKeyPressed())){
		if (keypress == KEY_ENTER) {
			return true;
		}
	}
	
	return false;
}

void render_over(){
	BeginDrawing();

		ClearBackground(WHITE);
		
		BeginMode3D(camera);

		draw_stack();
		draw_destroying();
		draw_topblock();
		draw_skybox();
		
		EndMode3D();
		
		// TODO -- clean up
		DrawText("GAME OVER", 100, 100, 40, BLACK);
		DrawText(point_txt, 10, 40, 20, BLACK);
		DrawFPS(10, 10);

	EndDrawing();
}

// TODO -- clean up
int staxy = 100;
int _txty = 120;
int playy = 200;
int optiy = 300;
int exity = 400;
int alltx = 100;
int marky = 200;

typedef enum { START, OPTIONS, EXIT, MOD_END } menu_state_t;
menu_state_t menu_s = START;

state_t update_menu(){
	
	int keypress = -1;
	while((keypress = GetKeyPressed())){
		if (keypress == KEY_DOWN) {
			menu_s++;
			menu_s %= MOD_END;
		}
		if (keypress == KEY_UP) {
			if(menu_s == START)
				menu_s = MOD_END;
			menu_s--;
		}
		if (keypress == KEY_ENTER) {
			if (menu_s == START)
				return PLAY;
			if (menu_s == OPTIONS)
				printf("options!\n");
			if (menu_s == EXIT)
				return QUIT;
		}
	}
		
	if (menu_s == START)
		marky = playy;
	if (menu_s == OPTIONS)
		marky = optiy;
	if (menu_s == EXIT)
		marky = exity;
	
	return MENU;
}

void render_menu(){
	BeginDrawing();

		ClearBackground(WHITE);
		
		BeginMode3D(camera);

		draw_stack();
		draw_skybox();
		//~ draw_destroying();
		//~ draw_topblock();
		
		//~ DrawGrid(10, 0.5f);
		
		EndMode3D();
		
		DrawText("STAX",    alltx, staxy, 40, RAYWHITE);
		DrawText("_____",   alltx, _txty, 40, RAYWHITE);
		DrawText("Play",    alltx, playy, 40, RAYWHITE);
		DrawText("Options", alltx, optiy, 40, RAYWHITE);
		DrawText("Exit",    alltx, exity, 40, RAYWHITE);
		DrawText(">", -40 + alltx, marky, 40, RAYWHITE);
		
		DrawFPS(10, 10);

	EndDrawing();
}

int main(void)
{
	
	// run once init
	window_init();
	round_init();
	
	// main loop
	while (!WindowShouldClose() && state != QUIT) // desktop [X]
	{
		
		UpdateCamera(&camera);
		
		// update
		//----------------------------------------------------------------------------------
		
		if (state == MENU)
			state = update_menu();
		if (state == PLAY && update_play()){
			state = OVER;
		} else if (state == OVER && update_over()){
			reset_s = CLEAR;
			state = RESET;
		} else if (state == RESET && update_reset()){
			state = MENU;
		}

		// render
		//----------------------------------------------------------------------------------
			
		if (state == MENU)
			render_menu();
		if (state == PLAY)
			render_play();
		if (state == OVER)
			render_over();
		if (state == RESET)
			render_reset();
		
	}

	// De-Initialization
	//--------------------------------------------------------------------------------------
	CloseWindow();        // Close window and OpenGL context
	//--------------------------------------------------------------------------------------

	return 0;
}
