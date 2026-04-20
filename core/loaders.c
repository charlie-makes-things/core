
//collection of useful loaders based around the SDL_Storage functions

SDL_Surface *load_BMP_storage(char *url){

	void *img;
	size_t imgLen = 0;

	img=load_file_storage(url,&imgLen);

    if(img==NULL){
      printf("storage BMP is null\n");
      return NULL;  
    }
    
    int desiredChannels=4;
    SDL_Surface *result=SDL_LoadBMP_IO(SDL_IOFromMem(img, imgLen),true);
    if (result == NULL)
    {
        SDL_Log("Failed to load BMP: %s", SDL_GetError());
        return NULL;
    }

    SDL_PixelFormat format;
    if (desiredChannels == 4)
    {
        format = SDL_PIXELFORMAT_ABGR8888;
    }
    else
    {
        SDL_assert(!"Unexpected desiredChannels");
        SDL_DestroySurface(result);
        return NULL;
    }
    if (result->format != format)
    {
        SDL_Surface *next = SDL_ConvertSurface(result, format);
        SDL_DestroySurface(result);
        result = next;
    }
   
    SDL_free(img);

   

    return(result);

}

SDL_Surface *load_PNG_storage(char *url){

	void *img;
	size_t imgLen = 0;

	img=load_file_storage(url,&imgLen);

    if(img==NULL){
      printf("storage PNG is null\n");
      return NULL;  
    }
    
    int desiredChannels=4;
    SDL_Surface *result=SDL_LoadPNG_IO(SDL_IOFromMem(img, imgLen),true);
    if (result == NULL)
    {
        SDL_Log("Failed to load PNG: %s", SDL_GetError());
        return NULL;
    }

    SDL_PixelFormat format;
    if (desiredChannels == 4)
    {
        format = SDL_PIXELFORMAT_ABGR8888;
    }
    else
    {
        SDL_assert(!"Unexpected desiredChannels");
        SDL_DestroySurface(result);
        return NULL;
    }
    if (result->format != format)
    {
        SDL_Surface *next = SDL_ConvertSurface(result, format);
        SDL_DestroySurface(result);
        result = next;
    }
   
    SDL_free(img);

    

    return(result);

}

int load_texture_storage_BMP(cg_texture *tex,char *imgPath, SDL_GPUDevice *device,SDL_GPUFilter filter){

     // Load the image data
    
    SDL_Surface *imageData = load_BMP_storage(imgPath);
    if (imageData == NULL)
    {
        SDL_Log("Could not load atlas image data on path %s",imgPath);
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
    tex->tex = SDL_CreateGPUTexture(
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

    
    SDL_GPUSamplerMipmapMode mip;
    if(filter==SDL_GPU_FILTER_NEAREST){
        mip=SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    }else{
        mip=SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;

    }

    tex->samp = SDL_CreateGPUSampler(
        device,
        &(SDL_GPUSamplerCreateInfo){
            .min_filter = filter,
            .mag_filter = filter,
            .mipmap_mode = mip,
            .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
            .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
            .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
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
            .texture = tex->tex,
            .w = imageData->w,
            .h = imageData->h,
            .d = 1
        },
        false
    );

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(uploadCmdBuf);

    tex->w=imageData->w;
    tex->h=imageData->h;

    SDL_DestroySurface(imageData);
    SDL_ReleaseGPUTransferBuffer(device, textureTransferBuffer);

    tex->loaded=true;
    return 0;
}

SDL_GPUShader* load_shader_storage(
    SDL_GPUDevice* device,
    const char* shaderFilename,
    Uint32 samplerCount,
    Uint32 uniformBufferCount,
    Uint32 storageBufferCount,
    Uint32 storageTextureCount
) {
    // Auto-detect the shader stage from the file name for convenience
    SDL_GPUShaderStage stage;
    if (SDL_strstr(shaderFilename, ".vert"))
    {
        stage = SDL_GPU_SHADERSTAGE_VERTEX;
    }
    else if (SDL_strstr(shaderFilename, ".frag"))
    {
        stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    }
    else
    {
        SDL_Log("Invalid shader stage!");
        return NULL;
    }

    char fullPath[256];
    SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
    const char *entrypoint;

    if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV) {
        SDL_snprintf(fullPath, sizeof(fullPath), "%s.spv", shaderFilename);
        printf("%s\n",fullPath);
        format = SDL_GPU_SHADERFORMAT_SPIRV;
        entrypoint = "main";
    } else if (backendFormats & SDL_GPU_SHADERFORMAT_MSL) {
        SDL_snprintf(fullPath, sizeof(fullPath), "%s.msl", shaderFilename);
        format = SDL_GPU_SHADERFORMAT_MSL;
        entrypoint = "main0";
    } else if (backendFormats & SDL_GPU_SHADERFORMAT_DXIL) {
        SDL_snprintf(fullPath, sizeof(fullPath), "%s.dxil", shaderFilename);
        format = SDL_GPU_SHADERFORMAT_DXIL;
        entrypoint = "main";
    } else {
        SDL_Log("%s", "Unrecognized backend shader format!");
        return NULL;
    }

    size_t codeSize;
    void* code = load_file_storage(fullPath, &codeSize);
    if (code == NULL)
    {
        SDL_Log("Failed to load shader from disk! %s", fullPath);
        return NULL;
    }

    SDL_GPUShaderCreateInfo shaderInfo = {
        .code = code,
        .code_size = codeSize,
        .entrypoint = entrypoint,
        .format = format,
        .stage = stage,
        .num_samplers = samplerCount,
        .num_uniform_buffers = uniformBufferCount,
        .num_storage_buffers = storageBufferCount,
        .num_storage_textures = storageTextureCount
    };
    SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
    if (shader == NULL)
    {
        SDL_Log("Failed to create shader!");
        SDL_free(code);
        return NULL;
    }

    SDL_free(code);
    return shader;
}