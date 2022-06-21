
#include "raylib.h"
#include <stdio.h>
#include <time.h>
#include <math.h>

typedef struct {
	Vector3 position;
	float   width;
	float   height;
	float   length;
	Color   color;
} cube_t;

Color block_colors[] = {
	GRAY      ,
	DARKGRAY  ,
	RED       ,
	MAROON    ,
	LIME      ,
	DARKGREEN ,
	BLUE      ,
	DARKBLUE  ,
	VIOLET    ,
	DARKPURPLE,
	BROWN     ,
	DARKBROWN ,
};

typedef enum { LEFT, RIGHT } direction_t;

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

cube_t * destroying = NULL;
int des_count = 0;

void draw_destroying(){
	
	for(int i = 0; i < des_count; i++){
		DrawCube     (destroying[i].position, destroying[i].width, destroying[i].height, destroying[i].length, destroying[i].color);
		DrawCubeWires(destroying[i].position, destroying[i].width, destroying[i].height, destroying[i].length, LIGHTGRAY     );
	}
	
}

void push_to_destroying(cube_t d){
	des_count++;
	destroying = MemRealloc(destroying, sizeof(d) * des_count);
	//~ d.color = RAYWHITE;
	destroying[des_count - 1] = d;
}

/*
cube_t chop_cubes(cube_t top, cube_t last) {
	
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
	
	Rectangle new     = GetCollisionRec(rtop, rlast);
	
	//~ printf("rtop -- x: %f\ty: %f\tw: %f\th: %f\n", rtop.x, rtop.y, rtop.width, rtop.height);
	//~ printf(" new -- x: %f\ty: %f\tw: %f\th: %f\n", new.x, new.y, new.width, new.height);
	//~ Rectangle des;
	//~ if(diffx > 0.05f){
		//~ des.x     = new.x + new.width;
		//~ des.y     = new.y;
		//~ des.width = rtop.width - new.width;
		//~ des.height = new.height;
	//~ } else {
		//~ des.x = new.x;
		//~ des.y = new.y + new.height;
		//~ des.width = new.width;
		//~ des.height = rtop.height - new.height;
	//~ }
	
	//~ cube_t descube = {
		//~ .position = { des.x, top.position.y, des.y },
		//~ .width = des.width,
		//~ .height = 0.5f,
		//~ .length = des.height,
		//~ .color = top.color,
	//~ };
	
	//~ push_to_destroying(descube);
	
	return newcube;
}
*/

cube_t chop_cubes(cube_t curr, cube_t base) {
	
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
		.color  = curr.color,
	};
	
	//~ curr.position.x = (curr.position.x + newc.position.x)/2;
	//~ curr.position.z = (curr.position.z + newc.position.z)/2;
				//~ DrawCubeWires(curr.position, curr.width, curr.height, curr.length, MAGENTA     );
	
	Vector2 n_ltmid = { newr.x,                  newr.y + newr.height / 2};
	Vector2 n_rtmid = { newr.x + newr.width,     newr.y + newr.height / 2};
	Vector2 n_upmid = { newr.x + newr.width / 2, newr.y };
	Vector2 n_dnmid = { newr.x + newr.width / 2, newr.y + newr.height};
	
	//~ Vector2 n_ltmid = { newr.x, newr.y };
	//~ Vector2 n_rtmid = { newr.x + newr.width, newr.y };
	//~ Vector2 n_upmid = { newr.x + newr.width, newr.y };
	//~ Vector2 n_dnmid = { newr.x, newr.y };
	
	DrawCubeWires((Vector3){.x = n_ltmid.x, .y=curr.position.y, .z=n_ltmid.y}, 0.1f, 0.1f, 0.1f, GREEN );
	DrawCubeWires((Vector3){.x = n_rtmid.x, .y=curr.position.y, .z=n_rtmid.y}, 0.1f, 0.1f, 0.1f, GREEN );
	DrawCubeWires((Vector3){.x = n_upmid.x, .y=curr.position.y, .z=n_upmid.y}, 0.1f, 0.1f, 0.1f, GREEN );
	DrawCubeWires((Vector3){.x = n_dnmid.x, .y=curr.position.y, .z=n_dnmid.y}, 0.1f, 0.1f, 0.1f, GREEN );
	
	Vector2 c_ltmid = { curr_rect.x,                  curr_rect.y + curr_rect.height / 2};
	Vector2 c_rtmid = { curr_rect.x + curr_rect.width,     curr_rect.y + curr_rect.height / 2};
	Vector2 c_upmid = { curr_rect.x + curr_rect.width / 2, curr_rect.y };
	Vector2 c_dnmid = { curr_rect.x + curr_rect.width / 2, curr_rect.y + curr_rect.height};
	
	DrawCubeWires((Vector3){.x = c_ltmid.x, .y=curr.position.y, .z=c_ltmid.y}, 0.1f, 0.1f, 0.1f, RED );
	DrawCubeWires((Vector3){.x = c_rtmid.x, .y=curr.position.y, .z=c_rtmid.y}, 0.1f, 0.1f, 0.1f, RED );
	DrawCubeWires((Vector3){.x = c_upmid.x, .y=curr.position.y, .z=c_upmid.y}, 0.1f, 0.1f, 0.1f, RED );
	DrawCubeWires((Vector3){.x = c_dnmid.x, .y=curr.position.y, .z=c_dnmid.y}, 0.1f, 0.1f, 0.1f, RED );
	
	Vector2 b_ltmid = { base_rect.x,                       base_rect.y + base_rect.height / 2};
	Vector2 b_rtmid = { base_rect.x + base_rect.width,     base_rect.y + base_rect.height / 2};
	Vector2 b_upmid = { base_rect.x + base_rect.width / 2, base_rect.y };
	Vector2 b_dnmid = { base_rect.x + base_rect.width / 2, base_rect.y + base_rect.height};
	
	DrawCubeWires((Vector3){.x = b_ltmid.x, .y=curr.position.y, .z=b_ltmid.y}, 0.1f, 0.1f, 0.1f, MAGENTA );
	DrawCubeWires((Vector3){.x = b_rtmid.x, .y=curr.position.y, .z=b_rtmid.y}, 0.1f, 0.1f, 0.1f, MAGENTA );
	DrawCubeWires((Vector3){.x = b_upmid.x, .y=curr.position.y, .z=b_upmid.y}, 0.1f, 0.1f, 0.1f, MAGENTA );
	DrawCubeWires((Vector3){.x = b_dnmid.x, .y=curr.position.y, .z=b_dnmid.y}, 0.1f, 0.1f, 0.1f, MAGENTA );
	
	
	bool ltcol = CheckCollisionPointRec(b_ltmid, curr_rect);
	bool rtcol = CheckCollisionPointRec(b_rtmid, curr_rect);
	bool upcol = CheckCollisionPointRec(b_upmid, curr_rect);
	bool dncol = CheckCollisionPointRec(b_dnmid, curr_rect);
	
	float xpad = 1.0f;
	float ypad = 1.0f;
	if(ltcol)
		xpad = -1.0f;
	if(upcol)
		ypad = -1.0f;
	
	Rectangle desr;
	if(upcol){
		desr.x = b_upmid.x;
		desr.y = (b_upmid.y + c_upmid.y) / 2;
		desr.width = curr.width;
		desr.height = c_upmid.y - b_upmid.y;
	}
	
	printf("lt: %d rt: %d up: %d dn: %d\n", ltcol, rtcol, upcol, dncol);
	printf("xp: %f yp: %f\n", xpad, ypad);
	
	cube_t desc = {
		.position = {
			.x = desr.x,
			.y = curr.position.y,
			.z = desr.y,
		},
		.width  = desr.width,
		.height = 0.5f,
		.length = desr.height,
		.color  = RAYWHITE,
	};
	desc.color.a = 64;
	
	// TODO -- deleteme
	//~ if (destroying == NULL)
		push_to_destroying(desc);
	
	return newc;
}

int main(void)
{
	// Initialization
	//--------------------------------------------------------------------------------------
	const int screenWidth = 640;
	const int screenHeight = 480;

	InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera mode");

	SetRandomSeed(time(0));

	// Define the camera to look into our 3d world
	Camera3D camera = { 0 };
	camera.position = (Vector3){ 6.0f, 6.0f, 6.0f };  // Camera position
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
	camera.up = (Vector3){ 0.0f,1.0f, 0.0f };          // Camera up vector (rotation towards target)
	camera.fovy = 20.0f;                                // Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type
	
	float camera_blend_height = camera.position.y;
	
	float radius = sqrt(
		pow(camera.position.x - camera.target.x, 2) +
		pow(camera.position.z - camera.target.z, 2)
	);
	Vector2 angle = { .x  = 45.0f, .y = 45.0f };
	
	SetCameraMode(camera, CAMERA_CUSTOM); // Set a free camera mode
	
	const unsigned int colorsize = sizeof(block_colors)/sizeof(block_colors[0]) - 1;
	
	cube_t topblock = {
		.position = { 0.0f, 0.25f, 0.0f },
		.width = 2.0f,
		.height = 0.5f,
		.length = 2.0f,
		.color = block_colors[GetRandomValue(0, colorsize)],
	};
	
	cube_t stack[10];
	for(int i = 0; i < 10; i++){
		stack[i] = (cube_t){ // starting cube
			.position = (Vector3){0.0f, (float)(i) * -0.5f + -0.25f, 0.0f},
			.width =     2.0f,
			.height =    0.5f,
			.length =    2.0f,
			.color =    topblock.color,
		};
		if(i > 4)
			stack[i].color = BLACK;
		else {
			stack[i].color.r -= stack[i].color.r / 5 * (i + 1);
			stack[i].color.g -= stack[i].color.g / 5 * (i + 1);
			stack[i].color.b -= stack[i].color.b / 5 * (i + 1);
		}
	}
	
	
	direction_t origin = GetRandomValue(LEFT, RIGHT);
	float direction = 1.0; // toward bottom
	
	if (origin == LEFT)
		topblock.position.x -= 5.0f;
	else
		topblock.position.z -= 5.0f;
	
	int  points = 0;
	char point_txt[13];
	Color bg = {.r = 60, .g = 120, .b = 180};
	
	sprintf(point_txt, "Points: %d", points);

	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
	//--------------------------------------------------------------------------------------

	// Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		
		UpdateCamera(&camera);
		
		// Update
		//----------------------------------------------------------------------------------
		
		// read stop input before here
		int keypress = -1;
		bool stop = false;
		while((keypress = GetKeyPressed())){
			if (keypress == KEY_ENTER) {
				stop = true;
			}
		}
		
		// L camera
		if (IsKeyDown(KEY_A)){
			angle.x += 1.0f;
			if (angle.x >= 360.0f)
				angle.x -= 360.0f;
			
			camera.position.x = camera.target.x + radius * cos(angle.x * PI / 180);
			camera.position.z = camera.target.z + radius * sin(angle.x * PI / 180);
		}
		
		// R camera
		if (IsKeyDown(KEY_D)){
			angle.x -= 1.0f;
			if (angle.x <= 0.0f)
				angle.x = 360.f - angle.x;
			
			camera.position.x = camera.target.x + radius * cos(angle.x * PI / 180);
			camera.position.z = camera.target.z + radius * sin(angle.x * PI / 180);
		}
		
		if(camera.position.y < camera_blend_height){
			camera.position.y += 0.01f * GetFrameTime() / 0.01667f;
			camera.target.y   += 0.01f * GetFrameTime() / 0.01667f;
		}
		
		// drop the block, add to stack
		if (stop) {
			
			if (check_cube_collision(topblock, stack[0])){
				
				topblock = chop_cubes(topblock, stack[0]);
				
			} else {
				// Game Over
			}
			
			points++;
			sprintf(point_txt, "Points: %d", points);
			
			for(int i = 9; i > 0; i--){
				stack[i] = stack[i - 1];
			}
			stack[0] = topblock;
			
			camera_blend_height += 0.5f;
			
			//~ topblock.position.x = 0.0f;
			topblock.position.y += 0.5f;
			//~ topblock.position.z = 0.0f;
			
			origin = (origin + 1) % 2;
			if (origin == LEFT)
				topblock.position.x -= 5.0f;
			else
				topblock.position.z -= 5.0f;
			
			topblock.color = block_colors[GetRandomValue(0, colorsize)];
			
			if(points < 125){
				if(points % 3 == 0){
					bg.r--;
					bg.g--;
					bg.b--;
				} else if (points % 2 == 0){
					bg.g--;
					bg.b--;
				} else {
					bg.b--;
				}
			}
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
		if ( *axis < -5.0f  || *axis > 5.0f ){
			*axis = direction * 5.0f;
			direction *= -1.0f;
		}

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();

			ClearBackground(bg);

			BeginMode3D(camera);
				
				// stack
				for(int i = 0; i < 10; i++){
					DrawCube     (stack[i].position, stack[i].width, stack[i].height, stack[i].length, stack[i].color);
					DrawCubeWires(stack[i].position, stack[i].width, stack[i].height, stack[i].length, LIGHTGRAY     );
				}
				
				// destroying
				draw_destroying();
				
				// topblock
				DrawCube     (topblock.position, topblock.width, topblock.height, topblock.length, topblock.color);
				DrawCubeWires(topblock.position, topblock.width, topblock.height, topblock.length, LIGHTGRAY     );
				DrawCubeWires(topblock.position, 0.5f, topblock.height, 0.5f, MAGENTA     );

				DrawGrid(10, .5f);

			EndMode3D();
			
			DrawText(point_txt, 10, 40, 20, LIGHTGRAY);

			DrawFPS(10, 10);

		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// De-Initialization
	//--------------------------------------------------------------------------------------
	CloseWindow();        // Close window and OpenGL context
	//--------------------------------------------------------------------------------------

	return 0;
}
