# OpenGL

### Description

Folder `includes` contain the header file `shader_utils.h`, `resources` contain the texture `res_texture.png` applied to the cube and `src` contains the following files:

* `cubby.cpp` contains the main functions and code that initialize openGL and render the textured cube. It also provides logic to move the cube.
* `MakeFile` to compile the `cubby.cpp` file
* `fshady_file.cpp` and `vshady_file.cpp` contain the frame and vertex variables respectively.

### Execution

To run this code, first [setup openGL](https://en.wikibooks.org/wiki/OpenGL_Programming/Installation/Linux), and `cd` to the directory containing the above code. Then `cd` to `src` directory and run:

```
make -f MakeFile
```

you should find an executable by the name of `cubby`, execute it using:

```
./cubby
```

A window should pop-up with a cube as shown:

![cubby](resouces/cubby.png)

The full animation can be found [here](https://www.youtube.com/watch?v=0-QsvuVvP-4&feature=youtu.be).
