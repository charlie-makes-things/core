//cg2d. a 2d rendering wotsit that uses SDL3 GPU to draw stuff to the screen.
//cobbled together by charlie in 2026 using the SDL3 pull sprite batch example
//as a guide.

//the example file cg2d_example.c goes through most of the functionality of 
//cg2d, so it's a good reference for usage.

/*
about:

cg2d is based on max2d, the blitzmax 2d library, to help port my old games to
modern systems. Max2d is old, and used the ancient openGL 1.x or directX 7 to
render things to the screen. these old apis made it pretty easy to draw stuff,
change blend states etc. and get up and running quicky: something 
modern apis arent quite as well equipped to do.

cg2d tries to maintain the ease of use of max2d in terms of getting stuff on 
screen and staying out of the way while using new GPU apis as the backend.
To accomplish this, a few extra steps have had to be put in place which add
slightly to the number of lines of code needed, though the extra flexibility
is, i think, worth it.

While this was written essentially to port old games, it could also prove useful
for other stuff further down the line such as gui stuff or offscreen 2d rendering.

notes in no particular order:
=============================

coordinates start in the top left corner of the window to match max2d

attempts to be as functionally close to max2d as possible with a few internal
differences:
			- image and primitive transformations now takes place in the vertex shader, 
			  with the one exception of font rendering, where i've used a matrix to
			  rotate/scale/translate the text so i could have multiline strings

			- everything is now rendered in layers. this is the biggest change
			  from max2d where you could just draw anything anywhere. The layers 
			  batch together draw calls in order to minimise state changes 
			  (blendmodes, textures etc). This adds minor complexity, though you 
			  have far greater control of how and when things are drawn, and what
			  they are drawn to. see the cg2d_example for usage.
			
			- the setorigin command had been replaced with functions to apply matrix
			  transforms to the layers. this is much more flexible now, as it adds
			  rotation and scaling, not just offsetting from 0,0
			  
			- the LoadImage() function in max2d is no longer present. cg2d does
			  not load any resources apart from the debug font if it is being used.
			  instead, you load/create any textures externally, either directly from
			  an image file and use cg2d_create_image() to create and image in
			  cg2d the same size as the texture, or you can use a texture atlas
			  to load the texture and the image data and use cg2d_load_atlas_image()
			  to add it's images to cg2d.
			
			- the collision features of max2d are not implemented.

			- there is no built in font like max2d, you must load a font from file with
			  cg2d_load_image_font();

			- the animation (animImage) features from max2d are not implemented. 
			  these were pretty basic and it would likely be better to roll your own
			  animation system, or use one from elsewhere.

			- cg2d_draw_sub_image_rect - based on drawSubImageRect, which was not very
			  easy to understand in the max2d code. cg2d accepts uv offsets to define the
			  sub image. there is an example in cg2d_example.c which clips a portion of an
			  image and scrolls it around.

			- there is no cls() function. instead use the cg2d_set_layer_clear() function. 
			  you can set the clear parameter here to true if you want all the pixels
			  overwritten with the cg2d clear colour, or false if you want to preserve
			  whats been drawn behind it. 

			- cg2d_init requires an active sdl window and gpu device. see cg2d_example.c
			  for usage.

			- there is no flip function. cg2d requires you to draw your layers individually.

			- while i've tried to keep this as self contained as possible, you will need
			  to link to SDL3, include the stb_ds.h header and the texAtlas.c file. see
			  the example file for system headers

			- initially i wanted to enable custom shader support, but i think i'll skip
			  the extra work for now. in the mean time rendering a scene to a texture will
			  allow me to do some post processing where needed, which is likely enough
			  for porting old stuff anyway. 

			- points, lines, rects, ovals etc. are all actually just images internally, with
			  a value passed to the shaders to say how to render them. This makes it possible
			  to have lots of different images/primitives drawn in each layer.

			- some of the basic primitives have extra options, such as drawing rect outlines
			  textured lines/points, some colour gradiation etc. that was not part of
			  max2d but shaders make them an easy and useful add.

*/

//defines
#define CG2D_MAX_VERTICES 10000
#define CG2D_DEG2RAD 3.14159 / 180.0
#define CG2D_RAD2DEG 180.0 / 3.14159


//structs

//Blend mode  |	Effect
//==========================================================
//MASKBLEND   |	Pixels with an alpha of <0.5 are not drawn
//SOLIDBLEND  |	Pixels overwrite existing backbuffer pixels
//ALPHABLEND  |	Pixels are alpha blended with existing backbuffer pixels
//LIGHTBLEND  |	Pixel colors are added to backbuffer pixel colors, giving a 'lighting' effect
//SHADEBLEND  |	Pixel colors are multiplied with backbuffer pixel colors, giving a 'shading' effect

typedef enum cg2d_blend{
	CG2D_MASKBLEND=1,
	CG2D_SOLIDBLEND=2,
	CG2D_ALPHABLEND=3,
	CG2D_LIGHTBLEND=4,
	CG2D_SHADEBLEND=5,
}cg2d_blend;

//the vertex struct. this is the data sent to the shader arranged to
//match it input struct exactly.
typedef struct cg2d_vertex{
	float x,y,z;
	float rotation;
	float hx,hy,w,h;
	float u,v,uw,vh;
	float mr,mg,mb,ma;
	float r,g,b,a;
	float mode,type;//note mode isn't used in the shader any more
	float scalex,scaley;
	float skewx,skewy,outlinew,yy;

}cg2d_vertex;

typedef struct cg2d_font{
	int id;
	int ptsz;
	TTF_Font *font;
	TTF_Text *text;
	cg_texture tex;
}cg2d_font;

typedef struct _cg2d_mat4{
	float m11, m12, m13, m14;
	float m21, m22, m23, m24;
	float m31, m32, m33, m34;
	float m41, m42, m43, m44;
} _cg2d_mat4;

//buffers. these are what the layers refer to.
typedef struct cg2d_buffer{
	uint id;
	SDL_GPUGraphicsPipeline* RenderPipeline;//one pipeline per blend mode
	SDL_GPUBuffer *vbuffer;
	SDL_GPUTransferBuffer *tbuffer;
	SDL_GPUSampler *samp;//these are passed to the buffer so don't free internally
    SDL_GPUTexture *tex;
    uint vbufsz;
    uint vbuflen;
    cg2d_blend blend;
    cg2d_vertex *verts;//this is what things get stored in before transfer.
    SDL_GPULoadOp load_op;
    bool used;
    _cg2d_mat4 transform;
}cg2d_buffer;

typedef struct cg2d_vertex_uniform {
    _cg2d_mat4 mat;
    float scrnx,scrny;
    float origx,origy;
} cg2d_vertex_uniform;

typedef struct cg2d_fragment_uniform{
	float scrnW,scrnH;
    float ticks;
    float posX,posY;
}cg2d_fragment_uniform;

typedef struct cg2d_image{
	int index;
	char name[256];
	float x,y,w,h;
	float hx,hy;
	float u,v,uw,vh;
	float ox,oy,ow,oh;
	float mr,mg,mb,ma;
}cg2d_image;

//the main cg2d struct.
typedef struct cg2d_t{

	//initialised and ready to go?
	bool ready;

	//viewport dimensions
	int vpx,vpy,vpw,vph;
	//window scale - use when setting viewport offsets
	int winW,winH;
	int virtW,virtH;
	float virtMXscale,virtMYscale;

	//origin and handle
	float hx,hy;
	bool automidhandle;

	//transform
	float scale_x,scale_y;
	float rotation;
	float skewx,skewy;
	//cls color
	bool cls;
	int clsr,clsb,clsg;


	
	//color
	int colr,colb,colg;
	float alpha;
	int mr,mb,mg;
	float ma;
	//outline
	float outlinew;//used in shader for rect and oval outlines

	//lines and points
	float linew;
	float pointsz;

	//vertex buffers
	cg2d_buffer *buffers;
	uint currentBuffer,bufferCount;
	cg2d_blend *lastBlend;
	
	//frag unform
	cg2d_fragment_uniform fragUniform;

	cg2d_image *imageList;


	//debug font
	// texAtlas fontAtlas;
	// cg2d_image *fontImages[128];
	// float spaceWidth;
	// float fontYoffset;
	// bool useDebugFont;

	//sdl stuff
	SDL_GPUDevice *device;
	SDL_Window *window;
	
	//sdl_ttf font stuff`
	TTF_TextEngine *textEngine;
	bool fontInitialised;
	cg2d_font *fonts;
	int fontcnt;
	int currentFont;

}cg2d_t;


//////////////////////////////////////////////////////////////////////
//function defs - with brief descriptions
//////////////////////////////////////////////////////////////////////

//helper functions for angle in degrees. max2d used degrees and theres some tricky
//trig in my old games, so these should mean i don't need to hange much existing code
float cg2d_sin(float ang);
float cg2d_cos(float ang);
float cg2d_tan(float ang);
float cg2d_atan(float ang);
float cg2d_asin(float ang);
float cg2d_acos(float ang);

/*initialise cg2d
	params:
		c2d - a cg2d_t pointer
		device - an active SDL_GDPDevice
		window - an active SDL_Window
		scrnW - screen width as sent to SDL_CreateWindow
		scrnH - screen height as sent to SDL_CreateWindow
*/
void cg2d_init(cg2d_t *c2d,SDL_Window *window,SDL_GPUDevice *device, int scrnW,int scrnH);

//free cg2d. use at exit, but before you have destroyed the window and gpu device
void cg2d_free(cg2d_t *c2d);

//creates an image for the given texture. the name param here is just in case you need
//to look it up in the imageList
cg2d_image *cg2d_create_image(cg2d_t *c2d,char *name,cg_texture *tex,int width,int height);

//create an image from a texAtlas. the name param should match the name in the
//texAtlas
cg2d_image* cg2d_load_atlas_image(cg2d_t *c2d,texAtlas *atlas,char* imgName);

// //prints image data, name, x,y,w,h uv coords, mask colours etc
// void cg2d_print_image_data(cg2d_image *img);

// //initialise the debug font. not done at cg2d_init as you wont want to use it
// //beyond debugging.
// void cg2d_init_debug_font(cg2d_t *c2d);

//set viewport dimensions
void cg2d_set_viewport(cg2d_t *c2d,int x, int y, int w, int h);

//returns scaled coords for use with SDL_SetGPUViewport
void cg2d_get_viewport(cg2d_t *c2d,int *x, int *y, int *w, int *h);

/*create a new layer
	params:
		c2d - a cg2d_t pointer
		blend - the layer's blend mode
		tex - a texture and sampler
		vertex_shader - the loaded vertex shader
		fragment_shader - the loaded fragment shader
		loadop - true if you want to write the cg2d clear colour to the layer on render,
				 false if you want what is behind the layer to be visible
*/
int cg2d_add_layer(cg2d_t *c2d, cg2d_blend blend,cg_texture *tex,SDL_GPUShader* vertex_shader,SDL_GPUShader* fragment_shader,SDL_GPULoadOp loadop);

//set the current active layer. all cg2d_draw_xxx calss will be written to the active layer
void cg2d_set_layer(cg2d_t *c2d, int layerID);

//get the current active layer
int cg2d_get_current_layer(cg2d_t *c2d);

//change how the layer clears piexels before rendering. true to set the layer to the cg2d clear 
//colour, false to allow previous layers to show through.
void cg2d_set_layer_clear(cg2d_t *c2d,int layerID, bool clear);

//reset layer transforms
void cg2d_layer_reset_transform(cg2d_t *c2d,int layerID);

//translate the layer
void cg2d_layer_translate(cg2d_t *c2d, int layerID, float x, float y);

//scale the layer
void cg2d_layer_scale(cg2d_t *c2d, int layerID, float x, float y);

//rotate the layer
void cg2d_layer_rotate(cg2d_t *c2d, int layerID, float degrees);

//clear a layer. if you are using a layer to render to each frame, say updating sprite 
//positions or drawing primitives you will need to clear the layer at the start of each
//frame before drawing to is or buffers will overrun
//if you just want to use the layer to hold static imagery, there is no need to clear it.
void cg2d_clear_layer(cg2d_t *c2d,int layerID);

//draw a layer to the given texture
void cg2d_draw_layer(cg2d_t *c2d,int layerID,SDL_GPUCommandBuffer* cmdBuf, SDL_GPUTexture *renderTarget);



//set the clear colour
void cg2d_set_cls_colour(cg2d_t *c2d,int r,int g, int b);

//get the clear colour
void cg2d_get_cls_colour(cg2d_t *c2d,int *r, int *g, int *b);

//set the colour. this is a tint to anything being drawn
void cg2d_set_colour(cg2d_t *c2d,int r,int g, int b);

//get the colour
void cg2d_get_colour(cg2d_t *c2d,int *r, int *g, int *b);

//set the global mask colour
void cg2d_set_mask_colour(cg2d_t *c2d,int r,int g, int b);

//get the global mask colour
void cg2d_get_mask_colour(cg2d_t *c2d,int *r, int *g, int *b);

//set the global mask alpha. it's unlikely you'll need this
void cg2d_set_mask_alpha(cg2d_t *c2d,float alpha);

//get the global alpha transparency
float cg2d_get_alpha(cg2d_t *c2d);

//set the global alpha transparency
void cg2d_set_alpha(cg2d_t *c2d,float alpha);

//set the handle. the handle is an offset from where an image/rect/ect.
//is drawn from, rotated around, scaled etc. 0,0 is top left corner, 1,1 is
//bottom right corner
void cg2d_set_handle(cg2d_t *c2d,float x, float y);

//get the current handle
void cg2d_get_handle(cg2d_t *c2d,float *x, float *y);

//set global rotation
void cg2d_set_rotation(cg2d_t *c2d,float degrees);

//get global rotation
float cg2d_get_rotation(cg2d_t *c2d);

//set global scale
void cg2d_set_scale(cg2d_t *c2d,float x, float y);

//get global scale
void cg2d_get_scale(cg2d_t *c2d,float *x,float *y);

//set gobal skew
void cg2d_set_skew(cg2d_t *c2d,float x, float y);

//get global skew
void cg2d_get_skew(cg2d_t *c2d,float *x, float *y);

//set outline width - for reccts and ovals drawn with the outline flag set
void cg2d_set_outline_width(cg2d_t *c2d,float width);

//get the outline width
float cg2d_get_outlinew_width(cg2d_t *c2d);

//set global rotation and scale
void cg2d_set_transform(cg2d_t *c2d,float degrees, float x, float y);

//set an image specific handle
void cg2d_set_image_handle(cg2d_image *img, float x, float y);

//automatically midhandle images on creation
void cg2d_auto_mid_handle(cg2d_t *c2d,bool enable);

//set an image handle to it's center
void cg2d_mid_handle_image(cg2d_image *img);


//set point size  - points drawn with cg2d_plot are effects
void cg2d_set_point_size(cg2d_t *c2d,float sz);

//get point size
float cg2d_get_point_size(cg2d_t *c2d);

//set line width
void cg2d_set_line_width(cg2d_t *c2d,float w);

//get line width
float cg2d_get_line_width(cg2d_t *c2d);

//get the width of an image
float cg2d_image_width(cg2d_image *img);

//get the height of an image
float cg2d_image_height(cg2d_image *img);

//set an image's colour mask
void cg2d_set_image_mask(cg2d_image *img,int r,int g, int b);

//get an images colour mask
void cg2d_get_image_mask(cg2d_image *img,int *r, int *g, int *b);

//draw an image to the current active layer. takes scale, rotation, colour, alpha, 
//handle, origin etc. into account
void cg2d_draw_image(cg2d_t *c2d, cg2d_image *img,float x, float y);

//draw an untextured rect to the current active layer. takes scale, rotation, colour, alpha, 
//handle, origin etc. into account
void cg2d_draw_rect(cg2d_t *c2d, float x, float y, float w, float h,bool outline);

//tiles an image x * y times starting at x,y
void cg2d_tile_image(cg2d_t *c2d,cg2d_image *img, float x, float y,int numx,int numy);

//draw an textured rect to the current active layer. takes scale, rotation, colour, alpha, 
//handle, origin etc. into account
void cg2d_draw_image_rect(cg2d_t *c2d,cg2d_image *img, float x, float y, float w, float h);

//this function works as a kind of manual mode for texture atlas style images
//sx,sy etc are offsets to the uv coords and dimension. in 0.0...1.0f range
//sx and sy are offsets to the image u and v coord
//sw and sh are new whole values representing the sub width and height. these
//are naively clamped to make sure bounds are not exceeded.
//giving sx,sy etc dimensions that exceed the bounds of the image UV rect
//will produce strange results, so don't do that!
//check example for usage
void cg2d_draw_sub_image_rect(cg2d_t *c2d,cg2d_image *img, float x, float y, float w, float h,
							   float sx, float sy, float sw, float sh);

//draw an untextured oval to the current active layer. takes scale, rotation, colour, alpha, 
//handle, origin etc. into account
void cg2d_draw_oval(cg2d_t *c2d, float x, float y, float w, float h,bool outline);


//draw an untextured point to the current active layer. takes scale, rotation, colour, alpha, 
//handle, origin etc. into account. 
//the rounded param lets you draw round points or square ones
void cg2d_plot(cg2d_t *c2d,float x, float y,bool rounded,bool gradiated);

//draw an untextured line between 2 points
void cg2d_draw_line(cg2d_t *c2d,float x1, float y1, float x2, float y2);

//draw an textured line between 2 points
void cg2d_draw_image_line(cg2d_t *c2d,cg2d_image *img,float x1, float y1, float x2, float y2);

//the width of a string of debug text
float cg2d_text_width(cg2d_t *c2d, char *text);

//the height of a string of debug text
float cg2d_text_height(cg2d_t *c2d, char *text);

//draw a string of debug text - takes the usual transfom factors into account
void cg2d_draw_text(cg2d_t *c2d,char *text,float x, float y);
























///////////////////////////
/////////code starts here...
/////////////////////////


/*
Notes to self:


	

//mode is the drawmode - image, point, line, oval etc.
//plot is type 0
//line is type 1
//rect is type 2
//image is type 3
//oval is type 4
//textured line/image line is 5
//outline rect is 6
//outline oval is 7
//gradiated point is 9 (this is where the colour fades in intensity the further it gets from the center)
//type is set to 1 if the blend is maskblend. this is so it's easier to
//discard fragments that have low alpha. I can't remember why i did
//this originally (this is a port of a port) though i'm sure it made sense at the time.


*/

//helper functions for angle in degrees
float cg2d_sin(float ang){
	return SDL_sinf(ang*CG2D_DEG2RAD);
}
float cg2d_cos(float ang){
	return SDL_cosf(ang*CG2D_DEG2RAD);
}
float cg2d_tan(float ang){
	return SDL_tanf(ang*CG2D_DEG2RAD);
}
float cg2d_atan(float ang){
	return SDL_atanf(ang*CG2D_DEG2RAD);
}
float cg2d_atan2(float y,float x){
	return SDL_atan2f(y,x)*CG2D_RAD2DEG;
}
float cg2d_asin(float ang){
	return SDL_asinf(ang*CG2D_DEG2RAD);
}
float cg2d_acos(float ang){
	return SDL_acosf(ang*CG2D_DEG2RAD);
}

//matrix functions
_cg2d_mat4 _cg2d_mat4_Multiply(_cg2d_mat4 matrix1, _cg2d_mat4 matrix2){
	_cg2d_mat4 result;

	result.m11 = ((matrix1.m11 * matrix2.m11) +	(matrix1.m12 * matrix2.m21) +(matrix1.m13 * matrix2.m31) +	(matrix1.m14 * matrix2.m41)	);
	result.m12 = ((matrix1.m11 * matrix2.m12) +	(matrix1.m12 * matrix2.m22) +(matrix1.m13 * matrix2.m32) +	(matrix1.m14 * matrix2.m42)	);
	result.m13 = ((matrix1.m11 * matrix2.m13) +	(matrix1.m12 * matrix2.m23) +(matrix1.m13 * matrix2.m33) +	(matrix1.m14 * matrix2.m43)	);
	result.m14 = ((matrix1.m11 * matrix2.m14) +	(matrix1.m12 * matrix2.m24) +(matrix1.m13 * matrix2.m34) +	(matrix1.m14 * matrix2.m44)	);
	result.m21 = ((matrix1.m21 * matrix2.m11) +	(matrix1.m22 * matrix2.m21) +(matrix1.m23 * matrix2.m31) +	(matrix1.m24 * matrix2.m41)	);
	result.m22 = ((matrix1.m21 * matrix2.m12) +	(matrix1.m22 * matrix2.m22) +(matrix1.m23 * matrix2.m32) +	(matrix1.m24 * matrix2.m42)	);
	result.m23 = ((matrix1.m21 * matrix2.m13) +	(matrix1.m22 * matrix2.m23) +(matrix1.m23 * matrix2.m33) +	(matrix1.m24 * matrix2.m43)	);
	result.m24 = ((matrix1.m21 * matrix2.m14) +	(matrix1.m22 * matrix2.m24) +(matrix1.m23 * matrix2.m34) +	(matrix1.m24 * matrix2.m44)	);
	result.m31 = ((matrix1.m31 * matrix2.m11) +	(matrix1.m32 * matrix2.m21) +(matrix1.m33 * matrix2.m31) +	(matrix1.m34 * matrix2.m41)	);
	result.m32 = ((matrix1.m31 * matrix2.m12) +	(matrix1.m32 * matrix2.m22) +(matrix1.m33 * matrix2.m32) +	(matrix1.m34 * matrix2.m42)	);
	result.m33 = ((matrix1.m31 * matrix2.m13) +	(matrix1.m32 * matrix2.m23) +(matrix1.m33 * matrix2.m33) +	(matrix1.m34 * matrix2.m43)	);
	result.m34 = ((matrix1.m31 * matrix2.m14) +	(matrix1.m32 * matrix2.m24) +(matrix1.m33 * matrix2.m34) +	(matrix1.m34 * matrix2.m44)	);
	result.m41 = ((matrix1.m41 * matrix2.m11) +	(matrix1.m42 * matrix2.m21) +(matrix1.m43 * matrix2.m31) +	(matrix1.m44 * matrix2.m41)	);
	result.m42 = ((matrix1.m41 * matrix2.m12) +	(matrix1.m42 * matrix2.m22) +(matrix1.m43 * matrix2.m32) +	(matrix1.m44 * matrix2.m42)	);
	result.m43 = ((matrix1.m41 * matrix2.m13) +	(matrix1.m42 * matrix2.m23) +(matrix1.m43 * matrix2.m33) +	(matrix1.m44 * matrix2.m43)	);
	result.m44 = ((matrix1.m41 * matrix2.m14) +	(matrix1.m42 * matrix2.m24) +(matrix1.m43 * matrix2.m34) +	(matrix1.m44 * matrix2.m44)	);

	return result;
}

_cg2d_mat4 _cg2d_mat4_CreateRotationZ(float radians){

	return (_cg2d_mat4) {
		 SDL_cosf(radians), SDL_sinf(radians), 0, 0,
		-SDL_sinf(radians), SDL_cosf(radians), 0, 0,
						 0, 				0, 1, 0,
						 0,					0, 0, 1
	};

}

_cg2d_mat4 _cg2d_mat4_CreateTranslation(float x, float y, float z){
	return (_cg2d_mat4) {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		x, y, z, 1
	};
}

_cg2d_mat4 _cg2d_mat4_CreateIdentity(){
	return (_cg2d_mat4) {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
}

_cg2d_mat4 _cg2d_mat4_CreateOrthographic(float left, float right, float bottom, float top,  float nearVal, float farVal){
	float tx = -((right + left) / (right - left));
	float ty = -((top + bottom) / (top - bottom));
	float tz = -((farVal + nearVal) / (farVal - nearVal));

	_cg2d_mat4 mat=_cg2d_mat4_CreateIdentity();
	mat.m11 = 2 / (right - left);
	mat.m22 = 2 / (top - bottom);
	mat.m33 = -2 / (farVal - nearVal);
	mat.m41 = tx;
	mat.m42 = ty;
	mat.m43 = tz;

	return mat;
}

_cg2d_mat4 _cg2d_mat4_CreateScaling(  float x,  float y,  float z){
	return (_cg2d_mat4) {
		x, 0, 0, 0,
		0, y, 0, 0,
		0, 0, z, 0,
		0, 0, 0, 1
	};
}



void cg2d_init(cg2d_t *c2d,SDL_Window *window,SDL_GPUDevice *device, int scrnW,int scrnH){

	c2d->ready=false;

	c2d->vpx=0;
	c2d->vpy=0;
	c2d->vpw=scrnW;
	c2d->vph=scrnH;

	int sw,sh;
    int scw,sch;
    SDL_GetWindowSize(window,&sw,&sh);
    SDL_GetWindowSizeInPixels(window, &scw, &sch);
   
    c2d->winW=scw/sw;
    c2d->winH=sch/sh;
    
    c2d->virtW=scrnW;
    c2d->virtH=scrnH;

    c2d->virtMXscale=((float)scrnW)/(float)scrnW;
    c2d->virtMYscale=((float)scrnH)/(float)scrnH;

	
	c2d->hx=0.0f;
	c2d->hy=0.0f;

	c2d->automidhandle=false;

	c2d->rotation=0.0f;
	c2d->scale_x=1.0f;
	c2d->scale_y=1.0f;
	c2d->skewx=0.0f;
	c2d->skewy=0.0f;

	c2d->cls=true;
	c2d->clsr=0;
	c2d->clsg=0;
	c2d->clsb=0;
	
	c2d->colr=c2d->colg=c2d->colb=c2d->alpha=255;
	c2d->mr=c2d->mg=c2d->mb=0;
	c2d->ma=1.0;

	c2d->outlinew=0.05;

	c2d->linew=1.0f;
	c2d->pointsz=1.0f;

	c2d->buffers=NULL;
	c2d->currentBuffer=c2d->bufferCount=0;

	c2d->imageList=NULL;
	
	//c2d->useDebugFont=false;

	c2d->device=device;
	c2d->window=window;

	c2d->fontInitialised=false;
	c2d->fontcnt=0;
	c2d->currentFont=0;

	c2d->textEngine = TTF_CreateGPUTextEngine(c2d->device);
    if(!c2d->textEngine){
    	SDL_Log("textengine load failed\n");    	
    }else{    
    	SDL_Log("created textengine\n");
    }
}

void cg2d_free(cg2d_t *c2d){

	if(c2d->imageList!=NULL){
		arrfree(c2d->imageList);
	}else{
		SDL_Log("imagelist is null\n");
	}
	if(c2d->fontInitialised==true){
		for(int i=0;i<arrlen(c2d->fonts);i++){
    		TTF_CloseFont(c2d->fonts[i].font);
    		SDL_ReleaseGPUSampler(c2d->device,c2d->fonts[i].tex.samp);
		}
		TTF_DestroyGPUTextEngine(c2d->textEngine);
		arrfree(c2d->fonts);
	}


	for(int i=0;i<arrlen(c2d->buffers);i++){
		arrfree(c2d->buffers[i].verts);
		SDL_ReleaseGPUGraphicsPipeline(c2d->device,c2d->buffers[i].RenderPipeline);
		SDL_ReleaseGPUTransferBuffer(c2d->device, c2d->buffers[i].tbuffer);
		SDL_ReleaseGPUBuffer(c2d->device,c2d->buffers[i].vbuffer);;

	}

}

cg2d_font *cg2d_load_image_font(cg2d_t *c2d,char *url,int sz, SDL_GPUFilter filter){
	
	cg2d_font font;
	font.id=0;
	font.ptsz=sz;
	font.tex.tex=NULL;
	font.tex.samp=NULL;
	font.font=NULL;
	font.text=NULL;

	
    size_t fontSize =0;
    void *fontData=load_file_storage(url,&fontSize);
    if(fontData==NULL){
    	SDL_Log("Error readng font data\n");
    	return NULL;
    }

    font.font=TTF_OpenFontIO(SDL_IOFromMem(fontData,fontSize),true, sz); /* Preferably use a Monospaced font */
    if (!font.font) {
        SDL_Log("error loading font %s\n",url);
        return NULL;
    }else{
    	SDL_Log("loaded %s\n", url);
    }
   TTF_SetFontWrapAlignment(font.font, TTF_HORIZONTAL_ALIGN_LEFT);
   

    font.text=TTF_CreateText(c2d->textEngine, font.font, "A", 0);
    if (!font.text) {
       SDL_Log("text NULL\n");
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return NULL;
    }else{
    	SDL_Log("created text\n");
    }

   SDL_GPUSamplerCreateInfo sampler_info = {
        .min_filter = filter,
        .mag_filter = filter,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
    };
    font.tex.samp = SDL_CreateGPUSampler(c2d->device, &sampler_info);
    if(!font.tex.samp){
    	SDL_Log("sampler create failer\n");
    	return NULL;
    }else{
    	SDL_Log("created sampler\n");
    }
    

    c2d->fontInitialised=true;
    font.id=c2d->fontcnt++;
    
    arrput(c2d->fonts,font);

    return (cg2d_font*)&c2d->fonts[c2d->fontcnt-1];
}

void cg2d_set_image_font(cg2d_t *c2d, cg2d_font *font){
	if(font!=NULL){
		c2d->currentFont=font->id;
	}
}

cg2d_font *cg2d_get_image_font(cg2d_t *c2d){
	if(c2d->fontInitialised==true){
		return &c2d->fonts[c2d->currentFont];
	}else{
		return NULL;
	}
}

void cg2d_delete_image(cg2d_t *c2d, cg2d_image *img){
	arrdelswap(c2d->imageList,img->index);
}


//creates an image for the given texture
cg2d_image *cg2d_create_image(cg2d_t *c2d,char *name,cg_texture *tex,int width,int height){

	if(name==NULL || tex==NULL){
		SDL_Log("c2d load image - tex or name is null\n");
		return NULL;
	}

	cg2d_image m2Img;
	SDL_strlcpy(m2Img.name,name,SDL_strlen(name)+1);
	m2Img.x=0.0f;
	m2Img.y=0.0f;
	m2Img.w=width;
	m2Img.h=height;		
	if(c2d->automidhandle==true){
		m2Img.hx=m2Img.hy=0.5f;
	}else{
		m2Img.hx=m2Img.hy=0.0f;
	}	
	m2Img.u=0.0f;
	m2Img.v=0.0f;
	m2Img.uw=1.0f;
	m2Img.vh=1.0f;
	m2Img.ox=0.0f;
	m2Img.oy=0.0f;
	m2Img.ow=tex->w;
	m2Img.oh=tex->h;
	m2Img.mr=c2d->mr;
	m2Img.mg=c2d->mg;	
	m2Img.mb=c2d->mb;
	m2Img.ma=c2d->ma;

	
	m2Img.index=arrlenu(c2d->imageList);

	if(c2d->imageList==NULL){
		//set an initial capacity
		arrsetcap(c2d->imageList,100);
	}
	arrput(c2d->imageList,m2Img);


	return (cg2d_image*)&c2d->imageList[arrlenu(c2d->imageList)-1];

}

//images are stored on an internal list to be free'd by cg2d_free
cg2d_image* cg2d_load_atlas_image(cg2d_t *c2d,texAtlas *atlas,char* imgName){

	texAtlasImage *atImg=NULL;
	cg2d_image m2Img;

	for(int i=0;i<atlas->count;i++){
		if(SDL_strcmp(imgName,atlas->images[i].name)==0){
			atImg=&atlas->images[i];
			break;
		}
	}
	
	if(atImg!=NULL){
		SDL_strlcpy(m2Img.name,atImg->name,SDL_strlen(atImg->name)+1);
		m2Img.x=atImg->x;
		m2Img.y=atImg->y;
		m2Img.w=atImg->w;
		m2Img.h=atImg->h;		
		if(c2d->automidhandle==true){
			m2Img.hx=m2Img.hy=0.5f;
		}else{
			m2Img.hx=m2Img.hy=0.0f;
		}	
		m2Img.u=atImg->u;
		m2Img.v=atImg->v;
		m2Img.uw=atImg->uw;
		m2Img.vh=atImg->vh;
		m2Img.ox=atImg->trimOffsetX;
		m2Img.oy=atImg->trimOffsetY;
		m2Img.ow=atImg->untrimmedWidth;
		m2Img.oh=atImg->untrimmedHeight;
		m2Img.mr=c2d->mr;
		m2Img.mg=c2d->mg;	
		m2Img.mb=c2d->mb;
		m2Img.ma=c2d->ma;
		m2Img.index=arrlenu(c2d->imageList);
	
	}else{
		SDL_Log("no match found for %s\n",imgName );
		return NULL;
	}

	if(c2d->imageList==NULL){
		//set an initial capacity
		arrsetcap(c2d->imageList,100);
	}
	arrput(c2d->imageList,m2Img);

	return (cg2d_image*)&c2d->imageList[arrlenu(c2d->imageList)-1];
}

void cg2d_print_image_data(cg2d_image *img){
	SDL_Log("image %s: x=%f, y=%f, w=%f, h=%f, hx=%f, hy=%f, u=%f, v=%f, uw=%f, vh=%f, mr=%f, mg=%f, mb=%f, ma=%f\n",
			img->name ,img->x,img->y,img->w,img->h,img->hx,img->hy,img->u,img->v,img->uw,img->vh,img->mr,img->mg,img->mb,img->ma);
}




//add a buffer layer. returns the layer id
int cg2d_add_layer(cg2d_t *c2d, cg2d_blend blend,cg_texture *tex,SDL_GPUShader* vertex_shader,SDL_GPUShader* fragment_shader,SDL_GPULoadOp loadop){
	
	cg2d_buffer buf;
	buf.id=c2d->bufferCount++;
	buf.blend=blend;
	buf.tex=tex->tex;
	buf.samp=tex->samp;
	buf.load_op=loadop;
	buf.used=false;
	buf.transform=(_cg2d_mat4) {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	SDL_GPUBlendFactor src;
	SDL_GPUBlendFactor dst;
	bool enableBlend=false;
	switch(blend){
		case CG2D_MASKBLEND:
			enableBlend=true;//disbles fragments with alpha <0.5; done in shader
			src=SDL_GPU_BLENDFACTOR_SRC_ALPHA;
			dst=SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
		case CG2D_SOLIDBLEND:
			enableBlend=false;
			src=SDL_GPU_BLENDFACTOR_SRC_ALPHA;
			dst=SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
			break;
		case CG2D_ALPHABLEND:
			enableBlend=true;
			src=SDL_GPU_BLENDFACTOR_SRC_ALPHA;
			dst=SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
			break;
		case CG2D_LIGHTBLEND:
			enableBlend=true;
			src=SDL_GPU_BLENDFACTOR_SRC_ALPHA;
			dst=SDL_GPU_BLENDFACTOR_ONE;
			break;
		case CG2D_SHADEBLEND:
			enableBlend=true;
			src=SDL_GPU_BLENDFACTOR_DST_COLOR;
			dst=SDL_GPU_BLENDFACTOR_ZERO;
			break;
		default:
			enableBlend=false;
			break;
	}

	buf.vbufsz=CG2D_MAX_VERTICES;
    buf.vbuflen=0;
    buf.verts=NULL;

	buf.RenderPipeline=  SDL_CreateGPUGraphicsPipeline(
       c2d->device,
        &(SDL_GPUGraphicsPipelineCreateInfo){
            .target_info = (SDL_GPUGraphicsPipelineTargetInfo){
                .num_color_targets = 1,
                .color_target_descriptions = (SDL_GPUColorTargetDescription[]){{
                    .format = SDL_GetGPUSwapchainTextureFormat(c2d->device, c2d->window),
                    .blend_state = {
                        .enable_blend = enableBlend,
                        .color_blend_op = SDL_GPU_BLENDOP_ADD,
                        .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
                        .src_color_blendfactor = src,
                        .src_alpha_blendfactor = src,                        
                        .dst_color_blendfactor = dst,
                        .dst_alpha_blendfactor = dst,
                    }
                }}
            },
            .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
            .vertex_shader = vertex_shader,
            .fragment_shader = fragment_shader
        }
    );

    buf.tbuffer= SDL_CreateGPUTransferBuffer(
        c2d->device,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = CG2D_MAX_VERTICES * sizeof(cg2d_vertex)
        }
    );

    buf.vbuffer = SDL_CreateGPUBuffer(
        c2d->device,
        &(SDL_GPUBufferCreateInfo) {
            .usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
            .size = CG2D_MAX_VERTICES * sizeof(cg2d_vertex)
        }
    );

    

    
    arrsetcap(buf.verts,10000);    

    arrput(c2d->buffers,buf);

    return buf.id;

}

void cg2d_set_layer_texture(cg2d_t *c2d, int layerID, cg_texture *tex){
	if(layerID>=0 && layerID<c2d->bufferCount){
		c2d->buffers[layerID].tex=tex->tex;
		c2d->buffers[layerID].samp=tex->samp;
		
	}else{
		//defualt to layer 0
		SDL_Log("max2d set layer texture - invalid layerID. \n");
		
	}
	return;
}

void cg2d_set_layer(cg2d_t *c2d, int layerID){
	if(layerID>=0 && layerID<c2d->bufferCount){
		c2d->currentBuffer=layerID;
	}else{
		//defualt to layer 0
		SDL_Log("max2d set layer - invalid layerID. defaulting to 0\n");
		c2d->currentBuffer=0;
	}
	return;
}

int cg2d_get_current_layer(cg2d_t *c2d){
	if(c2d->bufferCount>0){
		return c2d->currentBuffer;
	}
	SDL_Log("c2d get current layer - no layers set.\n");
	return -1;
}

int cg2d_get_layer_vertex_count(cg2d_t *c2d,int layerID){
	return arrlenu(c2d->buffers[layerID].verts);
}

void cg2d_set_layer_clear(cg2d_t *c2d,int layerID, bool clear){
	if(layerID>=0 && layerID<c2d->bufferCount){
		c2d->buffers[layerID].load_op=(clear==true) ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
	}else{
		//defualt  to layer 0
		SDL_Log("max2d set layer clear - invalid layerID. defaulting to 0\n");
		c2d->currentBuffer=0;
	}
	return;
}

void cg2d_layer_reset_transform(cg2d_t *c2d,int layerID){
	if(layerID>=0 && layerID<c2d->bufferCount){
		c2d->buffers[layerID].transform=_cg2d_mat4_CreateIdentity();
	}
}

void cg2d_layer_translate(cg2d_t *c2d, int layerID, float x, float y){
	if(layerID>=0 && layerID<c2d->bufferCount){
		_cg2d_mat4 trans=_cg2d_mat4_CreateTranslation(x,y,0);
		c2d->buffers[layerID].transform=_cg2d_mat4_Multiply(c2d->buffers[layerID].transform,trans);
	}
}

void cg2d_layer_scale(cg2d_t *c2d, int layerID, float x, float y){
	if(layerID>=0 && layerID<c2d->bufferCount){
		_cg2d_mat4 trans=_cg2d_mat4_CreateScaling(x,y,0);
		c2d->buffers[layerID].transform=_cg2d_mat4_Multiply(c2d->buffers[layerID].transform,trans);
	}
}

void cg2d_layer_rotate(cg2d_t *c2d, int layerID, float degrees){
	if(layerID>=0 && layerID<c2d->bufferCount){
		_cg2d_mat4 trans=_cg2d_mat4_CreateRotationZ(degrees*CG2D_DEG2RAD);
		c2d->buffers[layerID].transform=_cg2d_mat4_Multiply(c2d->buffers[layerID].transform,trans);
	}
}




void cg2d_set_cls_colour(cg2d_t *c2d,int r,int g, int b){
	c2d->clsr=SDL_min(SDL_max(r,0),255);
	c2d->clsg=SDL_min(SDL_max(g,0),255);
	c2d->clsb=SDL_min(SDL_max(b,0),255);
}

void cg2d_get_cls_colour(cg2d_t *c2d,int *r, int *g, int *b){
	*r=c2d->clsr;
	*g=c2d->clsg;
	*b=c2d->clsb;	
}

void cg2d_set_colour(cg2d_t *c2d,int r,int g, int b){
	c2d->colr=SDL_min(SDL_max(r,0),255);
	c2d->colg=SDL_min(SDL_max(g,0),255);
	c2d->colb=SDL_min(SDL_max(b,0),255);
}

void cg2d_get_colour(cg2d_t *c2d,int *r, int *g, int *b){
	*r=c2d->colr;
	*g=c2d->colg;
	*b=c2d->colb;
	
}

void cg2d_set_mask_colour(cg2d_t *c2d,int r,int g, int b){
	c2d->mr=SDL_min(SDL_max(r,0),255);
	c2d->mg=SDL_min(SDL_max(g,0),255);
	c2d->mb=SDL_min(SDL_max(b,0),255);
}

void cg2d_get_mask_colour(cg2d_t *c2d,int *r, int *g, int *b){
	*r=c2d->mr;
	*g=c2d->mg;
	*b=c2d->mb;
	
}

void cg2d_set_mask_alpha(cg2d_t *c2d,float alpha){
	c2d->alpha=SDL_min(SDL_max(alpha,0.0f),1.0f);
}

float cg2d_get_alpha(cg2d_t *c2d){
	return c2d->alpha;
}

void cg2d_set_alpha(cg2d_t *c2d,float alpha){
	c2d->alpha=SDL_min(SDL_max(alpha,0.0f),1.0f);// alpha;
}


void cg2d_set_viewport(cg2d_t *c2d,int x, int y, int w, int h){
	c2d->vpx=x;
	c2d->vpy=y;
	c2d->vpw=w;
	c2d->vph=h;

	int sw,sh;
    int scw,sch;
	SDL_GetWindowSize(c2d->window,&sw,&sh);
    SDL_GetWindowSizeInPixels(c2d->window, &scw, &sch);
   
    c2d->winW=scw/sw;
    c2d->winH=sch/sh;



}

//returns scaled coords for use with SDL_SetGPUViewport
void cg2d_get_viewport(cg2d_t *c2d,int *x, int *y, int *w, int *h){
	*x=c2d->vpx*c2d->winW;
	*y=c2d->vpy*c2d->winH;
	*w=c2d->vpw*c2d->winW;
	*h=c2d->vph*c2d->winH;
	return;
}



void cg2d_set_handle(cg2d_t *c2d,float x, float y){
	c2d->hx=SDL_min(SDL_max(x,0.0f),1.0f);
	c2d->hy=SDL_min(SDL_max(y,0.0f),1.0f);
}

void cg2d_get_handle(cg2d_t *c2d,float *x, float *y){
	*x=c2d->hx;
	*y=c2d->hy;
	return;
}

void cg2d_set_skew(cg2d_t *c2d,float x, float y){
	c2d->skewx=x;
	c2d->skewy=y;
}

void cg2d_get_skew(cg2d_t *c2d,float *x, float *y){
	*x=c2d->skewx;
	*y=c2d->skewy;
	return;
}

void cg2d_set_outline_width(cg2d_t *c2d,float width){
	c2d->outlinew=SDL_min(SDL_max(width,0.0),1.0);
	
	
}

float cg2d_get_outlinew_width(cg2d_t *c2d){
	
	return c2d->outlinew;
}


void cg2d_set_rotation(cg2d_t *c2d,float degrees){
	c2d->rotation=degrees;
	
}

float cg2d_get_rotation(cg2d_t *c2d){
	return c2d->rotation;
}


void cg2d_set_scale(cg2d_t *c2d,float x, float y){
	c2d->scale_x=x;
	c2d->scale_y=y;
}

void cg2d_get_scale(cg2d_t *c2d,float *x,float *y){
	*x=c2d->scale_x;
	*y=c2d->scale_y;
}

void cg2d_set_transform(cg2d_t *c2d,float degrees, float x, float y){
	cg2d_set_rotation(c2d,degrees);
	cg2d_set_scale(c2d,x,y);
}


void cg2d_set_image_handle(cg2d_image *img, float x, float y){
	img->hx=SDL_min(SDL_max(x,0.0f),1.0f);
	img->hy=SDL_min(SDL_max(y,0.0f),1.0f);
}

void cg2d_auto_mid_handle(cg2d_t *c2d,bool enable){
	c2d->automidhandle=enable;
}

void cg2d_mid_handle_image(cg2d_image *img){
	img->hx=0.5f;
	img->hy=0.5f;		
}

void cg2d_set_point_size(cg2d_t *c2d,float sz){
	c2d->pointsz=sz;
}

float cg2d_get_point_size(cg2d_t *c2d){
	return c2d->pointsz;
}

void cg2d_set_line_width(cg2d_t *c2d,float w){
	c2d->linew=w;
}

float cg2d_get_line_width(cg2d_t *c2d){
	return c2d->linew;
}

float cg2d_image_width(cg2d_image *img){
	return img->w;//img->width;
}

float cg2d_image_height(cg2d_image *img){
	return img->h;//img->height;
}

void cg2d_set_image_mask(cg2d_image *img,int r,int g, int b){
	img->mr=SDL_min(SDL_max(r,0),255);
	img->mg=SDL_min(SDL_max(g,0),255);
	img->mb=SDL_min(SDL_max(b,0),255);
}

void cg2d_get_image_mask(cg2d_image *img,int *r, int *g, int *b){
	*r=img->mr;
	*g=img->mg;
	*b=img->mb;	
}


//add a vertex to the current layer buffer.
//could send a custom image for lines/rects/etc.???
void _cg2d_add_vertex(cg2d_t *c2d,
					   cg2d_image *img,
					   float x,
					   float y,
					   float type,
					   float mode){
	//SDL_assert_release(arrlen(c2d->buffers[c2d->currentBuffer].verts)+1<CG2D_MAX_VERTICES);
	if(arrlen(c2d->buffers[c2d->currentBuffer].verts)+1>=CG2D_MAX_VERTICES){
		SDL_Log("WARNING - vertices exceed maximum. change CG2D_MAX_VERTICES if you need extra\n");
		return;
	}
	cg2d_vertex vert;

	vert.x=x;
	vert.y=y;
	vert.z=0.0f;
	
	vert.rotation=c2d->rotation*CG2D_DEG2RAD;
    
    vert.scalex=c2d->scale_x;
    vert.scaley=c2d->scale_y;
    
    //vert.mode=(c2d->buffers[c2d->currentBuffer].blend=CG2D_MASKBLEND) ? 1 : 0;
    vert.type=type;
    vert.mode=mode;

    vert.r=(float)c2d->colr/255.0f;
    vert.g=(float)c2d->colg/255.0f;
    vert.b=(float)c2d->colb/255.0f;
    vert.a=c2d->alpha;

	vert.mr=img->mr/255.0f;
    vert.mg=img->mg/255.0f;
    vert.mb=img->mb/255.0f;
    vert.ma=img->ma;
    

	vert.hx=img->hx;
    vert.hy=img->hy;
    vert.w=img->w;
    vert.h=img->h;

    vert.u=img->u;
    vert.v=img->v;
    vert.uw=img->uw;
    vert.vh=img->vh;

    vert.skewx=c2d->skewx;
	vert.skewy=c2d->skewy;
	
	vert.yy=0.0f;

	vert.outlinew=c2d->outlinew;

    	    
	//copy to the buffer array
    arrput(c2d->buffers[c2d->currentBuffer].verts,vert);

}

void cg2d_draw_image(cg2d_t *c2d, cg2d_image *img,float x, float y){

	_cg2d_add_vertex(c2d,img,x,y,3,(c2d->buffers[c2d->currentBuffer].blend==CG2D_MASKBLEND) ? 1.0:0.0);

}

void cg2d_draw_rect(cg2d_t *c2d, float x, float y, float w, float h,bool outline){

	cg2d_image rect;
	rect.x=0.f;
	rect.y=0.f;
	rect.w=w;
	rect.h=h;

	rect.hx=c2d->hx;
	rect.hy=c2d->hy;
	
	rect.u=0.0f;
	rect.v=0.0f;
	rect.uw=1.0f;
	rect.vh=1.0f;

	rect.mr=c2d->mr;
	rect.mg=c2d->mg;
	rect.mb=c2d->mb;
	rect.ma=c2d->ma;

	
	

	_cg2d_add_vertex(c2d,&rect,x,y,(outline==true) ? 6 : 2 ,
								   (c2d->buffers[c2d->currentBuffer].blend==CG2D_MASKBLEND) ? 1.0:0.0);


}

void cg2d_draw_image_rect(cg2d_t *c2d,cg2d_image *img, float x, float y, float w, float h){

	cg2d_image rect;
	rect.x=0.f;
	rect.y=0.f;
	rect.w=w;
	rect.h=h;

	rect.hx=c2d->hx;
	rect.hy=c2d->hy;
	
	rect.u=img->u;
	rect.v=img->v;
	rect.uw=img->uw;
	rect.vh=img->vh;

	rect.mr=img->mr;
	rect.mg=img->mg;
	rect.mb=img->mb;
	rect.ma=img->ma;

	

	_cg2d_add_vertex(c2d,&rect,x,y,3,(c2d->buffers[c2d->currentBuffer].blend==CG2D_MASKBLEND) ? 1.0:0.0);


}

//tiles an image into the current viewport x * y times
void cg2d_tile_image(cg2d_t *c2d,cg2d_image *img, float x, float y,int numx,int numy){

	int rptx=(img->w*numx)/numx;
	int rpty=(img->h*numy)/numy;
	
	for(int i=0;i<rptx;i++){
		for(int o=0;o<rpty;o++){
			cg2d_image rect;
			rect.x=0.f;
			rect.y=0.f;
			rect.w=rptx;
			rect.h=rpty;

			rect.hx=c2d->hx;
			rect.hy=c2d->hy;
			
			rect.u=img->u;
			rect.v=img->v;
			rect.uw=img->uw;
			rect.vh=img->vh;

			rect.mr=img->mr;
			rect.mg=img->mg;
			rect.mb=img->mb;
			rect.ma=img->ma;

			

		_cg2d_add_vertex(c2d,&rect,x+(i*rptx),y+(o*rpty),3,(c2d->buffers[c2d->currentBuffer].blend==CG2D_MASKBLEND) ? 1.0:0.0);

		}
	}
	

}




//this function works as a kind of manual mode for texture atlas style images
//sx,sy etc are offsets to the uv coords and dimension. in 0.0...1.0f range
//sx and sy are offsets to the image u and v coord
//sw and sh are new whole values representing the sub width and height. these
//are naively clamped to make sure bounds are not exceeded.
//giving sx,sy etc dimensions that exceed the bounds of the image UV rect
//will produce strange results, so don't do that!
//check example for usage
void cg2d_draw_sub_image_rect(cg2d_t *c2d,cg2d_image *img, float x, float y, float w, float h,
							   float sx, float sy, float sw, float sh){

	cg2d_image rect;
	rect.x=0.f;
	rect.y=0.f;
	rect.w=w;
	rect.h=h;

	rect.hx=c2d->hx;
	rect.hy=c2d->hy;
	

	rect.u=img->u+sx;
	rect.v=img->v+sy;
	rect.uw=sw;
	rect.vh=sh;

	rect.mr=img->mr;
	rect.mg=img->mg;
	rect.mb=img->mb;
	rect.ma=img->ma;

	

    if(rect.u < img->u) {
        rect.u = img->u;
    }
    if(rect.v < img->v) {
        rect.v = img->v;
    }
    if((rect.u+rect.uw)> (img->u+img->uw)) {
        
        rect.u=(img->u+img->uw)-rect.uw;
    }
    if((rect.v+rect.vh)> (img->v+img->vh)) {
        
        rect.v=(img->v+img->vh)-rect.vh;
    }
   


	_cg2d_add_vertex(c2d,&rect,x,y,3,(c2d->buffers[c2d->currentBuffer].blend==CG2D_MASKBLEND) ? 1.0:0.0);


}

void cg2d_draw_oval(cg2d_t *c2d, float x, float y, float w, float h,bool outline){

	cg2d_image rect;
	rect.x=0.f;
	rect.y=0.f;
	rect.w=w;
	rect.h=h;

	rect.hx=c2d->hx;
	rect.hy=c2d->hy;
	
	rect.u=0.0f;
	rect.v=0.0f;
	rect.uw=1.0f;
	rect.vh=1.0f;

	rect.mr=c2d->mr;
	rect.mg=c2d->mg;
	rect.mb=c2d->mb;
	rect.ma=c2d->ma;

	

	_cg2d_add_vertex(c2d,&rect,x,y,(outline==true) ? 7 : 4,
					(c2d->buffers[c2d->currentBuffer].blend==CG2D_MASKBLEND) ? 1.0:0.0);


}



void cg2d_plot(cg2d_t *c2d,float x, float y,bool rounded,bool gradiated){

	cg2d_image rect;
	rect.x=0.f;
	rect.y=0.f;
	rect.w=c2d->pointsz;
	rect.h=c2d->pointsz;

	rect.hx=0.5f;
	rect.hy=0.5f;
	
	rect.u=0.0f;
	rect.v=0.0f;
	rect.uw=1.0f;
	rect.vh=1.0f;

	rect.mr=c2d->mr;
	rect.mg=c2d->mg;
	rect.mb=c2d->mb;
	rect.ma=c2d->ma;

	

	int md=4;//oval by defualt
	if(gradiated==true){
		md=9;//gradiated by shader
	}

	_cg2d_add_vertex(c2d,&rect,x,y,(rounded==true)? md:0,(c2d->buffers[c2d->currentBuffer].blend==CG2D_MASKBLEND) ? 1.0:0.0);

}

void cg2d_draw_line(cg2d_t *c2d,float x1, float y1, float x2, float y2){

	float angle =SDL_atan2f(y2-y1,x2-x1)*CG2D_RAD2DEG;
	float c2drot=c2d->rotation;

	c2d->rotation+=angle;

	float dx,dy,length;
	dx=x1-x2;
	dy=y1-y2;
	length=SDL_sqrt(dx*dx+dy*dy);
	

	cg2d_image rect;
	rect.x=0.f;
	rect.y=0.f;
	rect.w=length;
	rect.h=c2d->linew;

	rect.hx=c2d->hx;
	rect.hy=c2d->hy;
	
	rect.u=0.0f;
	rect.v=0.0f;
	rect.uw=1.0f;
	rect.vh=1.0f;

	rect.mr=c2d->mr;
	rect.mg=c2d->mg;
	rect.mb=c2d->mb;
	rect.ma=c2d->ma;

	

	_cg2d_add_vertex(c2d,&rect,x1,y1,1,(c2d->buffers[c2d->currentBuffer].blend==CG2D_MASKBLEND) ? 1.0:0.0);




	c2d->rotation=c2drot;
}

void cg2d_draw_image_line(cg2d_t *c2d,cg2d_image *img,float x1, float y1, float x2, float y2){

	float angle =SDL_atan2f(y2-y1,x2-x1)*CG2D_RAD2DEG;
	float c2drot=c2d->rotation;

	c2d->rotation+=angle;

	float dx,dy,length;
	dx=x1-x2;
	dy=y1-y2;
	length=SDL_sqrt(dx*dx+dy*dy);
	

	cg2d_image rect;
	rect.x=0.f;
	rect.y=0.f;
	rect.w=length;
	rect.h=c2d->linew;

	rect.hx=c2d->hx;
	rect.hy=c2d->hy;
	
	rect.u=img->u;
	rect.v=img->v;
	rect.uw=img->uw;
	rect.vh=img->vh;

	rect.mr=c2d->mr;
	rect.mg=c2d->mg;
	rect.mb=c2d->mb;
	rect.ma=c2d->ma;

	

	_cg2d_add_vertex(c2d,&rect,x1,y1,5,(c2d->buffers[c2d->currentBuffer].blend==CG2D_MASKBLEND) ? 1.0:0.0);

	c2d->rotation=c2drot;
}

float cg2d_text_width(cg2d_t *c2d, char *text){
	if(c2d->fontInitialised==true){
		TTF_SetTextString(c2d->fonts[c2d->currentFont].text, text, 0);
       int tw,th;
       TTF_GetTextSize(c2d->fonts[c2d->currentFont].text,&tw,&th);
       return tw;
	}else{
		return -1;
	}

}

float cg2d_text_height(cg2d_t *c2d, char *text){
	if(c2d->fontInitialised==true){
		TTF_SetTextString(c2d->fonts[c2d->currentFont].text, text, 0);
       int tw,th;
       TTF_GetTextSize(c2d->fonts[c2d->currentFont].text,&tw,&th);
       return th;
	}else{
		return -1;
	}

}




// TTF_STYLE_NORMAL        0x00 /**< No special style */
// TTF_STYLE_BOLD          0x01 /**< Bold style */
// TTF_STYLE_ITALIC        0x02 /**< Italic style */
// TTF_STYLE_UNDERLINE     0x04 /**< Underlined text */
// TTF_STYLE_STRIKETHROUGH 0x08 /**< Strikethrough text */

void cg2d_set_font_style(cg2d_t* c2d, cg2d_font *font,TTF_FontStyleFlags style){
	//if(c2d->fontInitialised==true){
		TTF_SetFontStyle(c2d->fonts[font->id].font, style);
	// }else{
	// 	printf("cant set font style\n");
	// }
}

// TTF_DIRECTION_INVALID = 0,
// TTF_DIRECTION_LTR = 4,        /**< Left to Right */
// TTF_DIRECTION_RTL,            /**< Right to Left */
// TTF_DIRECTION_TTB,            /**< Top to Bottom */
// TTF_DIRECTION_BTT             /**< Bottom to Top */
void cg2d_set_font_direction(cg2d_t *c2d, cg2d_font *font, TTF_Direction dir){
	//if(c2d->fontInitialised==true){
		TTF_SetFontDirection(c2d->fonts[font->id].font, dir);
	//}
}


void cg2d_draw_text(cg2d_t *c2d,char *text,float x, float y){
	if(c2d->fontInitialised==true){
		float tw=cg2d_text_width(c2d,text);
		float th=cg2d_text_height(c2d,text); 

		float hx=c2d->hx;///2;
		float hy=c2d->hy;///2;
		float startX=-(hx)*tw;
		float startY=-(hy)*th;
		
		float vx,vy;

	    _cg2d_mat4 mat=_cg2d_mat4_CreateTranslation(x,y,0);
	    _cg2d_mat4 rot=_cg2d_mat4_CreateRotationZ(c2d->rotation*CG2D_DEG2RAD);
	    _cg2d_mat4 scl=_cg2d_mat4_CreateScaling(c2d->scale_x,c2d->scale_y,0.02);
	   
	    _cg2d_mat4 trans=_cg2d_mat4_Multiply(rot,scl);
	    trans=_cg2d_mat4_Multiply(trans,mat);
	    
	      
	    
		TTF_SetTextString(c2d->fonts[c2d->currentFont].text, text, 0);
		TTF_GPUAtlasDrawSequence *sequence =TTF_GetGPUTextDrawData(c2d->fonts[c2d->currentFont].text);
	      
	       c2d->fonts[c2d->currentFont].tex.tex=sequence->atlas_texture;
	       cg2d_set_layer_texture(c2d,c2d->currentBuffer,&c2d->fonts[c2d->currentFont].tex);
	       for ( ; sequence; sequence = sequence->next) {
	            
	           for(int i=0;i<sequence->num_indices;i+=6){
	                float mnx=FLT_MAX,mny=FLT_MAX,mxx=FLT_MIN,mxy=FLT_MIN;
	                float mnu=FLT_MAX,mnv=FLT_MAX,mxw=FLT_MIN,mxh=FLT_MIN;
	            
	                for(int o=0;o<6;o++){
	                    const SDL_FPoint pos=sequence->xy[sequence->indices[i+o]];
	                    mnx=SDL_min(mnx,pos.x);
	                    mny=SDL_min(mny,-pos.y);
	                    mxx=SDL_max(mxx,pos.x);
	                    mxy=SDL_max(mxy,-pos.y);
	                    const SDL_FPoint uv=sequence->uv[sequence->indices[i+o]];
	                    mnu=SDL_min(mnu,uv.x);
	                    mnv=SDL_min(mnv,uv.y);
	                    mxw=SDL_max(mxw,uv.x);
	                    mxh=SDL_max(mxh,uv.y);
	                    
	                }               

	                cg2d_image rect;
	                rect.x=mnx;
	                rect.y=mny;
	                rect.w=(mxx-mnx);
	                rect.h=(mxy-mny);

	                rect.hx=c2d->hx;
	                rect.hy=c2d->hy;
	                
	                rect.u=mnu;
	                rect.v=mnv;
	                rect.uw=mxw-mnu;
	                rect.vh=mxh-mnv;

	                rect.mr=c2d->mr;
	                rect.mg=c2d->mg;
	                rect.mb=c2d->mb;
	                rect.ma=c2d->ma;

	                

	                vx = (startX+mnx) * trans.m11 + (startY+mny) * trans.m21 + 0 * trans.m31 + trans.m41;
	    			vy = (startX+mnx) * trans.m12 + (startY+mny) * trans.m22 + 0 * trans.m32 + trans.m42;
	    

	                cg2d_draw_image(c2d,&rect,vx,vy);
	                 
	            }       
	       }
	}	
}





void cg2d_clear_layer(cg2d_t *c2d,int layerID){
	arrsetlen(c2d->buffers[layerID].verts,0);
}

int cg2d_get_virtual_width(cg2d_t *c2d){
    return c2d->virtW;
}

int cg2d_get_virtual_height(cg2d_t *c2d){
    return c2d->virtH;
}

void cg2d_set_virtual_resolution(cg2d_t *c2d,int w, int h){
    c2d->virtW=w;
    c2d->virtH=h;
    int vx,vy,vw,vh;
    cg2d_get_viewport(c2d,&vx,&vy,&vw,&vh);
    c2d->virtMXscale=((float)w)/WINDOW_WIDTH;
	c2d->virtMYscale=((float)h)/WINDOW_HEIGHT;
	c2d->virtMXscale=(float)w/(float)WINDOW_WIDTH;
    c2d->virtMYscale=(float)h/(float)WINDOW_HEIGHT;
}



void cg2d_draw_layer(cg2d_t *c2d,int layerID,SDL_GPUCommandBuffer* cmdBuf, SDL_GPUTexture *renderTarget){
	int bufitr=layerID;
            
    if(arrlenu(c2d->buffers[bufitr].verts)>0){
        
        cg2d_vertex *verts=SDL_MapGPUTransferBuffer(c2d->device,c2d->buffers[bufitr].tbuffer,true);
       
        memcpy(verts,c2d->buffers[bufitr].verts,sizeof(cg2d_vertex)*arrlenu(c2d->buffers[bufitr].verts));
    
        SDL_UnmapGPUTransferBuffer(c2d->device,c2d->buffers[bufitr].tbuffer );                                             


         // Upload instance data
        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuf);
        SDL_UploadToGPUBuffer(
            copyPass,
            &(SDL_GPUTransferBufferLocation) {
                .transfer_buffer = c2d->buffers[bufitr].tbuffer,
                .offset = 0
            },
            &(SDL_GPUBufferRegion) {
                .buffer = c2d->buffers[bufitr].vbuffer,
                .offset = 0,
                .size = arrlenu(c2d->buffers[bufitr].verts) * sizeof(cg2d_vertex)
            },
            true
        );
        SDL_EndGPUCopyPass(copyPass);

        // Render sprites
        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(
            cmdBuf,
            &(SDL_GPUColorTargetInfo){
                .texture = renderTarget,
                .cycle = false,
                .load_op = c2d->buffers[bufitr].load_op,
                .store_op = SDL_GPU_STOREOP_STORE,
                .clear_color = { (float)c2d->clsr/255.0f, (float)c2d->clsg/255.0f, (float)c2d->clsb/255.0f, 1.0f }
            },
            1,
            NULL
        );

        int vx,vy,vw,vh;
        cg2d_get_viewport(c2d,&vx,&vy,&vw,&vh);
    
    	//remember to scale window by winW and winH to account for high dpi displays
        SDL_GPUViewport vp=(SDL_GPUViewport){vx,vy,vw*c2d->winW,vh*c2d->winH,0,1};
        SDL_SetGPUViewport(renderPass,&vp);
        
        _cg2d_mat4 mat=_cg2d_mat4_CreateOrthographic(vx,c2d->virtW*c2d->winW, c2d->virtH*c2d->winH,vy, 0,-1);
        mat=_cg2d_mat4_Multiply(c2d->buffers[bufitr].transform,mat);

        cg2d_vertex_uniform uniform={
		    mat,
		    vw,
		    vh,
		    0.0,
		    0.0
		};

		c2d->fragUniform.scrnW=vw;
        c2d->fragUniform.scrnH=vh;
        c2d->fragUniform.ticks=(float)SDL_GetTicks();
        c2d->fragUniform.posX=0.0;
        c2d->fragUniform.posY=0.0;

		


        SDL_BindGPUGraphicsPipeline(renderPass, c2d->buffers[bufitr].RenderPipeline);
        
        SDL_BindGPUVertexStorageBuffers(
            renderPass,
            0,
            &c2d->buffers[bufitr].vbuffer,
            1
        );



        SDL_BindGPUFragmentSamplers(
            renderPass,
            0,
            &(SDL_GPUTextureSamplerBinding){
                .texture =c2d->buffers[bufitr].tex, 
                .sampler = c2d->buffers[bufitr].samp
            },
            1
        );
        
        SDL_PushGPUVertexUniformData(
            cmdBuf,
            0,
            &uniform,
            sizeof(cg2d_vertex_uniform)
        );

        

        SDL_PushGPUFragmentUniformData(
    		cmdBuf,
    		0,
    		&c2d->fragUniform,
    		sizeof(cg2d_fragment_uniform)
    	);
        

        SDL_DrawGPUPrimitives(
            renderPass,
            arrlenu(c2d->buffers[bufitr].verts) * 6,
            1,
            0,
            0
        );

        SDL_EndGPURenderPass(renderPass);
    }//end buffer length check
}