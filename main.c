
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

int main(void)
{
	// Initialization
	//--------------------------------------------------------------------------------------
	const int screenWidth = 800;
	const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera mode");

	SetRandomSeed(time(0));

	// Define the camera to look into our 3d world
	Camera3D camera = { 0 };
	camera.position = (Vector3){ 6.0f, 6.0f, 6.0f };  // Camera position
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
	camera.up = (Vector3){ 0.0f,20.0f, 0.0f };          // Camera up vector (rotation towards target)
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
		stack[i].color.r -= stack[i].color.r / 5 * (i + 1);
		stack[i].color.g -= stack[i].color.g / 5 * (i + 1);
		stack[i].color.b -= stack[i].color.b / 5 * (i + 1);
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
		while(keypress = GetKeyPressed()){
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
			camera.position.y += camera.position.y/camera_blend_height * 0.01f * GetFrameTime() / 0.01667f;
			camera.target.y   += camera.position.y/camera_blend_height * 0.01f * GetFrameTime() / 0.01667f;
		}
		
		// drop the block, add to stack
		if (stop) {
			points++;
			sprintf(point_txt, "Points: %d", points);
			
			for(int i = 9; i > 0; i--){
				stack[i] = stack[i - 1];
			}
			stack[0] = topblock;
			
			camera_blend_height += 0.5f;
			
			topblock.position.x = 0.0f;
			topblock.position.y += 0.5f;
			topblock.position.z = 0.0f;
			
			origin = (origin + 1) % 2;
			if (origin == LEFT)
				topblock.position.x -= 5.0f;
			else
				topblock.position.z -= 5.0f;
			
			topblock.color = block_colors[GetRandomValue(0, colorsize)];
			
			if(points < 125) // they call this the hanging chad
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
				
				// topblock
				DrawCube     (topblock.position, topblock.width, topblock.height, topblock.length, topblock.color);
				DrawCubeWires(topblock.position, topblock.width, topblock.height, topblock.length, LIGHTGRAY     );

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
