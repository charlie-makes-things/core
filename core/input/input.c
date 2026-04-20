



typedef enum cg_controller_type{
	CONTROLLER_TYPE_KEYBOARD,
	CONTROLLER_TYPE_MOUSE,
	CONTROLLER_TYPE_TOUCH,
	CONTROLLER_TYPE_GAMEPAD
}cg_controller_type;

typedef struct cg_controller_keyboard_map{
	SDL_Scancode left_up,left_down,left_left,left_right;
	SDL_Scancode right_up,right_down,right_left,right_right;
	SDL_Scancode trigL,trigR;
	SDL_Scancode key_a,key_b,key_x,key_y;
	SDL_Scancode key_lb,key_rb,key_lstick,key_rstick;
	SDL_Scancode key_start,key_select;
	SDL_Scancode dpadUp,dpadDown,dpadLeft,dpadRight;
}cg_controller_keyboard_map;

typedef struct cg_controller_axis_map{
	SDL_GamepadAxis leftX;
	SDL_GamepadAxis leftY;
	SDL_GamepadAxis rightX;
	SDL_GamepadAxis rightY;
	SDL_GamepadAxis trigL;
	SDL_GamepadAxis trigR;
}cg_controller_axis_map;

typedef struct cg_controller_button_map{ 
	SDL_GamepadButton buttonA;
	SDL_GamepadButton buttonB;
	SDL_GamepadButton buttonX;
	SDL_GamepadButton buttonY;
	SDL_GamepadButton buttonLB;
	SDL_GamepadButton buttonRB;
	SDL_GamepadButton buttonStart;
	SDL_GamepadButton buttonSelect;
	SDL_GamepadButton stickL;
	SDL_GamepadButton stickR;
	SDL_GamepadButton dpadUp;
    SDL_GamepadButton dpadDown;
    SDL_GamepadButton dpadLeft;
    SDL_GamepadButton dpadRight;
}cg_controller_button_map;

typedef struct cg_controller_axis_states{
	int leftX;
    int leftY;
    int rightX;
    int rightY;
    int trigL;
    int trigR;    
}cg_controller_axis_states;

typedef struct cg_controller_button_states{
	bool buttonA;
	bool buttonB;
	bool buttonX;
	bool buttonY;
	bool buttonLB;
	bool buttonRB;
	bool buttonStart;
	bool buttonSelect;
	bool stickL;
	bool stickR;
	bool dpadUp;
    bool dpadDown;
    bool dpadLeft;
    bool dpadRight;
}cg_controller_button_states;

typedef struct cg_controller{
	
	int index;//index in the array
	cg_controller_type type;
	SDL_JoystickID id;
	SDL_Gamepad *pad;

	int deadzone;
	
	bool hasRumble;
    bool hasTriggerRumble;
    bool hasPlayerLED;
    bool hasMonoLed;
    bool hasRGBLed;

    cg_controller_axis_states states;
    cg_controller_axis_states lastStates;

    cg_controller_button_states buttons;
    cg_controller_button_states lastButtons;

    cg_controller_keyboard_map map;
    cg_controller_axis_map axisMap;
    cg_controller_button_map buttonMap;

}cg_controller;

typedef enum cg_mouse_button{
	MOUSE_LEFT =1,
	MOUSE_RIGHT =2,
	MOUSE_MIDDLE =3,
	MOUSE_X1 =4,
	MOUSE_X2 =5
}cg_mouse_button;

typedef struct cg_mouse_data{
	float lastMouseX,lastMouseY;
	float mouseX,mouseY,mouseRelX,mouseRelY;
	bool mouseLeft,mouseRight,mouseMiddle;
	bool mouseX1,mouseX2;
	float mouseWheelX,mouseWheelY;
}cg_mouse_data;

static cg_controller *_input_controller_list;
static cg_controller *_input_activeController;

static cg_controller_keyboard_map _default_map;
static cg_controller_axis_map _default_axis;
static cg_controller_button_map _default_button;

static bool _input_has_keyboard=false;
static bool _input_has_mouse=false;
static cg_mouse_data _input_mouse_data;

static MIX_Track *_input_connectedTrack=NULL;
static MIX_Track *_input_disconnectedTrack=NULL;
static void *_input_connectedData=NULL;
static void *_input_disconnectedData=NULL;


static cg2d_image *_input_accept_image=NULL;
static cg2d_image *_input_cancel_image=NULL;


























float input_get_mouseX(){
	
	return _input_mouse_data.mouseX;
}

float input_get_mouseY(){
	
	return _input_mouse_data.mouseY;
}

float input_get_last_mouseX(){
	
	return _input_mouse_data.lastMouseX;
}

float input_get_last_mouseY(){
	
	return _input_mouse_data.lastMouseY;
}

float input_get_mouse_wheelX(){
	return _input_mouse_data.mouseWheelX;
}

float input_get_mouse_wheelY(){
	return _input_mouse_data.mouseWheelY;
}

bool input_get_mouse_down(cg_mouse_button button){
	switch((int)button){
		case MOUSE_LEFT:
			return _input_mouse_data.mouseLeft;
			break;
		case MOUSE_RIGHT:
			return _input_mouse_data.mouseRight;
			break;
		case MOUSE_MIDDLE:
			return _input_mouse_data.mouseMiddle;
			break;
		case MOUSE_X1:
			return _input_mouse_data.mouseX1;
			break;
		case MOUSE_X2:
			return _input_mouse_data.mouseX2;
			break;
		default:
			return false;
	}
	return false;
}

void input_get_mouse_speed(float *x,float *y){
	*x=_input_mouse_data.mouseRelX;
	*y=_input_mouse_data.mouseRelY;
}





static const char *battery_state_string(SDL_PowerState state)
{
    switch (state) {
        case SDL_POWERSTATE_ERROR: return "ERROR";
        case SDL_POWERSTATE_UNKNOWN: return "UNKNOWN";
        case SDL_POWERSTATE_ON_BATTERY: return "ON BATTERY";
        case SDL_POWERSTATE_NO_BATTERY: return "NO BATTERY";
        case SDL_POWERSTATE_CHARGING: return "CHARGING";
        case SDL_POWERSTATE_CHARGED: return "CHARGED";
        default: break;
    }
    return "UNKNOWN";
}

void cg_controller_init(cg_controller *c,cg_controller_type type, SDL_Gamepad *pad, SDL_JoystickID id,float deadzone,int index){
	SDL_Log("initialising controller %d\n",index );
	c->index=index;
	c->type=type;
	c->id=id;
	c->pad=pad;
	c->deadzone=deadzone;

	SDL_PropertiesID padProps;
    padProps= SDL_GetGamepadProperties(pad);
    if(padProps!=0){
        c->hasRumble = SDL_GetBooleanProperty(padProps, SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN,false);
        c->hasTriggerRumble = SDL_GetBooleanProperty(padProps, SDL_PROP_GAMEPAD_CAP_TRIGGER_RUMBLE_BOOLEAN,false);
        c->hasPlayerLED = SDL_GetBooleanProperty(padProps, SDL_PROP_GAMEPAD_CAP_PLAYER_LED_BOOLEAN,false);
        c->hasMonoLed = SDL_GetBooleanProperty(padProps, SDL_PROP_GAMEPAD_CAP_MONO_LED_BOOLEAN,false);
        c->hasRGBLed = SDL_GetBooleanProperty(padProps, SDL_PROP_GAMEPAD_CAP_RGB_LED_BOOLEAN,false);
    }

    c->lastStates.leftX=0.0;
    c->lastStates.leftY=0.0;
    c->lastStates.rightX=0.0;
    c->lastStates.rightY=0.0;
    c->lastStates.trigL=0.0;
    c->lastStates.trigR=0.0;

    c->lastButtons.dpadUp=false;
    c->lastButtons.dpadDown=false;
    c->lastButtons.dpadLeft=false;
    c->lastButtons.dpadRight=false;
    c->lastButtons.buttonA=false;
	c->lastButtons.buttonB=false;
	c->lastButtons.buttonX=false;
	c->lastButtons.buttonY=false;
	c->lastButtons.buttonLB=false;
	c->lastButtons.buttonRB=false;
	c->lastButtons.buttonStart=false;
	c->lastButtons.buttonSelect=false;
	c->lastButtons.stickL=false;
	c->lastButtons.stickR=false;

	c->map.left_up=_default_map.left_up;
	c->map.left_down=_default_map.left_down;
	c->map.left_left=_default_map.left_left;
	c->map.left_right=_default_map.left_right;
	c->map.right_up=_default_map.right_up;
	c->map.right_down=_default_map.right_down;
	c->map.right_left=_default_map.right_left;
	c->map.right_right=_default_map.right_right;
	c->map.key_a=_default_map.key_a;
	c->map.key_b=_default_map.key_b;
	c->map.key_x=_default_map.key_x;
	c->map.key_y=_default_map.key_y;
	c->map.key_lb=_default_map.key_lb;
	c->map.key_rb=_default_map.key_rb;
	c->map.key_lstick=_default_map.key_lstick;
	c->map.key_rstick=_default_map.key_rstick;
	c->map.key_start=_default_map.key_start;
	c->map.key_select=_default_map.key_select;
	c->map.trigL=_default_map.trigL;
	c->map.trigR=_default_map.trigR;
	c->map.dpadUp=_default_map.dpadUp;
	c->map.dpadDown=_default_map.dpadDown;
	c->map.dpadLeft=_default_map.dpadLeft;
	c->map.dpadRight=_default_map.dpadRight;

	c->axisMap.leftX=_default_axis.leftX;
	c->axisMap.leftY=_default_axis.leftY;
	c->axisMap.rightX=_default_axis.rightX;
	c->axisMap.rightY=_default_axis.rightY;
	c->axisMap.trigL=_default_axis.trigL;
	c->axisMap.trigR=_default_axis.trigR;

	c->buttonMap.buttonA=_default_button.buttonA;
	c->buttonMap.buttonB=_default_button.buttonB;
	c->buttonMap.buttonX=_default_button.buttonX;
	c->buttonMap.buttonY=_default_button.buttonY;
	c->buttonMap.buttonLB=_default_button.buttonLB;
	c->buttonMap.buttonRB=_default_button.buttonRB;
	c->buttonMap.buttonStart=_default_button.buttonStart;
	c->buttonMap.buttonSelect=_default_button.buttonSelect;
	c->buttonMap.stickL=_default_button.stickL;
	c->buttonMap.stickR=_default_button.stickR;
	c->buttonMap.dpadUp=_default_button.dpadUp;
    c->buttonMap.dpadDown=_default_button.dpadDown;
    c->buttonMap.dpadLeft=_default_button.dpadLeft;
    c->buttonMap.dpadRight=_default_button.dpadRight;
	

}

void cg_controller_remap_axis(cg_controller *c, cg_controller_axis_map *map){
	c->axisMap.leftX=map->leftX;
	c->axisMap.leftY=map->leftY;
	c->axisMap.rightX=map->rightX;
	c->axisMap.rightY=map->rightY;
	c->axisMap.trigL=map->trigL;
	c->axisMap.trigR=map->trigR;
}

void cg_controller_set_default_axismap(cg_controller *c){
	c->axisMap.leftX=_default_axis.leftX;
	c->axisMap.leftY=_default_axis.leftY;
	c->axisMap.rightX=_default_axis.rightX;
	c->axisMap.rightY=_default_axis.rightY;
	c->axisMap.trigL=_default_axis.trigL;
	c->axisMap.trigR=_default_axis.trigR;
}

void cg_controller_remap_buttons(cg_controller *c, cg_controller_button_map *map){
	c->buttonMap.buttonA=map->buttonA;
	c->buttonMap.buttonB=map->buttonB;
	c->buttonMap.buttonX=map->buttonX;
	c->buttonMap.buttonY=map->buttonY;
	c->buttonMap.buttonLB=map->buttonLB;
	c->buttonMap.buttonRB=map->buttonRB;
	c->buttonMap.buttonStart=map->buttonStart;
	c->buttonMap.buttonSelect=map->buttonSelect;
	c->buttonMap.stickL=map->stickL;
	c->buttonMap.stickR=map->stickR;
	c->buttonMap.dpadUp=map->dpadUp;
    c->buttonMap.dpadDown=map->dpadDown;
    c->buttonMap.dpadLeft=map->dpadLeft;
    c->buttonMap.dpadRight=map->dpadRight;
}

void cg_controller_set_default_buttonmap(cg_controller *c){
	c->buttonMap.buttonA=_default_button.buttonA;
	c->buttonMap.buttonB=_default_button.buttonB;
	c->buttonMap.buttonX=_default_button.buttonX;
	c->buttonMap.buttonY=_default_button.buttonY;
	c->buttonMap.buttonLB=_default_button.buttonLB;
	c->buttonMap.buttonRB=_default_button.buttonRB;
	c->buttonMap.buttonStart=_default_button.buttonStart;
	c->buttonMap.buttonSelect=_default_button.buttonSelect;
	c->buttonMap.stickL=_default_button.stickL;
	c->buttonMap.stickR=_default_button.stickR;
	c->buttonMap.dpadUp=_default_button.dpadUp;
    c->buttonMap.dpadDown=_default_button.dpadDown;
    c->buttonMap.dpadLeft=_default_button.dpadLeft;
    c->buttonMap.dpadRight=_default_button.dpadRight;
}

void cg_controller_remap_keys(cg_controller *c,cg_controller_keyboard_map *map){
	c->map.left_up=map->left_up;
	c->map.left_down=map->left_down;
	c->map.left_left=map->left_left;
	c->map.left_right=map->left_right;
	c->map.right_up=map->right_up;
	c->map.right_down=map->right_down;
	c->map.right_left=map->right_left;
	c->map.right_right=map->right_right;
	c->map.key_a=map->key_a;
	c->map.key_b=map->key_b;
	c->map.key_x=map->key_x;
	c->map.key_y=map->key_y;
	c->map.key_lb=map->key_lb;
	c->map.key_rb=map->key_rb;
	c->map.key_lstick=map->key_lstick;
	c->map.key_rstick=map->key_rstick;
	c->map.key_start=map->key_start;
	c->map.key_select=map->key_select;
	c->map.trigL=map->trigL;
	c->map.trigR=map->trigR;
	c->map.dpadUp=map->dpadUp;
	c->map.dpadDown=map->dpadDown;
	c->map.dpadLeft=map->dpadLeft;
	c->map.dpadRight=map->dpadRight;
}

void cg_controller_set_default_keymap(cg_controller *c){
	c->map.left_up=_default_map.left_up;
	c->map.left_down=_default_map.left_down;
	c->map.left_left=_default_map.left_left;
	c->map.left_right=_default_map.left_right;
	c->map.right_up=_default_map.right_up;
	c->map.right_down=_default_map.right_down;
	c->map.right_left=_default_map.right_left;
	c->map.right_right=_default_map.right_right;
	c->map.key_a=_default_map.key_a;
	c->map.key_b=_default_map.key_b;
	c->map.key_x=_default_map.key_x;
	c->map.key_y=_default_map.key_y;
	c->map.key_lb=_default_map.key_lb;
	c->map.key_rb=_default_map.key_rb;
	c->map.key_lstick=_default_map.key_lstick;
	c->map.key_rstick=_default_map.key_rstick;
	c->map.key_start=_default_map.key_start;
	c->map.key_select=_default_map.key_select;
	c->map.trigL=_default_map.trigL;
	c->map.trigR=_default_map.trigR;
	c->map.dpadUp=_default_map.dpadUp;
	c->map.dpadDown=_default_map.dpadDown;
	c->map.dpadLeft=_default_map.dpadLeft;
	c->map.dpadRight=_default_map.dpadRight;
}

SDL_AppResult input_init(MIX_Mixer *mixer){
	
	MIX_Audio *connectedAudio=NULL;
    MIX_Audio *disconnectedAudio=NULL;

    char *sfxPath="audio/SineThrow1.wav",*sfxPath2="audio/SineThrow2.wav";
    size_t sz1=0,sz2=0;
    _input_connectedData=load_file_storage(sfxPath,&sz1);
    if(_input_connectedData==NULL){SDL_Log("failed to load %s\n",sfxPath );return SDL_APP_FAILURE;}

    connectedAudio = MIX_LoadAudio_IO(mixer, SDL_IOFromMem(_input_connectedData,sz1), false,true);
    if (!connectedAudio) {
        SDL_Log("Couldn't load %s: %s", sfxPath, SDL_GetError());
        return SDL_APP_FAILURE;
    }
    

    _input_disconnectedData=load_file_storage(sfxPath2,&sz2);
    if(_input_disconnectedData==NULL){SDL_Log("failed to load %s\n",sfxPath2 );return SDL_APP_FAILURE;}

    disconnectedAudio = MIX_LoadAudio_IO(mixer, SDL_IOFromMem(_input_disconnectedData,sz2), false,true);
    if (!disconnectedAudio) {
        SDL_Log("Couldn't load %s: %s", sfxPath2, SDL_GetError());
        return SDL_APP_FAILURE;
    }
    

    _input_connectedTrack = MIX_CreateTrack(mixer);
    if (!_input_connectedTrack) {
        SDL_Log("Couldn't create a mixer track: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    _input_disconnectedTrack = MIX_CreateTrack(mixer);
    if (!_input_disconnectedTrack) {
        SDL_Log("Couldn't create a mixer track: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    MIX_SetTrackAudio(_input_connectedTrack,connectedAudio);
    MIX_SetTrackAudio(_input_disconnectedTrack, disconnectedAudio);


	_default_map.left_up=SDL_SCANCODE_W;
	_default_map.left_down=SDL_SCANCODE_S;
	_default_map.left_left=SDL_SCANCODE_A;
	_default_map.left_right=SDL_SCANCODE_D;
	_default_map.right_up=SDL_SCANCODE_I;
	_default_map.right_down=SDL_SCANCODE_K;
	_default_map.right_left=SDL_SCANCODE_J;
	_default_map.right_right=SDL_SCANCODE_L;
	_default_map.key_a=SDL_SCANCODE_SPACE;
	_default_map.key_b=SDL_SCANCODE_ESCAPE;
	_default_map.key_x=SDL_SCANCODE_Z;
	_default_map.key_y=SDL_SCANCODE_X;
	_default_map.key_lb=SDL_SCANCODE_C;
	_default_map.key_rb=SDL_SCANCODE_V;
	_default_map.key_lstick=SDL_SCANCODE_B;
	_default_map.key_rstick=SDL_SCANCODE_N;
	_default_map.key_start=SDL_SCANCODE_ESCAPE;
	_default_map.key_select=SDL_SCANCODE_M;
	_default_map.trigL=SDL_SCANCODE_1;
	_default_map.trigR=SDL_SCANCODE_2;
	_default_map.dpadUp=SDL_SCANCODE_UP;
	_default_map.dpadDown=SDL_SCANCODE_DOWN;
	_default_map.dpadLeft=SDL_SCANCODE_LEFT;
	_default_map.dpadRight=SDL_SCANCODE_RIGHT;


	_default_axis.leftX=SDL_GAMEPAD_AXIS_LEFTX;
	_default_axis.leftY=SDL_GAMEPAD_AXIS_LEFTY;
	_default_axis.rightX=SDL_GAMEPAD_AXIS_RIGHTX;
	_default_axis.rightY=SDL_GAMEPAD_AXIS_RIGHTY;
	_default_axis.trigL=SDL_GAMEPAD_AXIS_LEFT_TRIGGER;
	_default_axis.trigR=SDL_GAMEPAD_AXIS_RIGHT_TRIGGER;
	
	_default_button.dpadUp=SDL_GAMEPAD_BUTTON_DPAD_UP;
    _default_button.dpadDown=SDL_GAMEPAD_BUTTON_DPAD_DOWN;
    _default_button.dpadLeft=SDL_GAMEPAD_BUTTON_DPAD_LEFT;
    _default_button.dpadRight=SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
    _default_button.buttonA=SDL_GAMEPAD_BUTTON_SOUTH;
	_default_button.buttonB=SDL_GAMEPAD_BUTTON_EAST;
	_default_button.buttonX=SDL_GAMEPAD_BUTTON_WEST;
	_default_button.buttonY=SDL_GAMEPAD_BUTTON_NORTH;
	_default_button.buttonLB=SDL_GAMEPAD_BUTTON_LEFT_SHOULDER;
	_default_button.buttonRB=SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER;
	_default_button.buttonStart=SDL_GAMEPAD_BUTTON_START;
	_default_button.buttonSelect=SDL_GAMEPAD_BUTTON_BACK;
	_default_button.stickL=SDL_GAMEPAD_BUTTON_LEFT_STICK;
	_default_button.stickR=SDL_GAMEPAD_BUTTON_RIGHT_STICK;
	

	SDL_PollEvent(NULL);
	if(SDL_HasKeyboard()){
        SDL_Log("found keyboard\n");
    }
    if(SDL_HasMouse()){
        SDL_Log("found mouse\n");
        cg_controller c;
        cg_controller_init(&c,CONTROLLER_TYPE_MOUSE,NULL,0,0,arrlen(_input_controller_list));
        arrpush(_input_controller_list,c);
        _input_has_mouse=true;
    }


    if(SDL_HasScreenKeyboardSupport()==false && SDL_HasKeyboard()==true){
        cg_controller c;
        cg_controller_init(&c,CONTROLLER_TYPE_KEYBOARD, NULL,0,8000,arrlen(_input_controller_list));        
        arrpush(_input_controller_list,c);
        _input_has_keyboard=true;     
    }

   _input_activeController=NULL;

   return SDL_APP_CONTINUE;
    
}

cg_controller* input_get_keyboard_controller(){
	if(_input_has_keyboard==true){
		for(int i=0;i<arrlen(_input_controller_list);i++){
			if(_input_controller_list[i].type==CONTROLLER_TYPE_KEYBOARD){
				return &_input_controller_list[i];
			}
		}
	}
	return NULL;
}

cg_controller* input_get_mouse_controller(){
	if(_input_has_keyboard==true){
		for(int i=0;i<arrlen(_input_controller_list);i++){
			if(_input_controller_list[i].type==CONTROLLER_TYPE_MOUSE){
				return &_input_controller_list[i];
			}
		}
	}
	return NULL;
}

cg_mouse_data *input_get_mouse_data(){
	return &_input_mouse_data;
}

void input_free(){
	for(int i=0;i<arrlen(_input_controller_list);i++){
		SDL_Log("freeing controller id %d index %d\n",_input_controller_list[i].id,i );
		if(_input_controller_list[i].type==CONTROLLER_TYPE_GAMEPAD){
			if(_input_controller_list[i].pad){
				SDL_Log("freeing pad for id %d\n",_input_controller_list[i].id);
				if(_input_controller_list[i].pad!=NULL){
					SDL_CloseGamepad(_input_controller_list[i].pad);

				}else{
					SDL_Log("pad is null for id %d\n",_input_controller_list[i].id );
				}
			}
		}
	}
	SDL_Log("freeing list\n");
	arrfree(_input_controller_list);
	SDL_Log("freeing input audio data\n");
	SDL_free(_input_connectedData);
	SDL_free(_input_disconnectedData);
	SDL_Log("done\n");
}

void input_remove_controller(int which){
	for(int i=0;i<arrlen(_input_controller_list);i++){
		if(_input_controller_list[i].id==which){
			SDL_Log("deleting controller %d, index %d\n",which,i);
			if(which>0){
				SDL_CloseGamepad(_input_controller_list[i].pad);
			}
			if(&_input_controller_list[i]==_input_activeController){
				SDL_Log("setting active controller to null\n");
				_input_activeController=NULL;
			}
			MIX_PlayTrack(_input_disconnectedTrack,0);
			arrdelswap(_input_controller_list,i);
			break;
		}else{
			//SDL_Log("couldn't find id %d index %d\n",which,i);
		}
	}
}

cg_controller *input_add_controller(cg_controller_type type, SDL_Gamepad *pad, SDL_JoystickID id,float deadzone){
	cg_controller c;
	cg_controller_init(&c,type,pad, id,deadzone,arrlen(_input_controller_list));
	arrpush(_input_controller_list,c);
	MIX_PlayTrack(_input_connectedTrack,0);
	return &_input_controller_list[arrlen(_input_controller_list)-1];
}




int input_get_connected_controllers(){
	return arrlen(_input_controller_list);
}

cg_controller *input_get_controller_by_index(int index){
	if(index<arrlen(_input_controller_list) && index>=0){
		
		return &_input_controller_list[index];
	}
	
	return NULL;
}

cg_controller *input_get_controller_by_id(int id){
	for(int i=0;i<arrlen(_input_controller_list);i++){
		if(_input_controller_list[i].id==id){
			return &_input_controller_list[i];
		}
	}
	return NULL;
}

int input_get_controller_index_from_id(uint id){
	for(int i=0;i<arrlen(_input_controller_list);i++){
		if(_input_controller_list[i].id==id){
			return i;
		}
	}
	return -1;
}

int input_get_controller_index(cg_controller *c){
	if(c!=NULL){
		return c->index;
	}
	return -1;
}

const char *cg_controller_get_name(cg_controller *c){
	if(c->type==CONTROLLER_TYPE_KEYBOARD){
		return "keyboard";
	}else if (c->type==CONTROLLER_TYPE_MOUSE){
		return "mouse";
	}else{
		return SDL_GetGamepadName(c->pad);
	}
}


void cg_controller_update(cg_controller *c){



	if(c->type==CONTROLLER_TYPE_GAMEPAD){

		c->lastStates.leftX=c->states.leftX;
	    c->lastStates.leftY=c->states.leftY;
	    c->lastStates.rightX=c->states.rightX;
	    c->lastStates.rightY=c->states.rightY;
	    c->lastStates.trigL=c->states.trigL;
	    c->lastStates.trigR=c->states.trigR;
	    
	    c->lastButtons.dpadUp=c->buttons.dpadUp;
	    c->lastButtons.dpadDown=c->buttons.dpadDown;
	    c->lastButtons.dpadLeft=c->buttons.dpadLeft;
	    c->lastButtons.dpadRight=c->buttons.dpadRight;
	    c->lastButtons.buttonA=c->buttons.buttonA;
		c->lastButtons.buttonB=c->buttons.buttonB;
		c->lastButtons.buttonX=c->buttons.buttonX;
		c->lastButtons.buttonY=c->buttons.buttonY;
		c->lastButtons.buttonLB=c->buttons.buttonLB;
		c->lastButtons.buttonRB=c->buttons.buttonRB;
		c->lastButtons.buttonStart=c->buttons.buttonStart;
		c->lastButtons.buttonSelect=c->buttons.buttonSelect;
		c->lastButtons.stickL=c->buttons.stickL;
		c->lastButtons.stickR=c->buttons.stickR;



		Sint16 leftX=SDL_GetGamepadAxis(c->pad,c->axisMap.leftX);
	    Sint16 leftY=SDL_GetGamepadAxis(c->pad,c->axisMap.leftY);
	    Sint16 rightX=SDL_GetGamepadAxis(c->pad,c->axisMap.rightX);
	    Sint16 rightY=SDL_GetGamepadAxis(c->pad,c->axisMap.rightY);
	    Sint16 trigL=SDL_GetGamepadAxis(c->pad, c->axisMap.trigL);
	    Sint16 trigR=SDL_GetGamepadAxis(c->pad, c->axisMap.trigR);
	    
	    c->states.leftX=(SDL_abs(leftX)>c->deadzone) ? leftX : 0;
	    c->states.leftY=(SDL_abs(leftY)>c->deadzone) ? leftY : 0;
	    c->states.rightX=(SDL_abs(rightX)>c->deadzone) ? rightX : 0;
	    c->states.rightY=(SDL_abs(rightY)>c->deadzone) ? rightY : 0;
	    c->states.trigL=(SDL_abs(trigL)>c->deadzone) ? trigL : 0;
	    c->states.trigR=(SDL_abs(trigR)>c->deadzone) ? trigR : 0;
	    


	    c->buttons.dpadUp=SDL_GetGamepadButton(c->pad,c->buttonMap.dpadUp);
	    c->buttons.dpadDown=SDL_GetGamepadButton(c->pad,c->buttonMap.dpadDown);
	    c->buttons.dpadLeft=SDL_GetGamepadButton(c->pad,c->buttonMap.dpadLeft);
	    c->buttons.dpadRight=SDL_GetGamepadButton(c->pad,c->buttonMap.dpadRight);
	    c->buttons.buttonA=SDL_GetGamepadButton(c->pad,c->buttonMap.buttonA);
		c->buttons.buttonB=SDL_GetGamepadButton(c->pad,c->buttonMap.buttonB);
		c->buttons.buttonX=SDL_GetGamepadButton(c->pad,c->buttonMap.buttonX);
		c->buttons.buttonY=SDL_GetGamepadButton(c->pad,c->buttonMap.buttonY);
		c->buttons.buttonLB=SDL_GetGamepadButton(c->pad,c->buttonMap.buttonLB);
		c->buttons.buttonRB=SDL_GetGamepadButton(c->pad,c->buttonMap.buttonRB);
		c->buttons.buttonStart=SDL_GetGamepadButton(c->pad,c->buttonMap.buttonStart);
		c->buttons.buttonSelect=SDL_GetGamepadButton(c->pad,c->buttonMap.buttonSelect);
		c->buttons.stickL=SDL_GetGamepadButton(c->pad,c->buttonMap.stickL);
		c->buttons.stickR=SDL_GetGamepadButton(c->pad,c->buttonMap.stickR);
	
	}else if(c->type==CONTROLLER_TYPE_KEYBOARD){
		
		const bool *key_states = SDL_GetKeyboardState(NULL);

		Sint16 left_up=(key_states[c->map.left_up]==true) ? 32768 : 0;
		Sint16 left_down=(key_states[c->map.left_down]==true) ? 32768 : 0;
		Sint16 left_left=(key_states[c->map.left_left]==true) ? 32768 : 0;
		Sint16 left_right=(key_states[c->map.left_right]==true) ? 32768 : 0;
		Sint16 right_up=(key_states[c->map.right_up]==true) ? 32768 : 0;
		Sint16 right_down=(key_states[c->map.right_down]==true) ? 32768 : 0;
		Sint16 right_left=(key_states[c->map.right_left]==true) ? 32768 : 0;
		Sint16 right_right=(key_states[c->map.right_right]==true) ? 32768 : 0;
		Sint16 trigL=(key_states[c->map.trigL]==true) ? 32768 : 0;
		Sint16 trigR=(key_states[c->map.trigR]==true) ? 32768 : 0;
		Sint16 key_a=(key_states[c->map.key_a]==true) ? 1 : 0;
		Sint16 key_b=(key_states[c->map.key_b]==true) ? 1 : 0;
		Sint16 key_x=(key_states[c->map.key_x]==true) ? 1 : 0;
		Sint16 key_y=(key_states[c->map.key_y]==true) ? 1 : 0;
		Sint16 key_lb=(key_states[c->map.key_lb]==true) ? 1 : 0;
		Sint16 key_rb=(key_states[c->map.key_rb]==true) ? 1 : 0;
		Sint16 key_lstick=(key_states[c->map.key_lstick]==true) ? 1 : 0;
		Sint16 key_rstick=(key_states[c->map.key_rstick]==true) ? 1 : 0;
		Sint16 key_start=(key_states[c->map.key_start]==true) ? 1 : 0;
		Sint16 key_select=(key_states[c->map.key_select]==true) ? 1 : 0;
		Sint16 dpadUp=(key_states[c->map.dpadUp]==true) ? 1 : 0;
		Sint16 dpadDown=(key_states[c->map.dpadDown]==true) ? 1 : 0;
		Sint16 dpadLeft=(key_states[c->map.dpadLeft]==true) ? 1 : 0;
		Sint16 dpadRight=(key_states[c->map.dpadRight]==true) ? 1 : 0;

		c->lastStates.leftX=c->states.leftX;
	    c->lastStates.leftY=c->states.leftY;
	    c->lastStates.rightX=c->states.rightX;
	    c->lastStates.rightY=c->states.rightY;
	    c->lastStates.trigL=c->states.trigL;
	    c->lastStates.trigR=c->states.trigR;
	    
	    c->lastButtons.dpadUp=c->buttons.dpadUp;
	    c->lastButtons.dpadDown=c->buttons.dpadDown;
	    c->lastButtons.dpadLeft=c->buttons.dpadLeft;
	    c->lastButtons.dpadRight=c->buttons.dpadRight;
	    c->lastButtons.buttonA=c->buttons.buttonA;
		c->lastButtons.buttonB=c->buttons.buttonB;
		c->lastButtons.buttonX=c->buttons.buttonX;
		c->lastButtons.buttonY=c->buttons.buttonY;
		c->lastButtons.buttonLB=c->buttons.buttonLB;
		c->lastButtons.buttonRB=c->buttons.buttonRB;
		c->lastButtons.buttonStart=c->buttons.buttonStart;
		c->lastButtons.buttonSelect=c->buttons.buttonSelect;
		c->lastButtons.stickL=c->buttons.stickL;
		c->lastButtons.stickR=c->buttons.stickR;

		
		c->states.leftX=(left_left-left_right)+((dpadRight-dpadLeft)*32768);
	    c->states.leftY=(left_up-left_down)+((dpadDown-dpadUp)*32768);
	    c->states.rightX=(right_left-right_right);
	    c->states.rightY=(right_up-right_down);
	    c->states.trigL=-trigL;
	    c->states.trigR=-trigR;
	    
	    c->buttons.dpadUp=dpadUp==1;
	    c->buttons.dpadDown=dpadDown==1;
	    c->buttons.dpadLeft=dpadLeft==1;
	    c->buttons.dpadRight=dpadRight==1;
	    c->buttons.buttonA=key_a==1;
		c->buttons.buttonB=key_b==1;
		c->buttons.buttonX=key_x==1;
		c->buttons.buttonY=key_y==1;
		c->buttons.buttonLB=key_lb==1;
		c->buttons.buttonRB=key_rb==1;
		c->buttons.buttonStart=key_start==1;
		c->buttons.buttonSelect=key_select==1;
		c->buttons.stickL=key_lstick==1;
		c->buttons.stickR=key_rstick==1;


	}else if(c->type==CONTROLLER_TYPE_MOUSE){
		


		c->lastStates.leftX=c->states.leftX;
	    c->lastStates.leftY=c->states.leftY;
	    c->lastStates.rightX=c->states.rightX;
	    c->lastStates.rightY=c->states.rightY;
	    c->lastStates.trigL=c->states.trigL;
	    c->lastStates.trigR=c->states.trigR;
	    
	    c->lastButtons.dpadUp=c->buttons.dpadUp;
	    c->lastButtons.dpadDown=c->buttons.dpadDown;
	    c->lastButtons.dpadLeft=c->buttons.dpadLeft;
	    c->lastButtons.dpadRight=c->buttons.dpadRight;
	    c->lastButtons.buttonA=c->buttons.buttonA;
		c->lastButtons.buttonB=c->buttons.buttonB;
		c->lastButtons.buttonX=c->buttons.buttonX;
		c->lastButtons.buttonY=c->buttons.buttonY;
		c->lastButtons.buttonLB=c->buttons.buttonLB;
		c->lastButtons.buttonRB=c->buttons.buttonRB;
		c->lastButtons.buttonStart=c->buttons.buttonStart;
		c->lastButtons.buttonSelect=c->buttons.buttonSelect;
		c->lastButtons.stickL=c->buttons.stickL;
		c->lastButtons.stickR=c->buttons.stickR;


		c->states.leftX=_input_mouse_data.mouseRelX*8000;
	    c->states.leftY=_input_mouse_data.mouseRelY*8000;
	    c->states.rightX=_input_mouse_data.mouseWheelX*8000;
	    c->states.rightY=_input_mouse_data.mouseWheelY*8000;
	    c->states.trigL=_input_mouse_data.mouseRight==true ? 32768 :0;
	    c->states.trigR=_input_mouse_data.mouseRight==true ? 32768 :0;
	    
	    c->buttons.dpadUp=0;
	    c->buttons.dpadDown=0;
	    c->buttons.dpadLeft=0;
	    c->buttons.dpadRight=0;
	    c->buttons.buttonA=_input_mouse_data.mouseLeft==true ? 1:0;
		c->buttons.buttonB=_input_mouse_data.mouseRight==true ? 1:0;
		c->buttons.buttonX=_input_mouse_data.mouseMiddle==true ? 1:0;
		c->buttons.buttonY=_input_mouse_data.mouseX1==true ? 1:0;
		c->buttons.buttonLB=_input_mouse_data.mouseRight==true ? 1:0;
		c->buttons.buttonRB=_input_mouse_data.mouseRight==true ? 1:0;
		c->buttons.buttonStart=_input_mouse_data.mouseLeft==true ? 1:0;
		c->buttons.buttonSelect=_input_mouse_data.mouseRight==true ? 1:0;
		c->buttons.stickL=0;
		c->buttons.stickR=0;

		
		_input_mouse_data.mouseRelX=0.0;
	_input_mouse_data.mouseRelY=0.0;
	_input_mouse_data.mouseWheelX=0.0;
	_input_mouse_data.mouseWheelY=0.0;
		
	}
}



int cg_controller_get_axis(cg_controller *c,SDL_GamepadAxis axis){
	switch(axis){
		case SDL_GAMEPAD_AXIS_LEFTX:
			return c->states.leftX;
			break;
		case SDL_GAMEPAD_AXIS_LEFTY:
			return c->states.leftY;
			break;
		case SDL_GAMEPAD_AXIS_RIGHTX:
			return c->states.rightX;
			break;
		case SDL_GAMEPAD_AXIS_RIGHTY:
			return c->states.rightY;
			break;
		case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
			return c->states.trigL;
			break;
		case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
			return c->states.trigR;
			break;
		default:
			return 0;
			break;
		
	}
	return 0;
}

float cg_controller_get_axis_normalized(cg_controller *c,SDL_GamepadAxis axis){
	
	switch(axis){
		case SDL_GAMEPAD_AXIS_LEFTX:
			return (float)c->states.leftX/32768.0f;
			break;
		case SDL_GAMEPAD_AXIS_LEFTY:
			return (float)c->states.leftY/32768.0f;
			break;
		case SDL_GAMEPAD_AXIS_RIGHTX:
			return (float)c->states.rightX/32768.0f;
			break;
		case SDL_GAMEPAD_AXIS_RIGHTY:
			return (float)c->states.rightY/32768.0f;
			break;
		case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
			return (float)c->states.trigL/32768.0f;
			break;
		case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
			return (float)c->states.trigR/32768.0f;
			break;
		default:
			return 0.0f;
			break;
		
	}
	return 0.0f;
}

bool cg_controller_get_button(cg_controller *c,SDL_GamepadButton button){
	switch(button){
		case SDL_GAMEPAD_BUTTON_SOUTH:
			return c->buttons.buttonA;
			break;
		case SDL_GAMEPAD_BUTTON_EAST:
			return c->buttons.buttonB;
			break;
		case SDL_GAMEPAD_BUTTON_WEST:
			return c->buttons.buttonX;
			break;
		case SDL_GAMEPAD_BUTTON_NORTH:
			return c->buttons.buttonY;
			break;
		case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
			return c->buttons.buttonLB;
			break;
		case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
			return c->buttons.buttonRB;
			break;
		case SDL_GAMEPAD_BUTTON_LEFT_STICK:
			return c->buttons.stickL;
			break;
		case SDL_GAMEPAD_BUTTON_RIGHT_STICK:
			return c->buttons.stickR;
			break;
		case SDL_GAMEPAD_BUTTON_START:
			return c->buttons.buttonStart;
			break;
		case SDL_GAMEPAD_BUTTON_BACK:
			return c->buttons.buttonSelect;
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_UP:
			return c->buttons.dpadUp;
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
			return c->buttons.dpadDown;
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
			return c->buttons.dpadLeft;
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
			return c->buttons.dpadRight;
			break;		
		default:
			return false;
			break;		
	}
	return false;
}

bool cg_controller_get_button_pressed(cg_controller *c,SDL_GamepadButton button){
	switch(button){
		case SDL_GAMEPAD_BUTTON_SOUTH:
			return (c->buttons.buttonA==true && c->lastButtons.buttonA==false) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_EAST:
			return (c->buttons.buttonB==true && c->lastButtons.buttonB==false) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_WEST:
			return (c->buttons.buttonX==true && c->lastButtons.buttonX==false) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_NORTH:
			return (c->buttons.buttonY==true && c->lastButtons.buttonY==false) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
			return (c->buttons.buttonLB==true && c->lastButtons.buttonLB==false) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
			return (c->buttons.buttonRB==true && c->lastButtons.buttonRB==false) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_LEFT_STICK:
			return (c->buttons.stickL==true && c->lastButtons.stickL==false) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_RIGHT_STICK:
			return (c->buttons.stickR==true && c->lastButtons.stickR==false) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_START:
			return (c->buttons.buttonStart==true && c->lastButtons.buttonStart==false) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_BACK:
			return (c->buttons.buttonSelect==true && c->lastButtons.buttonSelect==false) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_UP:
			return (c->buttons.dpadUp==true && c->lastButtons.dpadUp==false) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
			return (c->buttons.dpadDown==true && c->lastButtons.dpadDown==false) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
			return (c->buttons.dpadLeft==true && c->lastButtons.dpadLeft==false) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
			return (c->buttons.dpadRight==true && c->lastButtons.dpadRight==false) ? true : false;
			break;		
		default:
			return false;
			break;		
	}
	return false;
}


bool cg_controller_get_button_released(cg_controller *c,SDL_GamepadButton button){
	switch(button){
		case SDL_GAMEPAD_BUTTON_SOUTH:
			return (c->buttons.buttonA==false && c->lastButtons.buttonA==true) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_EAST:
			return (c->buttons.buttonB==false && c->lastButtons.buttonB==true) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_WEST:
			return (c->buttons.buttonX==false && c->lastButtons.buttonX==true) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_NORTH:
			return (c->buttons.buttonY==false && c->lastButtons.buttonY==true) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
			return (c->buttons.buttonLB==false && c->lastButtons.buttonLB==true) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
			return (c->buttons.buttonRB==false && c->lastButtons.buttonRB==true) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_LEFT_STICK:
			return (c->buttons.stickL==false && c->lastButtons.stickL==true) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_RIGHT_STICK:
			return (c->buttons.stickR==false && c->lastButtons.stickR==true) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_START:
			return (c->buttons.buttonStart==false && c->lastButtons.buttonStart==true) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_BACK:
			return (c->buttons.buttonSelect==false && c->lastButtons.buttonSelect==true) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_UP:
			return (c->buttons.dpadUp==false && c->lastButtons.dpadUp==true) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
			return (c->buttons.dpadDown==false && c->lastButtons.dpadDown==true) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
			return (c->buttons.dpadLeft==false && c->lastButtons.dpadLeft==true) ? true : false;
			break;
		case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
			return (c->buttons.dpadRight==false && c->lastButtons.dpadRight==true) ? true : false;
			break;		
		default:
			return false;
			break;		
	}
	return false;
}

char *cg_controller_get_buttons_string(cg_controller *c,char *out,size_t len){

	SDL_strlcpy(out,"buttons: ",sizeof(char)*len);
	if(cg_controller_get_button(c,SDL_GAMEPAD_BUTTON_SOUTH)){
		SDL_strlcat(out,SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_SOUTH),sizeof(char)*len);
		SDL_strlcat(out,", ",sizeof(char)*len);
	}
	if(cg_controller_get_button(c,SDL_GAMEPAD_BUTTON_EAST)){
		SDL_strlcat(out,SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_EAST),sizeof(char)*len);
		SDL_strlcat(out,", ",sizeof(char)*len);
	}
	if(cg_controller_get_button(c,SDL_GAMEPAD_BUTTON_WEST)){
		SDL_strlcat(out,SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_WEST),sizeof(char)*len);
		SDL_strlcat(out,", ",sizeof(char)*len);
	}
	if(cg_controller_get_button(c,SDL_GAMEPAD_BUTTON_NORTH)){
		SDL_strlcat(out,SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_NORTH),sizeof(char)*len);
		SDL_strlcat(out,", ",sizeof(char)*len);
	}
	if(cg_controller_get_button(c,SDL_GAMEPAD_BUTTON_LEFT_SHOULDER)){
		SDL_strlcat(out,SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER),sizeof(char)*len);
		SDL_strlcat(out,", ",sizeof(char)*len);
	}
	if(cg_controller_get_button(c,SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)){
		SDL_strlcat(out,SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER),sizeof(char)*len);
		SDL_strlcat(out,", ",sizeof(char)*len);
	}
	if(cg_controller_get_button(c,SDL_GAMEPAD_BUTTON_LEFT_STICK)){
		SDL_strlcat(out,SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_LEFT_STICK),sizeof(char)*len);
		SDL_strlcat(out,", ",sizeof(char)*len);
	}
	if(cg_controller_get_button(c,SDL_GAMEPAD_BUTTON_RIGHT_STICK)){
		SDL_strlcat(out,SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_RIGHT_STICK),sizeof(char)*len);
		SDL_strlcat(out,", ",sizeof(char)*len);
	}
	if(cg_controller_get_button(c,SDL_GAMEPAD_BUTTON_START)){
		SDL_strlcat(out,SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_START),sizeof(char)*len);
		SDL_strlcat(out,", ",sizeof(char)*len);
	}
	if(cg_controller_get_button(c,SDL_GAMEPAD_BUTTON_BACK)){
		SDL_strlcat(out,SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_BACK),sizeof(char)*len);
		SDL_strlcat(out,", ",sizeof(char)*len);
	}
	if(cg_controller_get_button(c,SDL_GAMEPAD_BUTTON_DPAD_UP)){
		SDL_strlcat(out,SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_DPAD_UP),sizeof(char)*len);
		SDL_strlcat(out,", ",sizeof(char)*len);
	}
	if(cg_controller_get_button(c,SDL_GAMEPAD_BUTTON_DPAD_DOWN)){
		SDL_strlcat(out,SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_DPAD_DOWN),sizeof(char)*len);
		SDL_strlcat(out,", ",sizeof(char)*len);
	}
	if(cg_controller_get_button(c,SDL_GAMEPAD_BUTTON_DPAD_LEFT)){
		SDL_strlcat(out,SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_DPAD_LEFT),sizeof(char)*len);
		SDL_strlcat(out,", ",sizeof(char)*len);
	}
	if(cg_controller_get_button(c,SDL_GAMEPAD_BUTTON_DPAD_RIGHT)){
		SDL_strlcat(out,SDL_GetGamepadStringForButton(SDL_GAMEPAD_BUTTON_DPAD_RIGHT),sizeof(char)*len);
		SDL_strlcat(out,", ",sizeof(char)*len);
	}

	return out;

}

bool cg_controller_get_any_input(cg_controller *c){


	if(SDL_fabsf(c->states.leftX)>(c->deadzone/32758)) return true;
	if(SDL_fabsf(c->states.leftY)>(c->deadzone/32758)) return true;
	if(SDL_fabsf(c->states.rightX)>(c->deadzone/32758)) return true;
	if(SDL_fabsf(c->states.rightY)>(c->deadzone/32758)) return true;
	if(SDL_fabsf(c->states.trigL)>(c->deadzone/32758)) return true;
	if(SDL_fabsf(c->states.trigR)>(c->deadzone/32758)) return true;
	
	for(int i=0;i<16;i++){
		if(cg_controller_get_button(c,i)==true) return true;
	}

	return false;
}


void input_update(){
	for(int i=0;i<arrlen(_input_controller_list);i++){
		cg_controller_update(&_input_controller_list[i]);
		if(cg_controller_get_any_input(&_input_controller_list[i])==true){
			_input_activeController=&_input_controller_list[i];
		}
	}

}

cg_controller *input_get_active_controller(){
	return _input_activeController;
}


void input_update_events(SDL_Event *event ,int *controllerChangeIndex,cg2d_t *c2d){

	cg_mouse_data *md=input_get_mouse_data();
	

	if(event->type == SDL_EVENT_MOUSE_MOTION){
        _input_mouse_data.lastMouseX=_input_mouse_data.mouseX;
        _input_mouse_data.lastMouseY=_input_mouse_data.mouseY;
        SDL_MouseMotionEvent *mev=(SDL_MouseMotionEvent*)event;
        md->mouseX=mev->x;
        md->mouseY=mev->y;
        md->mouseRelX=mev->xrel;
        md->mouseRelY=mev->yrel;
    
    }else if(event->type == SDL_EVENT_MOUSE_WHEEL){
        
        SDL_MouseWheelEvent *mev=(SDL_MouseWheelEvent*)event;
        if(mev->direction == SDL_MOUSEWHEEL_FLIPPED){
            md->mouseWheelX=-mev->x;
            md->mouseWheelY=-mev->y;
        }else{
            md->mouseWheelX=mev->x;
            md->mouseWheelY=mev->y;
        }
    }else if(event->type== SDL_EVENT_MOUSE_BUTTON_DOWN){

        SDL_MouseButtonEvent *mbut=(SDL_MouseButtonEvent*)event;
        if(mbut->button==SDL_BUTTON_LEFT ) md->mouseLeft=true;        
        if(mbut->button==SDL_BUTTON_RIGHT ) md->mouseRight=true;        
        if(mbut->button==SDL_BUTTON_MIDDLE ) md->mouseMiddle=true;        
        if(mbut->button==SDL_BUTTON_X1 ) md->mouseX1=true;        
        if(mbut->button==SDL_BUTTON_X2 ) md->mouseX2=true;       

    }else if(event->type== SDL_EVENT_MOUSE_BUTTON_UP){

        SDL_MouseButtonEvent *mbut=(SDL_MouseButtonEvent*)event;
        if(mbut->button==SDL_BUTTON_LEFT) md->mouseLeft=false;        
        if(mbut->button==SDL_BUTTON_RIGHT) md->mouseRight=false;        
        if(mbut->button==SDL_BUTTON_MIDDLE) md->mouseMiddle=false;        
        if(mbut->button==SDL_BUTTON_X1) md->mouseX1=false;        
        if(mbut->button==SDL_BUTTON_X2) md->mouseX2=false;       

    } else if (event->type == SDL_EVENT_GAMEPAD_ADDED) {
        /* this event is sent for each hotplugged stick, but also each already-connected gamepad during SDL_Init(). */
        const SDL_JoystickID which = event->gdevice.which;
        SDL_Gamepad *gamepad = SDL_OpenGamepad(which);
        if (!gamepad) {
           SDL_Log("Gamepad #%u add, but not opened: %s\n", (unsigned int) which, SDL_GetError());
        } else {
                      
            cg_controller *c=input_add_controller(CONTROLLER_TYPE_GAMEPAD,gamepad,which,8000);
            *controllerChangeIndex=input_get_controller_index(c);
            SDL_Log("adding controller %d\n",which);
            
            popup_messags_add_message("Controller Connected!",(char*)cg_controller_get_name(c),c2d, _input_accept_image,0.2);

            }
    } else if (event->type == SDL_EVENT_GAMEPAD_REMOVED) {
        const SDL_JoystickID which = event->gdevice.which;
        SDL_Gamepad *gamepad = SDL_GetGamepadFromID(which);
        if (gamepad) {            
            
            popup_messags_add_message("Controller Disonnected!",(char*)SDL_GetGamepadName(gamepad),c2d, _input_cancel_image,0.2);
                       
            *controllerChangeIndex=input_get_controller_index_from_id(which);
            SDL_Log("index is %d\n",*controllerChangeIndex );
            input_remove_controller(which);
        }
           
        
    }  else if (event->type == SDL_EVENT_JOYSTICK_BATTERY_UPDATED) {
        const SDL_JoystickID which = event->jbattery.which;
        if (SDL_IsGamepad(which)) {  /* this is only reported for joysticks, so make sure this joystick is _actually_ a gamepad. */
        SDL_Log("Gamepad #%u battery -> %s - %d%%\n", (unsigned int) which, battery_state_string(event->jbattery.state), event->jbattery.percent);
        }
    }

	return;
}


void input_set_images(cg2d_image *accept,cg2d_image *cancel){
	_input_accept_image=accept;
	_input_cancel_image=cancel;
}

