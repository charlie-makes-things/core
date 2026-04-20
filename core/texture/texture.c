
typedef struct cg_texture{
	bool loaded;
	SDL_GPUTexture *tex;
	SDL_GPUSampler *samp;
	int w,h;
}cg_texture;


void cg_texture_gen_2d(cg_texture *tex,int w, int h,SDL_GPUFilter filter,SDL_GPUDevice *device,SDL_Window *window){
	float wscale=SDL_GetWindowPixelDensity(window);
	tex->tex = SDL_CreateGPUTexture(device, &(SDL_GPUTextureCreateInfo){
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
		.width = w*wscale,
		.height = h*wscale,
		.layer_count_or_depth = 1,
		.num_levels = 1,
		.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET
	});

	tex->samp = SDL_CreateGPUSampler(device, &(SDL_GPUSamplerCreateInfo){
		.min_filter = filter,
		.mag_filter = filter,
		.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
		.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
	});
	tex->w=w;
	tex->h=h;
	tex->loaded=true;
}

void cg_texture_copy_texture(SDL_GPUCommandBuffer *buf,SDL_Window *win,cg_texture *from, cg_texture *to){
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(buf);
    SDL_CopyGPUTextureToTexture(
        copyPass,
        &(SDL_GPUTextureLocation){
            .texture = from->tex
        },
        &(SDL_GPUTextureLocation){
            .texture = to->tex
        },
        WINDOW_WIDTH*(int)SDL_GetWindowPixelDensity(win),
        WINDOW_HEIGHT*(int)SDL_GetWindowPixelDensity(win),
        1,
        false
    );
    SDL_EndGPUCopyPass(copyPass);
}