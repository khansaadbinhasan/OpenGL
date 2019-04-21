#include "shader_utils.h"

char* file_read(const char* filename)
{
	SDL_RWops *rw = SDL_RWFromFile(filename, "rb");

	if( rw == NULL )
	{
		return NULL;
	}

	Sint64 res_size = SDL_RWsize(rw);
	char* res = (char*)malloc(res_size + 1);

	Sint64 nb_read_total = 0, nb_read = 1;

	char* buf = res;

	while( nb_read_total < res_size && nb_read != 0 )
	{
		nb_read = SDL_RWread(rw, buf, 1, (res_size - nb_read_total));
		nb_read_total = nb_read_total + nb_read;
		buf = buf + nb_read;
	}

	SDL_RWclose(rw);

	if( nb_read_total != res_size )
	{
		free(res);
		return NULL;
	}

	res[nb_read_total] = '\0';
	return res;
}


void print_log(GLuint object)
{
	GLint log_length = 0;

	if( glIsShader(object) )
	{
		glGetShaderiv( object, GL_INFO_LOG_LENGTH, &log_length );
	}

	else if( glIsProgram(object) )
	{
		glGetProgramiv( object, GL_INFO_LOG_LENGTH, &log_length );
	}

	else
	{
		cerr << "printlog: Not a shader or a program" << endl;
	}

	char* log = (char*)malloc(log_length);

	if(glIsShader(object))
		glGetShaderInfoLog(object, log_length, NULL, log);

	else if( glIsProgram(object) )
		glGetProgramInfoLog(object, log_length, NULL, log);

	cerr << log;

	free(log);
}


// void print_log(GLuint object)
// {
// 	GLint log_length = 0;

// 	if( glIsShader(object) )
// 	{
// 		glGetShaderiv( object, GL_INFO_LOG_LENGTH, &log_length );
// 		char* log = (char*)malloc(log_length);
// 		glGetShaderInfoLog(object, log_length, NULL, log);
// 	}

// 	else if( glIsProgram(object) )
// 	{
// 		glGetProgramiv( object, GL_INFO_LOG_LENGTH, &log_length );
// 		char* log = (char*)malloc(log_length);
// 		glGetProgramInfoLog(object, log_length, NULL, log);
// 	}

// 	else
// 	{
// 		cerr << "printlog: Not a shader or a program" << endl;
// 	}

// 	cerr << log;

// 	free(log);
// }


GLuint create_shader(const char* filename, GLenum type)
{
	const GLchar* source = file_read(filename);

	if( source == NULL )
	{
		cerr << "Error opening " << filename << ": " << SDL_GetError() << endl;
		return 0;
	}

	GLuint res = glCreateShader(type);

	// GLSL version
	const char* version;
	int profile;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile);

	if (profile == SDL_GL_CONTEXT_PROFILE_ES)
		version = "#version 100\n";  // OpenGL ES 2.0
	else
		version = "#version 120\n";  // OpenGL 2.1

	// GLES2 precision specifiers
	const char* precision;
	precision =
		"#ifdef GL_ES                        \n"
		"#  ifdef GL_FRAGMENT_PRECISION_HIGH \n"
		"     precision highp float;         \n"
		"#  else                             \n"
		"     precision mediump float;       \n"
		"#  endif                            \n"
		"#else                               \n"
		// Ignore unsupported precision specifiers
		"#  define lowp                      \n"
		"#  define mediump                   \n"
		"#  define highp                     \n"
		"#endif                              \n";

	const GLchar* sources[] = {
		version,
		precision,
		source
	};
	
	glShaderSource(res, 3, sources, NULL);

	free((void*)source);

	glCompileShader(res);
	GLint compile_ok = GL_FALSE;
	glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);

	if( compile_ok == GL_FALSE )
	{
		cerr << filename << ":";
		print_log(res);
		glDeleteShader(res);
		return 0;
	}

	return res;
}
