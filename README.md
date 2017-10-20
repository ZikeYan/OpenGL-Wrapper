# OpenGL Wrapper

Initially, this project was a wrapper for OpenGL 3.x+ pipeline, merely for 
easier use. Later, when I have to deal with various tasks, functions are 
added, and it becomes more comprehensive. There might be a lot of bugs, since
 it is designed for individual projects. 
 
 - The code that wraps OpenGL raw functions is located in `src`.
 - The code that provides higher level functions is located in `example`.
 - Related OpenGL shader code can be found in `shader`.
 - Models (.obj files) and textures (.png files) can be found in `model`. 
 
 ## Build and Run
 ### Dependencies
 - [CMake](https://cmake.org/)
 - [GLFW3](http://www.glfw.org/docs/latest/)
 - [GLEW](http://glew.sourceforge.net/)
 - [GLM](https://glm.g-truc.net/)
 - [OpenCV 2](https://opencv.org/)

 ### Compile
 ```bash
 mkdir build
 cd build
 cmake ..
 make -j4
 ```
### Run
Go to `./bin` and run corresponding binaries.
Typically,
- `encode_image_pixel2uv_projection` is a demo using the Beethoven model. After 
launching the program, you can 
navigate through keyboard: `W/A/S/D/SPACE/SHIFT` will move you around, 
`UP/DOWN/LEFT/RIGHT` will rotate your head. When you find a suitable 
viewpoint, press `ENTER` to record the view. The program will generate a 
 .txt file encoding the pixel-uv correspondence at the viewpoint, 
along with a screenshot. The generated file can be 
decoded by MATLAB code, `example/script/TestBeethovenProjection.m`.
- `encode_sequence_pixel2uv_projection` is a demo using the Face model. It 
integrates **no** interaction: it loads preset camera poses (generated from the 
calib0000n.txt) and generates per-frame projection .txt file and rendered image.

