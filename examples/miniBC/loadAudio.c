static MIX_Track *menuMusic=NULL;
static void *menuMusicData=NULL;


SDL_AppResult load_audio(MIX_Mixer *mix){
	 
	MIX_Audio *menuMusicAudio=NULL;

    char *sfxPath="audio/Circle.mp3";
    size_t sz1=0;
    menuMusicData=load_file_storage(sfxPath,&sz1);
    if(menuMusicData==NULL){SDL_Log("failed to load %s\n",sfxPath );return SDL_APP_FAILURE;}

    menuMusicAudio = MIX_LoadAudio_IO(mix, SDL_IOFromMem(menuMusicData,sz1), false,true);
    if (!menuMusicAudio) {
        SDL_Log("Couldn't load %s: %s", sfxPath, SDL_GetError());
        return SDL_APP_FAILURE;
    }

    menuMusic = MIX_CreateTrack(mix);
    if (!menuMusic) {
        SDL_Log("Couldn't create a mixer track: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if(!MIX_SetTrackAudio(menuMusic,menuMusicAudio)){
    	SDL_Log("couldn't set track audio\n");
    }


    return SDL_APP_CONTINUE;
    
}