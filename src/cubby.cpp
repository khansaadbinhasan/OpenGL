/* Using standard C++ output libraries */
#include <cstdlib>
#include <iostream>

#include <GL/glew.h>
#include <SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SDL_image.h"

#include "../includes/shader_utils.h"
// #include "res_texture.c"


using namespace std;


GLuint program;
GLuint vbo_triangle, vbo_triangle_colors;
GLuint vbo_cube_vertices, vbo_cube_texcoords, vbo_cube_colors;
GLuint ibo_cube_elements;
GLint attribute_coord3d, attribute_v_color, attribute_texcoord;
GLint uniform_m_transform;
GLint uniform_mvp;
GLuint texture_id, program_id;
GLint uniform_mytexture;
float x = 1.5;
float y = -1.5;
float z = -1.5;
float velx = 1.5;
float vely = 1.5;
float velz = 1.5;

int screen_width=800, screen_height=600;


bool init_resources(void) ;
void render(SDL_Window* window);
void free_resources() ;
void mainLoop(SDL_Window* window);
void onResize(int width, int height);

int main(int argc, char* argv[]) 
{

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow("The Cube",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		screen_width, screen_height,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

	if (window == NULL) 
	{
		cerr << "Error: can't create window: " << SDL_GetError() << endl;
		return EXIT_FAILURE;
	}
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	// SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);

	if (SDL_GL_CreateContext(window) == NULL) 
	{
		cerr << "Error: SDL_GL_CreateContext: " << SDL_GetError() << endl;
		return EXIT_FAILURE;
	}


	GLenum glew_status = glewInit();

	if (glew_status != GLEW_OK) 
	{
		cerr << "Error: glewInit: " << glewGetErrorString(glew_status) << endl;
		return EXIT_FAILURE;
	}

	if (!init_resources())
		return EXIT_FAILURE;


    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	mainLoop(window);

	free_resources();

	return EXIT_SUCCESS;
}


bool init_resources(void) 
{
	GLfloat cube_vertices[] = {
	    // front
	    -0.5, -0.5,  0.5,
	     0.5, -0.5,  0.5,
	     0.5,  0.5,  0.5,
	    -0.5,  0.5,  0.5,
	    // top
	    -0.5,  0.5,  0.5,
	     0.5,  0.5,  0.5,
	     0.5,  0.5, -0.5,
	    -0.5,  0.5, -0.5,
	    // back
	     0.5, -0.5, -0.5,
	    -0.5, -0.5, -0.5,
	    -0.5,  0.5, -0.5,
	     0.5,  0.5, -0.5,
	    // bottom
	    -0.5, -0.5, -0.5,
	     0.5, -0.5, -0.5,
	     0.5, -0.5,  0.5,
	    -0.5, -0.5,  0.5,
	    // left
	    -0.5, -0.5, -0.5,
	    -0.5, -0.5,  0.5,
	    -0.5,  0.5,  0.5,
	    -0.5,  0.5, -0.5,
	    // right
	     0.5, -0.5,  0.5,
	     0.5, -0.5, -0.5,
	     0.5,  0.5, -0.5,
	     0.5,  0.5,  0.5,
	  };


	glGenBuffers(1, &vbo_cube_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);


	GLfloat cube_texcoords[2*4*6] = {
		// front
		0.0, 0.0,
		1.0, 0.0,
		1.0, 1.0,
		0.0, 1.0,
	};

	for (int i = 1; i < 6; i++)
		memcpy(&cube_texcoords[i*4*2], &cube_texcoords[0], 2*4*sizeof(GLfloat));


	glGenBuffers(1, &vbo_cube_texcoords);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_texcoords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_texcoords), cube_texcoords, GL_STATIC_DRAW);


	GLushort cube_elements[] = {
		// front
		0,  1,  2,
		2,  3,  0,
		// top
		4,  5,  6,
		6,  7,  4,
		// back
		8,  9, 10,
		10, 11,  8,
		// bottom
		12, 13, 14,
		14, 15, 12,
		// left
		16, 17, 18,
		18, 19, 16,
		// right
		20, 21, 22,
		22, 23, 20,
	};

	glGenBuffers(1, &ibo_cube_elements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);


	SDL_Surface* res_texture = IMG_Load("../resources/res_texture.png");

	if (res_texture == NULL) 
	{
		cerr << "IMG_Load: " << SDL_GetError() << endl;
		return false;
	}
	
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	
	glTexImage2D(GL_TEXTURE_2D, // target
		0, // level, 0 = base, no minimap,
		GL_RGBA, // internalformat
		res_texture->w, // width
		res_texture->h, // height
		0, // border, always 0 in OpenGL ES
		GL_RGBA, // format
		GL_UNSIGNED_BYTE, // type
		res_texture->pixels);

	SDL_FreeSurface(res_texture);




	GLint link_ok = GL_FALSE;
	GLuint vs, fs;
	
	if ( (vs = create_shader("vshady_file.cpp", GL_VERTEX_SHADER)) == 0 ) 
		return false;
	
	if ( (fs = create_shader("fshady_file.cpp", GL_FRAGMENT_SHADER)) == 0 )  
		return false;





	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	
	if (!link_ok) 
	{
		cerr << "Error in glLinkProgram" << endl;
		return false;
	}


	const char* attribute_name;
	attribute_name = "coord3d";
	attribute_coord3d = glGetAttribLocation(program, attribute_name);

	if (attribute_coord3d == -1) 
	{
		cerr << "Could not bind attribute " << attribute_name << endl;
		return false;
	}

	attribute_name = "texcoord";
	attribute_texcoord = glGetAttribLocation(program, attribute_name);
	
	if (attribute_texcoord == -1) 
	{
		cerr << "Could not bind attribute " << attribute_name << endl;
		return false;
	}

	// cout << "jer";

	const char* uniform_name = "mytexture";
	uniform_mytexture = glGetUniformLocation(program, uniform_name);
	
	if (uniform_mytexture == -1) 
	{
		cerr << "Could not bind uniform " << uniform_name << endl;
		return false;
	}




	return true;
}

void render(SDL_Window* window) 
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(uniform_mytexture, 0);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	glEnableVertexAttribArray(attribute_coord3d);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);

	glVertexAttribPointer(
		attribute_coord3d,   // attribute
		3,                   // number of elements per vertex, here (x,y,z)
		GL_FLOAT,            // the type of each element
		GL_FALSE,            // take our values as-is
		0,  // next coord3d appears every 5 floats
		0                    // offset of first element
	);

	glEnableVertexAttribArray(attribute_texcoord);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_texcoords);

	glVertexAttribPointer(
		attribute_texcoord, // attribute
		2,                  // number of elements per vertex, here (x,y)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
	);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
	int size;  
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

	glDisableVertexAttribArray(attribute_coord3d);
	// glDisableVertexAttribArray(attribute_v_color);
	SDL_GL_SwapWindow(window);
}

void free_resources() 
{
	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo_cube_vertices);
	glDeleteBuffers(1, &vbo_cube_texcoords);
	glDeleteBuffers(1, &ibo_cube_elements);
	glDeleteTextures(1, &texture_id);
}

void logic() 
{
	float angle = SDL_GetTicks() / 1000.0 * glm::radians(15.0);  // base 15° per second
	
	glm::mat4 anim =
		glm::rotate(glm::mat4(1.0f), angle*3.0f, glm::vec3(1, 0, 0)) *  // X axis
		glm::rotate(glm::mat4(1.0f), angle*2.0f, glm::vec3(0, 1, 0)) *  // Y axis
		glm::rotate(glm::mat4(1.0f), angle*4.0f, glm::vec3(0, 0, 1));   // Z axis
	
	glm::mat4 Pos = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -4.0));
	glm::mat4 view = glm::lookAt(glm::vec3(0.0, 2.0, 0.0), glm::vec3(0.0, 0.0, -4.0), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 projection = glm::perspective(45.0f, 1.0f*screen_width/screen_height, 0.1f, 10.0f);
	
	glm::mat4 mvp = projection * view * model * Pos * anim;
	glUseProgram(program);
	glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));




	float xprev = x;
	float yprev = y;
	float zprev = z;

	// cout << (x == 1.7) << endl;

	int flag1 = 0;
	int flag2 = 0;

	if( x <= 2.0 && x > -2.0 && velx > 0 )
	{
		x = x + 0.01;
		flag1 = 1;
		cout << "1" << endl;
	}

	else if ( x <= 2.0 && x > -2.0 && velx < 0 )
	{
		x = x - 0.01;
		flag2 = 1;
		cout << "2" << endl;
	}

	if( x <= -2.0 && flag2 != 1 )
	{
		x = x + 0.01;
		cout << "3" << endl;
	}


	else if( x >= 2.0 && flag1 != 1 )
	{
		x = x - 0.01;
		cout << "4" << endl;
	}


	flag1 = 0;
	flag2 = 0;

	if( y <= 1.3 && y > -1.5 && vely > 0 )
	{
		y = y + 0.01;
		flag1 = 1;
		cout << "1" << endl;
	}

	else if ( y <= 1.3 && y > -1.5 && vely < 0 )
	{
		y = y - 0.01;
		flag2 = 1;
		cout << "2" << endl;
	}

	if( y <= -1.5 && flag2 != 1 )
	{
		y = y + 0.01;
		cout << "3" << endl;
	}


	else if( y >= 1.3 && flag1 != 1 )
	{
		y = y - 0.01;
		cout << "4" << endl;
	}

	
	flag1 = 0;
	flag2 = 0;

	if( z <= 0.5 && z > -1.5 && velz > 0 )
	{
		z = z + 0.01;
		flag1 = 1;
		cout << "1" << endl;
	}

	else if ( z <= 0.5 && z > -1.5 && velz < 0 )
	{
		z = z - 0.01;
		flag2 = 1;
		cout << "2" << endl;
	}

	if( z <= -1.5 && flag2 != 1 )
	{
		z = z + 0.01;
		cout << "3" << endl;
	}


	else if( z >= 0.5 && flag1 != 1 )
	{
		z = z - 0.01;
		cout << "4" << endl;
	}

	
	cout << x << "\t" << velx << "\t" << flag1 << "\t" << flag2 << endl;
	cout << y << "\t" << vely << "\t" << flag1 << "\t" << flag2 << endl;
	cout << z << "\t" << velz << "\t" << flag1 << "\t" << flag2 << endl;


	if( x != xprev )
		velx = x - xprev;

	if( y != yprev )
		vely = y - yprev;

	if( z != zprev )
		velz = z - zprev;
}

void mainLoop(SDL_Window* window) 
{
	while (true) 
	{
		SDL_Event ev;

		while (SDL_PollEvent(&ev)) 
		{
			if (ev.type == SDL_QUIT)
				return;

			if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				onResize(ev.window.data1, ev.window.data2);
		}
		render(window);
		logic();

		// cout << "Start shall we?(y/n): ";
    	// cin >> start;

    	// if(start == 'n')
    		// continue;
	}
}

void onResize(int width, int height) 
{
  screen_width = width;
  screen_height = height;
  glViewport(0, 0, screen_width, screen_height);
}