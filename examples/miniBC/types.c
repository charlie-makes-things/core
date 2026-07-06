
//enums
typedef enum cg_audio_type{
    AUDIO_TYPE_MUSIC,
    AUDIO_TYPE_SFX
}cg_audio_type;

typedef enum enemy_type{
    ENEMY_GRUNT,
    ENEMY_TANK,
    ENEMY_INV,
    ENEMY_MINI,
    ENEMY_RING,
    ENEMY_ORBITER,
    ENEMY_ELECTRODE,
    ENEMY_MINE,
    ENEMY_THREEWAY,
    ENEMY_FOURWAY,
    ENEMY_RING3,
    ENEMY_STRAIGHT,
    ENEMY_GENERATOR,
    ENEMY_GENERATOR2,
    ENEMY_SPINNER,
    ENEMY_CRAWLER,
    ENEMY_CRAWLER3,
    ENEMY_BOSS1,
    ENEMY_BOSS2,
    ENEMY_HARVESTER,
    ENEMY_LAUNCHER,
    ENEMY_SHOT,
    ENEMY_EVIL_LAUNCHER,
    ENEMY_EVIL_SHOT,
    ENEMY_SHURIKEN,
    ENEMY_GEN_GRUNT,
    ENEMY_SPREADER
}enemy_type;

typedef enum bonus_type{
    BONUS_SCORE,
    BONUS_TRIPLE,
    BONUS_LIFE,
    BONUS_INVINCIBLE
}bonus_type;

typedef enum game_phase{
    PHASE_SETUP,
    PHASE_WARP,
    PHASE_SPAWN,
    PHASE_PLAY,
    PHASE_DIE,
    PHASE_COMPLETE,
    PHASE_END
}game_phase;



//structs
typedef struct blob{
    float x,y;
    float sx,sy;
    float deg;
    float rad;
    float spd;
    int r,g,b;
    float alpha;
}blob;



typedef struct cg_audio_track{
    MIX_Track *track;
    MIX_Audio *audio;
    cg_audio_type type;
    float vol;
}cg_audio_track;

typedef struct cued_audio{
    cg_audio_track *track;
    float vol;
    SDL_PropertiesID props;
    int delay;
    float pan;
}cued_audio;

typedef struct high_score_entry{
    char name[32];
    Uint64 score;
}high_score_entry;

typedef struct player{
    float x,y;
    float xvel,yvel;
    float ang;
    float colRad;
    bool active;
    Uint64 score;
    int multiplier;
    int streak;
    int lives;
    int threewayTimer;
    bool horse;
}player;

typedef struct particle{
    float x,y;
    float xvel,yvel;
    float xvelmod,yvelmod;
    float ang;
    float angVel;
    float sz;
    float szvel;
    float alpha;
    float alphaVel;
    int life;
    int r,g,b;
    cg2d_image *img;
    int delay;
    int (* die)(struct particle*);
}particle;

typedef struct player_bullet{
    float x,y;
    float xvel,yvel;
    float ang;
    float colRad;
    int energy;
    int r,g,b;
}player_bullet;

typedef struct enemy{
    enemy_type type;
    float x,y;
    float x2,y2;
    float ang;
    float xvel,yvel;
    float targetAng,targetSpd;
    float txvel,tyvel;
    int timer1;
    cg2d_image *img[8];
    int imgCnt;
    float hndlx,hndly;
    int currentFrame,frameDelay;
    int (* update)(struct enemy*);
    int (* add_shots)(struct enemy*);
    int (* die)(struct enemy*);
    int (* add_effects)(struct enemy*);
    float colRad;
    float scale;//for drawing
    float angOffset;
    int energy;
    int score;
    int shotRate,shotSpeed;
    float spinSpeed;
    float knockbackScale;
    float knockbackX,knockbackY;
    int bossPhase;
}enemy;

typedef struct bonus{
    float x,y;
    float ang;
    bonus_type type;
}bonus;

typedef struct message{
    float x,y;
    float alpha;
    float alphaMod;
    int life;
    float scl;
    float sclMod;
    char str[128];
}message;


//global variables

//stretchy buffers
message *messages=NULL;
bonus *bonuses=NULL;
enemy *enemies=NULL;
player_bullet *player_bullets=NULL;
particle *particles=NULL;//stretchy buffer - remenber to free!
blob *blobs=NULL;//stretchy buffer for blobs. see stb_ds.h for info
high_score_entry *highScoreTable=NULL;
cued_audio *cued_SFX=NULL;

//for score entry.c
Uint64 entryScore;
char dispString[32];

//for menu.c
int menuPosition=0;
char *menuOptions[5]={"New Game","Scores","About","Settings","Quit"};

//for settings.c
int settingsPosition=0;
char fullscreenMessage[64];
char musicMessage[64];
char sfxMessage[64];
char backMessage[64];
char *settingsOptions[4];

//for pause.c
int pausePosition=0;
char pause_resumeMessage[64];
char pause_fullscreenMessage[64];
char pause_SFXMessage[64];
char pause_MusicMessage[64];
char pause_quitMessage[64];
char *pauseOptions[5];

//game variables - mostly used in game.c
int waveTicker=0;
int waveResetTime=0;
int waveNumber=0;
game_phase phase=0;
float camOffsetX=0.0;
float camOffsetY=0.0;
bool levelEndCondition=false;
bool gameComplete=false;
int tripleCount=0;
int bonusCount=0;
int lifeCount=0;
int killCount=0;
int deathTicker=0;
int spawnTicker=0;
int onDeathEnemyKillPerTick=0;

//player
player p1;

//score table


//audio volumes
float sfxGain=1.0f;
float musicGain=1.0f;

//current audio tracks
cg_audio_track *_currentMusic=NULL;
cg_audio_track *_audioList=NULL;

//blobs for menu background
Uint32 BLOB_COUNT=1000;
