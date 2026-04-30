
#define FRAMERATE_DEFAULT 60

static Uint32 _framecount;
static float _rateticks;
static Uint64 _baseticks;
static Uint64 _lastticks;
static Uint32 _rate;
static bool _framerate_initialised=false;


void init_framerate(){
	_framecount = 0;
	_rate = FRAMERATE_DEFAULT;
	_rateticks = (1000.0f / (float) FRAMERATE_DEFAULT);
	_baseticks = SDL_max(1,SDL_GetTicks());
	_lastticks = _baseticks;
	_framerate_initialised=true;
}

bool set_framerate(Uint32 framerate){
	if ((framerate >= 30)) {
		_framecount = 0;
		_rate = framerate;
		_rateticks = (1000.0f / (float) framerate);
		return true;
	} else {
		return false;
	}
}

int get_framerate(){
	if (_framerate_initialised==false)
		return (-1);
	
	return (int)_rate;	
}


Uint32 get_framecount(){
	if (_framerate_initialised==false)
		return (-1);
	
	return (_framecount);	
}

Uint64 framerate_delay(){
	Uint64 current;
	Uint64 target;
	Uint64 delay;
	Uint64 time_passed = 0;

	if (_framerate_initialised==false) {
		return 0;
	}

	if (_baseticks == 0) {
		init_framerate();
	}

	_framecount++;

	current = SDL_max(1,SDL_GetTicks());
	time_passed = current - _lastticks;
	_lastticks = current;
	target = _baseticks + (Uint64) ((float)_framecount * _rateticks);

	if (current <= target) {
		delay = target - current;
		SDL_Delay(delay);
	} else {
		_framecount = 0;
		_baseticks = SDL_max(1,SDL_GetTicks());
	}

	return time_passed;
}