
bool video_initialised=false;
SDL_DisplayMode **video_mode_list=NULL;
int video_mode_count=0;
const SDL_DisplayMode *video_largest_window=NULL;
const SDL_DisplayMode *video_desktop_mode=NULL;
const SDL_DisplayMode *video_current_mode=NULL;
float video_display_scale=1.0;
bool video_fullscreen_is_exclusive=false;

bool video_compare_modes(const SDL_DisplayMode *a,const SDL_DisplayMode *b){

	if(a->displayID == b->displayID &&
			a->format == b->format &&
			a->w == b->w &&
			a->h == b->h &&
			a->pixel_density == b->pixel_density &&
			a->refresh_rate == b->refresh_rate &&
			a->refresh_rate_numerator == b->refresh_rate_numerator &&
			a->refresh_rate_denominator == b->refresh_rate_denominator){
			return true;
			}
	return false;
}

bool video_is_fullscreen(SDL_Window *window){
	if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN)
	{
		if (SDL_GetWindowFullscreenMode(window) != NULL)
		{
			return true;
		}
		else
		{
			return true;
		}
	}
	
	return false;
}

SDL_DisplayMode *video_find_mode(int w, int h){
	SDL_DisplayMode *closest=NULL;
	if(video_initialised==true){
		for(int i=0;i<arrlen(video_mode_list);i++){
			//check if the dimensions are the same and return the displaymode if true
			if(w==video_mode_list[i]->w && h==video_mode_list[i]->h){
				return video_mode_list[i];
			}
		}
		//if no match check area and store which is closest to the w+h
		
		int area=w*h;
		int dst=INT_MAX;
		for(int i=0;i<arrlen(video_mode_list);i++){
			int a=video_mode_list[i]->w*video_mode_list[i]->h;
			int d=(SDL_abs(a-area));
			if(d<=dst){
				dst=d;
				closest=video_mode_list[i];
			}
		}
	}
	return closest;
}

SDL_AppResult video_init(){

	int dc=0;
	SDL_DisplayID *video_displays=SDL_GetDisplays(&dc);
	if(dc==0){return SDL_APP_FAILURE;}

	for(int i=0;i<dc;i++){

		 int cnt=0;
		 SDL_DisplayMode **list=SDL_GetFullscreenDisplayModes(video_displays[i], &cnt);
		 for(int o=0;o<cnt;o++){
		 	arrpush(video_mode_list,list[o]);
		 }
		 //SDL_free(list);
	}
	//SDL_free(video_displays);

	for(int i=0;i<arrlen(video_mode_list);i++){
		SDL_Log("mode %d x %d x %.3fHz %f\n",video_mode_list[i]->w,video_mode_list[i]->h,video_mode_list[i]->refresh_rate,(float)video_mode_list[i]->w/(float)video_mode_list[i]->h);
	}

	video_desktop_mode=SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());
	SDL_Log("desktop size is %d x %d\n",video_desktop_mode->w,video_desktop_mode->h);

	for(int i=0;i<arrlen(video_mode_list);i++){
		//make sure its smaller AND takes the height of the window decorations into account
		if(video_desktop_mode->w > video_mode_list[i]->w){
			if(video_desktop_mode->h >video_mode_list[i]->h){
				if(video_desktop_mode->h - video_mode_list[i]->h >= 150){
				
						video_largest_window=video_mode_list[i];
						SDL_Log("match found for desktop - %d x %d\n",video_mode_list[i]->w,video_mode_list[i]->h);
						break;
					
				}
			}
		}			
				
	}
	if(video_largest_window==NULL){
		SDL_Log("no match found for largest window, setting to current desktop\n");
		video_largest_window=video_desktop_mode;
	}
	video_mode_count=arrlen(video_mode_list);
	video_initialised=true;
	return SDL_APP_CONTINUE;

}

void video_set_windowed(SDL_Window *win){
    WINDOW_WIDTH=video_largest_window->w;
    WINDOW_HEIGHT=video_largest_window->h;
    SDL_SetWindowFullscreen(win,false);                    
    SDL_SetWindowSize(win,WINDOW_WIDTH,WINDOW_HEIGHT);
    SDL_SyncWindow(win);
    int ww=0,wh=0;
   SDL_GetWindowSize(win, &ww, &wh);
    
   SDL_Log("set window to %d x %d with %d x %d\n",ww,wh,WINDOW_WIDTH,WINDOW_HEIGHT);
}

void video_set_fullscreen_desktop(SDL_Window *win){
    SDL_SetWindowFullscreenMode(win, NULL);
    SDL_SetWindowFullscreen(win,true);
    SDL_SyncWindow(win);
    int ww=0,wh=0;
    SDL_GetWindowSize(win, &ww, &wh);    
    WINDOW_WIDTH=ww;
    WINDOW_HEIGHT=wh;
    SDL_Log("set window fullscreen desktop to %d x %d with %d x %d\n",ww,wh,WINDOW_WIDTH,WINDOW_HEIGHT);
}

void video_set_exclusive_fullscreen(SDL_Window *win,SDL_DisplayMode *d){
    SDL_SetWindowFullscreenMode(win, d);
    SDL_SetWindowFullscreen(win, true);
    SDL_SyncWindow(win);
    int ww=0,wh=0;
    SDL_GetWindowSize(win, &ww, &wh);    
    WINDOW_WIDTH=ww;
    WINDOW_HEIGHT=wh;
    SDL_Log("set window fullscreen exclusive to %d x %d with %d x %d\n",ww,wh,WINDOW_WIDTH,WINDOW_HEIGHT);

}

bool video_test_aspect_ratio(float test){
    float a16x9=16.0f/9.0f;
    float a16x10=16.0f/10.0f;
    float a21x9=21.0f/9.0f;
    float a32x9=32.0f/9.0f;

    if(test==a16x9 ||
       test==a16x10 ||
       test==a21x9 ||
       test==a32x9){
        return true;
    }
    return false;

}

char *video_get_aspect_string(float test){
    float a16x9=16.0f/9.0f;
    float a16x10=16.0f/10.0f;
    float a21x9=21.0f/9.0f;
    float a32x9=32.0f/9.0f;

    if(test==a16x9) return "16:9";
    if(test==a16x10) return "16:10";
    if(test==a21x9) return "21:9";
    if(test==a32x9) return "32:9";
    return "na";
}