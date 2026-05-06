



SDL_AppResult load_audio(MIX_Mixer *mix,char *path,cg_audio_track *a,cg_audio_type type){
	 
	
    a->audio=NULL;
    a->track=NULL;
    void *data=NULL;
    a->type=type;
    a->vol=1.0;
    
    size_t sz1=0;
    data=load_file_storage(path,&sz1);
    if(data==NULL){SDL_Log("failed to load %s\n",path );return SDL_APP_FAILURE;}

    a->audio = MIX_LoadAudio_IO(mix, SDL_IOFromMem(data,sz1), false,true);
    if (!a->audio) {
        SDL_Log("Couldn't load %s: %s", path, SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_free(data);

    a->track = MIX_CreateTrack(mix);
    if (!a->track) {
        SDL_Log("Couldn't create a mixer track:%s, %s", path,SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if(!MIX_SetTrackAudio(a->track,a->audio)){
    	SDL_Log("couldn't set track audio %s\n",path);
    }

    arrpush(_audioList,*a);

    return SDL_APP_CONTINUE;
    
}

bool play_audio(cg_audio_track *a,float vol,SDL_PropertiesID props){
    if(a->track!=NULL){
        a->vol=vol;
        if(a->type==AUDIO_TYPE_MUSIC){
             
             if(!MIX_PlayTrack(a->track,props)){
                SDL_Log("Couldn't play music\n");
                return false;
             }
             MIX_SetTrackGain(a->track, a->vol*musicGain);
             _currentMusic=a;
        }else{
             if(!MIX_PlayTrack(a->track, props)){
                SDL_Log("Couldn't play SFX\n");
                return false;
             }
             MIX_SetTrackGain(a->track, a->vol* sfxGain);
        }
        return true;
    }
    return false;
}

void audio_update_volumes(){
    for(int i=0;i<arrlen(_audioList);i++){
        cg_audio_track *a=&_audioList[i];
        if(a->type==AUDIO_TYPE_MUSIC){
            MIX_SetTrackGain(a->track, a->vol* musicGain);
            //SDL_Log("found music - changing to %f x %f = %f\n",a->vol,musicGain,a->vol*musicGain);
        }else{
            MIX_SetTrackGain(a->track, a->vol* sfxGain);
        }
    }
}

void audio_fade_out_and_stop(cg_audio_track *a, Uint64 millisecs){
   if(_currentMusic!=NULL){
        MIX_StopTrack(a->track,MIX_TrackMSToFrames(a->track, millisecs));    
   }
}

void audio_fade_in_and_play(cg_audio_track *a, Uint64 millisecs){
   //start playing the title music
    SDL_PropertiesID options = SDL_CreateProperties();    
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_FADE_IN_START_GAIN_FLOAT, 0.0);  /* start the first loop 1 second into the audio. */
    SDL_SetNumberProperty(options,MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER,millisecs);
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);//-1 loop forever
    if(!play_audio(a,1.0, options)){
        SDL_Log("couldn't play track\n");
    }
    SDL_DestroyProperties(options);
}

void audio_free(){
    for(int i=0;i<arrlen(_audioList);i++){
        cg_audio_track *a=&_audioList[i];
        MIX_DestroyTrack(a->track);
        MIX_DestroyAudio(a->audio);
    }

    arrfree(_audioList);
}

