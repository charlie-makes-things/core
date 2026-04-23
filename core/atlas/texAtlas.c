//texture atlas loading.

//a fairly basic texture atlas lib. 
//it requires:
//			 SDL3
//			 json.h from https://github.com/sheredom/json.h
//			 
//it also expects atlases to be made using the Cram texture packing tool
//which is available from: https://github.com/MoonsideGames/Cram/

//usage
//after opening a window and gpu device use atlas init and point it at a
//json atlas file as produced by cram, the url to the directory containing 
//the image to load in png format (lowercase file extension - so file.png not
//file.PNG) and the SDL_GPUDevice. e.g.
//
//atlas_init(&myAtlas,"path/to/json/file.json","path/to/image/",device);
//
//this will parse the json file and populate the images array in the atlas
//struct with texAtlasImages and also load and prepare the texture and sampler
//needed to pass to a shader. you can loop through the loaded image data with
//texAtlas.count, i.e
//for(int i=0;i<myAtlas.count;i++){
//	texAtlasImage *img=(texAtlasImage*)myAtlas.images[i];
//  ...do stuff
//}
//
//when you're done, you can free the atlas using atlas_free(), which
//deletes the images array as well as the texture and sampler.
//
//also note that if you pass NULL in to device and image path parameters
//the atlas will not load the images or create texture and sampler, which
//might be useful if you just want to load the .json data for use in a
//non-SDL_GPU setup. it should be fairly trivial to change/replage the
//atlas_load_image and atlas_load_texture functions to something else.
//
//see the test file atlas.c for a working example.
//*******************************************************************
//enums
//*******************************************************************
typedef enum atlas_tex_filter{
	ATLAS_FILTER_NEAREST =1,
	ATLAS_FILTER_LINEAR =2,
}atlas_tex_filter;

//*******************************************************************
//structs
//*******************************************************************
typedef struct texAtlasImage{
	char name[256];
	int x,y,w,h;
	int trimOffsetX,trimOffsetY;
	int untrimmedWidth,untrimmedHeight;
	float u,v,uw,vh;
}texAtlasImage;

typedef struct texAtlas{
	texAtlasImage *images;
	cg_texture tex;
	char name[256];
	char imagePath[256];
	int w,h;
	size_t count;
	bool initialised;
}texAtlas;

//*******************************************************************
//functions
//*******************************************************************

//note that function prefixed with an underscore aren't really meant for 
//use outside of this file, though they can be replaced with non-SDL_GPU
//alternatives if needed.

//initialise the atlas.
//this function sets everything up, loads the .json and image data and sets
//up the texture and sampler ready for use. Note the texture is upload to the GPU
//here so you can simply pass myAtlas.tex to a shader.
//once loaded the images array will contain data about the images in the atlas,
//including its name, uv coords, trim data etc. See the docs for Cram to see what
//these do.
//the uv coords are calculated here for ease. 
//RETURNS: returns 0 if everything is ok otherwise check log for where things went wrong
int atlas_init(texAtlas *tex, char *fullPath, char *imagePath, SDL_GPUDevice *device, atlas_tex_filter filter);

//loads the image from file into an sdl_surface struct. normally you wouldn't
//use this function as atlas_init does all this for you. if you do use this
//separately, make sure to call SDL_DestroySurface(); on the SDL_Surface this
//function returns
SDL_Surface* _atlas_load_image(const char *imagePath,const char* imageFilename);

//loads the texture and sample onto the gpu. once this is done you can then
//access the texture with myAtlas.tex. you probably won't need to call this
//function directly.
int _atlas_load_texture(texAtlas *atlas,SDL_GPUDevice *device,atlas_tex_filter filter);

//prints the name of the atlas image file,its width and height and the number
//of images in the image array.
void atlas_print_info(texAtlas *atlas);

//prints info about each image in the image array.
void atlas_print_image_info(texAtlas *atlas);

//prints the names of all the images in the atlas
void atlas_print_image_names(texAtlas *atlas);

//frees data created by the atlas at init. this is the image array and the GPU
//texture and sample.
void atlas_free(texAtlas *atlas,SDL_GPUDevice *device);




//*******************************************************************
//function code
//*******************************************************************

SDL_Surface* _atlas_load_image(const char *imagePath,const char* imageFilename)
{
	char fullPath[256];
	SDL_Surface *result;

	SDL_snprintf(fullPath, sizeof(fullPath), "%s%s.png",imagePath, imageFilename);
	printf("fullpath is %s\n",fullPath );
	result = load_PNG_storage(fullPath);
	return result;
}


//*******************************************************************
//*******************************************************************
//*******************************************************************

int _atlas_load_texture(texAtlas *atlas, SDL_GPUDevice *device,atlas_tex_filter filter){

	 // Load the image data
    SDL_Surface *imageData = _atlas_load_image(atlas->imagePath,atlas->name);
    if (imageData == NULL)
    {
        SDL_Log("Could not load atlas image data on path %s.png",atlas->name);
        return -1;
    }

    SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(
        device,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = imageData->w * imageData->h * 4
        }
    );

    Uint8 *textureTransferPtr = SDL_MapGPUTransferBuffer(
        device,
        textureTransferBuffer,
        false
    );
    SDL_memcpy(textureTransferPtr, imageData->pixels, imageData->w * imageData->h * 4);
    SDL_UnmapGPUTransferBuffer(device, textureTransferBuffer);

    // Create the GPU resources
    atlas->tex.tex = SDL_CreateGPUTexture(
        device,
        &(SDL_GPUTextureCreateInfo){
            .type = SDL_GPU_TEXTURETYPE_2D,
            .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
            .width = imageData->w,
            .height = imageData->h,
            .layer_count_or_depth = 1,
            .num_levels = 1,
            .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER
        }
    );

    SDL_GPUFilter fil;
    SDL_GPUSamplerMipmapMode mip;
    if(filter==ATLAS_FILTER_NEAREST){
    	fil=SDL_GPU_FILTER_NEAREST;
    	mip=SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    }else{
    	fil=SDL_GPU_FILTER_LINEAR;
    	mip=SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;

    }

    atlas->tex.samp= SDL_CreateGPUSampler(
        device,
        &(SDL_GPUSamplerCreateInfo){
            .min_filter = fil,
            .mag_filter = fil,
            .mipmap_mode = mip,
            .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
            .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
            .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT
        }
    );
    // Transfer the up-front data
    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

    SDL_UploadToGPUTexture(
        copyPass,
        &(SDL_GPUTextureTransferInfo) {
            .transfer_buffer = textureTransferBuffer,
            .offset = 0, /* Zeroes out the rest */
        },
        &(SDL_GPUTextureRegion){
            .texture = atlas->tex.tex,
            .w = imageData->w,
            .h = imageData->h,
            .d = 1
        },
        false
    );

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(uploadCmdBuf);

    SDL_DestroySurface(imageData);
    SDL_ReleaseGPUTransferBuffer(device, textureTransferBuffer);
    return 0;
}

//*******************************************************************
//*******************************************************************
//*******************************************************************

int atlas_init(texAtlas *tex, char *fullPath, char *imagePath,  SDL_GPUDevice *device,atlas_tex_filter filter){
	
	if(tex->initialised==true){
		SDL_Log("can't initialise atlas twice! create a new one to load more stuff\n");
		return 1;
	}

	size_t fileSize;
	char *jsonData=(char*)load_file_storage(fullPath,&fileSize);

	SDL_strlcpy(tex->imagePath,imagePath,sizeof(imagePath));
	

	SDL_Log("%s\n",SDL_GetError() );
	if(jsonData==NULL){
		SDL_Log("failed to load file %s\n",fullPath);
		return 1;
	}

	struct json_value_s* root = json_parse(jsonData, fileSize);

	struct json_object_s *obj=(struct json_object_s*)root->payload;
	struct json_object_element_s *a=obj->start;

	tex->count=0;

	while(a!=NULL){

		struct json_value_s * a_val = a->value;

		char *name=(char*)a->name->string;
		char val[256];
		
		switch(a_val->type){
			case json_type_string:
				struct json_string_s *s=(struct json_string_s*)a_val->payload;
				SDL_strlcpy(val,(char*)s->string,255);
				break;
			case json_type_number:
				struct json_number_s *n=(struct json_number_s*)a_val->payload;
				SDL_strlcpy(val,n->number,255);
				break;
			case json_type_object:

				break;

			case json_type_array:
				struct json_array_s *arr=(struct json_array_s*)a_val->payload;
				tex->count=arr->length;
				tex->images=SDL_malloc(sizeof(texAtlasImage)*tex->count);
				struct json_array_element_s *el=(struct json_array_element_s*)arr->start;
				int index=0;
				while(el!=NULL){
					
					struct json_value_s *arrval=(struct json_value_s*)el->value;

					if(arrval->type==json_type_object){
						
						struct json_object_s *arrobj=(struct json_object_s*)arrval->payload;
						struct json_object_element_s *arrobjel=(struct json_object_element_s*)arrobj->start;
						
						while(arrobjel!=NULL){
							
							struct json_string_s *isit=(struct json_string_s*)arrobjel->name;
							struct json_value_s *arrval=(struct json_value_s*)arrobjel->value;

							if(SDL_strcmp((char*)isit->string,"Name")==0){
								struct json_string_s *elname=(struct json_string_s*)arrval->payload;
								SDL_strlcpy(tex->images[index].name,(char*)elname->string,255);
							}else if(SDL_strcmp((char*)isit->string,"X")==0){
								struct json_number_s *elval=(struct json_number_s*)arrval->payload;
								tex->images[index].x=SDL_atoi((char*)elval->number);
							}else if(SDL_strcmp((char*)isit->string,"Y")==0){
								struct json_number_s *elval=(struct json_number_s*)arrval->payload;
								tex->images[index].y=SDL_atoi((char*)elval->number);
							}else if(SDL_strcmp((char*)isit->string,"W")==0){
								struct json_number_s *elval=(struct json_number_s*)arrval->payload;
								tex->images[index].w=SDL_atoi((char*)elval->number);
							}else if(SDL_strcmp((char*)isit->string,"H")==0){
								struct json_number_s *elval=(struct json_number_s*)arrval->payload;
								tex->images[index].h=SDL_atoi((char*)elval->number);
							}else if(SDL_strcmp((char*)isit->string,"TrimOffsetX")==0){
								struct json_number_s *elval=(struct json_number_s*)arrval->payload;
								tex->images[index].trimOffsetX=SDL_atoi((char*)elval->number);
							}else if(SDL_strcmp((char*)isit->string,"TrimOffsetY")==0){
								struct json_number_s *elval=(struct json_number_s*)arrval->payload;
								tex->images[index].trimOffsetY=SDL_atoi((char*)elval->number);
							}else if(SDL_strcmp((char*)isit->string,"UntrimmedWidth")==0){
								struct json_number_s *elval=(struct json_number_s*)arrval->payload;
								tex->images[index].untrimmedWidth=SDL_atoi((char*)elval->number);
							}else if(SDL_strcmp((char*)isit->string,"UntrimmedHeight")==0){
								struct json_number_s *elval=(struct json_number_s*)arrval->payload;
								tex->images[index].untrimmedHeight=SDL_atoi((char*)elval->number);
							}

							arrobjel=arrobjel->next;
						}
					}

					index++;
					el=el->next;
				}

				break;
			case json_type_true:
				SDL_strlcpy(val,"true",5);
				break;
			case json_type_false:
				SDL_strlcpy(val,"false",6);
				break;
			case json_type_null:
				SDL_strlcpy(val,"null",5);
				break;
			default:
				break;
		}
		//printf("object name is %s value is %s\n",name,val );
		if(SDL_strcmp(name,"Name")==0){
			SDL_strlcpy(tex->name,val,SDL_strlen(val)+1);
		}
		if(SDL_strcmp(name,"Width")==0){
			tex->w=SDL_atoi(val);
		}
		if(SDL_strcmp(name,"Height")==0){
			tex->h=SDL_atoi(val);
		}
		


		a=a->next;
	}

	//calc uv coords
	for(int i=0;i<tex->count;i++){
		tex->images[i].u=(float)tex->images[i].x/(float)tex->w;
		tex->images[i].v=(float)tex->images[i].y/(float)tex->h;
		tex->images[i].uw=(float)tex->images[i].w/(float)tex->w;
		tex->images[i].vh=(float)tex->images[i].h/(float)tex->h;
	}

	tex->tex.loaded=false;

	if(device!=NULL){
		SDL_Log("loading texture %s\n", tex->name);
		int res=_atlas_load_texture(tex,device,filter);
		if(res==-1){
			SDL_Log("something went wrong loading the teture for %s. returning early\n",tex->name);
			free(root);
			free(jsonData);
			free(tex->images);
			return 1;
		}
	}else{
		SDL_Log("device is null, not loading texture\n");
	}

	tex->tex.loaded=true;
	tex->tex.w=tex->w;
	tex->tex.h=tex->h;

	free(root);
	free(jsonData);
	tex->initialised=true;
	return 0;
} 

//*******************************************************************
//*******************************************************************
//*******************************************************************

void atlas_print_info(texAtlas *atlas){
	SDL_Log("atlas texture name is %s - w=%d, h=%d, num images=%d\n",atlas->name,atlas->w,atlas->h,(int)atlas->count );

}

//*******************************************************************
//*******************************************************************
//*******************************************************************

void atlas_print_image_info(texAtlas *atlas){
	
	for(int i=0;i<atlas->count;i++){



		SDL_Log("atlas image %d;  name %s\n\t\t x %d - u %f\n\t\t y %d - v %f\n\t\t w %d - uw %f\n\t\t h %d - uh %f\n\t\t trimOffsetX %d\n\t\t trimOffsetY %d\n\t\t untrimmedWidth %d\n\t\t untrimmedHeight %d\n\t\t\n",
								i,
								atlas->images[i].name,
								atlas->images[i].x, atlas->images[i].u,
								atlas->images[i].y, atlas->images[i].v,
								atlas->images[i].w, atlas->images[i].uw,
								atlas->images[i].h, atlas->images[i].vh,
								atlas->images[i].trimOffsetX,
								atlas->images[i].trimOffsetY,
								atlas->images[i].untrimmedWidth,
								atlas->images[i].untrimmedHeight);

	}
}

//*******************************************************************
//*******************************************************************
//*******************************************************************

void atlas_print_image_names(texAtlas *atlas){
	
	for(int i=0;i<atlas->count;i++){
		SDL_Log("image name %s\n",atlas->images[i].name);
	}
}



//*******************************************************************
//*******************************************************************
//*******************************************************************

void atlas_free(texAtlas *atlas,SDL_GPUDevice *device){
	free(atlas->images);
	if(device!=NULL){
		SDL_ReleaseGPUSampler(device, atlas->tex.samp);
	    SDL_ReleaseGPUTexture(device, atlas->tex.tex);
	}
}