//wrapper around SDL_Storage for loading and saving data.

static SDL_EnumerationResult SDLCALL enum_storage_callback(void *userdata, const char *origdir, const char *fname)
{
    SDL_Storage *storage = (SDL_Storage *) userdata;
    SDL_PathInfo info;
    char *fullpath = NULL;

    if (SDL_asprintf(&fullpath, "%s%s", origdir, fname) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Out of memory!");
        return SDL_ENUM_FAILURE;
    }

    if (!SDL_GetStoragePathInfo(storage, fullpath, &info)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't stat '%s': %s", fullpath, SDL_GetError());
    } else {
        const char *type;
        if (info.type == SDL_PATHTYPE_FILE) {
            type = "FILE";
        } else if (info.type == SDL_PATHTYPE_DIRECTORY) {
            type = "DIRECTORY";
        } else {
            type = "OTHER";
        }
        SDL_Log("STORAGE %s (type=%s, size=%" SDL_PRIu64 ", create=%" SDL_PRIu64 ", mod=%" SDL_PRIu64 ", access=%" SDL_PRIu64 ")",
                fullpath, type, info.size, info.modify_time, info.create_time, info.access_time);

        if (info.type == SDL_PATHTYPE_DIRECTORY) {
            if (!SDL_EnumerateStorageDirectory(storage, fullpath, enum_storage_callback, userdata)) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Enumeration failed!");
            }
        }
    }

    SDL_free(fullpath);
    return SDL_ENUM_CONTINUE;  /* keep going */
}

//startDir could be "/" for the project root directory or "images/"
void print_filesys_files(char *startDir){
	 SDL_Storage *title = SDL_OpenTitleStorage(NULL, 0);
    if (title == NULL) {
        SDL_Log("couldn't open title storage - returning null");
    }
    while (!SDL_StorageReady(title)) {
        SDL_Delay(1);
    }



    if (!SDL_EnumerateStorageDirectory(title, startDir, enum_storage_callback, title)) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Storage Base path enumeration failed!");
    }

    SDL_CloseStorage(title);
}

void *load_file_storage(char *url,size_t *len){

	SDL_Storage *title = SDL_OpenTitleStorage(NULL, 0);
    if (title == NULL) {
        // Something bad happened!
        SDL_Log("couldn't open title storage - returning null");
        return NULL;
    }
    while (!SDL_StorageReady(title)) {
        SDL_Delay(1);
    }

    void *dst;
    Uint64 dstLen = 0;

    if (SDL_GetStorageFileSize(title, url, &dstLen) && dstLen > 0) {
       
         dst = SDL_malloc(dstLen);
        if (!SDL_ReadStorageFile(title, url, dst, dstLen)) {
            // Something bad happened!
            printf("storage file %s read failed\n",url);
            return NULL;
        }
        
     } else {
    
         SDL_Log("couldn't load storage file %s\n",url);
         return NULL;
    
    }

    SDL_CloseStorage(title);

    *len=dstLen;
    return dst;

}


bool write_user_file_storage(char* filepath,char* org,char*app,SDL_PropertiesID props,char *data,size_t dataLen)
{



    SDL_Storage *user = SDL_OpenUserStorage(org, app, props);
    if (user == NULL) {
        // Something bad happened!
        SDL_Log("cant open storage at %s %s\n",org,app);
        return false;
    }
    while (!SDL_StorageReady(user)) {
        SDL_Delay(1);
    }

    size_t remain= SDL_GetStorageSpaceRemaining(user);
    if(remain<dataLen){
        SDL_Log("insufficient storage remaining to save file. size %ld, remain %ld\n",dataLen,remain);
        return false;
    }

   
    if (!SDL_WriteStorageFile(user, filepath, data, dataLen)) {
        // Something bad happened!
        SDL_Log("unable to write save file\n");
        return false;
    }

    SDL_CloseStorage(user);
    return true;
}

bool create_user_storage_folder(char *path,char* org,char*app,SDL_PropertiesID props){
    
     SDL_Storage *user = SDL_OpenUserStorage(org, app, props);
    if (user == NULL) {
        // Something bad happened!
        SDL_Log("cant open storage at %s %s\n",org,app);
        return false;
    }
    while (!SDL_StorageReady(user)) {
        SDL_Delay(1);
    }

    if(!SDL_CreateStorageDirectory(user, path)){
        SDL_Log("couldn't create storage folder\n");
        return false;
    }
    SDL_CloseStorage(user);
    return true;
}


void* read_user_storage_file(char *path,char* org,char*app,SDL_PropertiesID props,size_t *outsz)
{
    SDL_Storage *user = SDL_OpenUserStorage(org,app,props);
    if (user == NULL) {
        // Something bad happened!
         SDL_Log("cant open storage at %s %s\n",org,app);
        return NULL;
    }
    while (!SDL_StorageReady(user)) {
        SDL_Delay(1);
    }

    
    void* dst;
    Uint64 len = 0;
    if (SDL_GetStorageFileSize(user, path, &len) && len > 0) {
        dst = SDL_malloc(len);
        if (!SDL_ReadStorageFile(user,path, dst, len)) {
            // Something bad happened!
            SDL_Log("couldn't read data from %s\n",path);
            return NULL;
        }
        
    } else {
        // Something bad happened!
        SDL_Log("couldn't get filesize for %s\n",path);
        return NULL;
    }

    SDL_CloseStorage(user);
    *outsz=len;
    
    return dst;
}

char **read_user_storage_directory(char *path,char* org,char*app,SDL_PropertiesID props,char *pattern,size_t *outsz){
    //char ** SDL_GlobStorageDirectory(SDL_Storage *storage, const char *path, const char *pattern, SDL_GlobFlags flags, int *count);

    SDL_Storage *user = SDL_OpenUserStorage(org,app,props);
    if (user == NULL) {
        // Something bad happened!
         SDL_Log("cant open storage at %s %s\n",org,app);
        return NULL;
    }
    while (!SDL_StorageReady(user)) {
        SDL_Delay(1);
    }

    char** dst;
    int len = 0;
    dst=SDL_GlobStorageDirectory(user, path, pattern, 0, &len);

    if(dst==NULL){
        SDL_Log("directory array is null\n");
        return NULL;
    }
    if(len<=0){
        SDL_Log("no matching files in directory %s\n",path);
    }

    SDL_CloseStorage(user);
     *outsz=len;
    
    return dst;

}

