
typedef struct blob{
    float x,y;
    float sx,sy;
    float deg;
    float rad;
    float spd;
    int r,g,b;
    float alpha;
}blob;

//blobs for menu background
static Uint32 BLOB_COUNT=1000;
static blob *blobs=NULL;//stretchy buffer for blobs. see stb_ds.h for info

typedef enum cg_audio_type{
    AUDIO_TYPE_MUSIC,
    AUDIO_TYPE_SFX
}cg_audio_type;

typedef struct cg_audio_track{
    MIX_Track *track;
    MIX_Audio *audio;
    cg_audio_type type;
    float vol;
}cg_audio_track;

cg_audio_track *_currentMusic=NULL;
cg_audio_track *_audioList=NULL;

typedef struct high_score_entry{
    char name[32];
    Uint64 score;
}high_score_entry;

high_score_entry *highScoreTable=NULL;

int SDLCALL high_score_sort(const void *a, const void *b)
{
    const high_score_entry *A = (const high_score_entry *)a;
    const high_score_entry *B = (const high_score_entry *)b;

    if (A->score < B->score) {
        return 1;
    } else if (B->score < A->score) {
        return -1;
    } else {
        return 0;
    }
}

float sfxGain=1.0f;
float musicGain=1.0f;

typedef struct player{
    float x,y;
    float xvel,yvel;
    float ang;
    float colRad;
    bool active;
    Uint64 score;
    int multiplier;
    int nextMultiplier;
    int lives;
    bool hasLasers;
}player;

player p1;

typedef struct particle{
    float x,y;
    float xvel,yvel;
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
}particle;

particle *particles=NULL;//stretchy buffer - remenber to free!

typedef struct player_bullet{
    float x,y;
    float xvel,yvel;
    float ang;
    float colRad;
    int energy;
}player_bullet;

player_bullet *player_bullets=NULL;

typedef enum enemy_type{
    ENEMY_GRUNT,
    ENEMY_TANK,
    ENEMY_INV,
    ENEMY_MINI,
    ENEMY_RING,
    ENEMY_ORBIT,
    ENEMY_ELECTRODE,
    ENEMY_MINE,
    ENEMY_THREEWAY,
    ENEMY_FOURWAY,
    ENEMY_RING3,
    ENEMY_STRIGHT,
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
    ENEMY_SHURIKEN,
    ENEMY_GEN_GRUNT
}enemy_type;

typedef struct enemy{
    enemy_type type;
    float x,y;
    float x2,y2;
    float ang;
    float xvel,yvel;
    int timer1,timer2;
    cg2d_image *img;
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
}enemy;

enemy *enemies=NULL;
    

