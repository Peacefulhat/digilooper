#include "note.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define SAMPLE_SIZE 32
#define CHANNELS    1

global_variable float Samples[SAMPLE_BUFFER_SIZE] = {}; // buffer where we write our data before playing
global_variable notes KeyboardNotes = {};               // Buffer to store available keyboard notes
global_variable uint32_t FrameCount = 0;

#define BEATS_PER_MIN  120 // Beats per minute
#define BAR_BEATS      4
#define BAR_QUANT      32
#define BEAT_SECS      60.0f/BEATS_PER_MIN  // amount of beast in milliseoncds
#define BAR_SECS       (BAR_BEATS * BEAT_SECS)
#define QUANT_SECS     (BAR_SECS / BAR_QUANT)

typedef float quant;

typedef struct {
    KeyboardKey Key;
    bool Keystate;
    float Semitone;
    quant TimeStamp;
} event;

typedef enum {
    REPLAY,
    WAIT_UNTIL_END_OF_BAR,
    RECORD,

} state; // Recording states


Color BgColor = {0x18, 0x18, 0x18, 255};

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    //    SetConfigFlags(FLAG_MSAA_4X_HINT); // enable antialiasing ?
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Digilooper");
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(SAMPLE_BUFFER_SIZE);
    AudioStream Stream = LoadAudioStream(SAMPLE_RATE, SAMPLE_SIZE, CHANNELS);
    PlayAudioStream(Stream);
    SetTargetFPS(60);

    float BeatTime = 0.0;
    type_info ClampType = FLOAT;
    type Low            = {.AsF32 = -1.0f};
    type High           = {.AsF32 =  1.0f};

    bool Recording = false;
    KeyBoardNotes(&KeyboardNotes);
    Sound Bomb = LoadSound("plant-bomb.wav");
    state State = {};
    while (!WindowShouldClose()) {
        float Quant = (GetFrameTime()/(float)QUANT_SECS);
        float A = fmodf(BeatTime, BEAT_SECS);
        BeatTime += GetFrameTime();
        float B = fmodf(BeatTime, BEAT_SECS);
        if(A > B) {
            PlaySound(Bomb);
        }
        uint32_t NotesPlaying = 0;

        for(uint32_t Index = 0;  Index < KeyboardNotes.Count; Index++) {
            if(IsKeyDown(KeyboardNotes.NoteKeys[Index].Key)) {
                KeyboardNotes.NoteKeys[Index].Playing = true;
                NotesPlaying += 1;
            }
            if(!IsKeyDown(KeyboardNotes.NoteKeys[Index].Key)) {
                KeyboardNotes.NoteKeys[Index].Playing = false;
            }
        }


        if(IsKeyPressed(KEY_R)) {
            switch(State) {
            case REPLAY: {
                State = WAIT_UNTIL_END_OF_BAR;
            }
            break;
            case WAIT_UNTIL_END_OF_BAR: {
                State = REPLAY;
            }
            break;

            case RECORD: {
                State = REPLAY;
            }
            break;
            }
        }

        if(IsAudioStreamProcessed(Stream)) {
            if(NotesPlaying) {
                for(int32_t Y = 0; Y <KeyboardNotes.Count; Y++) {
                    if(KeyboardNotes.NoteKeys[Y].Playing) {
                        NoteUpdate(&KeyboardNotes.NoteKeys[Y],Samples, FrameCount,
                                   1.0f/NotesPlaying, SAMPLE_BUFFER_SIZE);
                    }
                    for(int32_t X = 0; X < SAMPLE_BUFFER_SIZE; X++) {
                        Clamp2(&Samples[X], Low, High, FLOAT);
                    }
                }
                FrameCount += SAMPLE_BUFFER_SIZE; // updating the frame count
                UpdateAudioStream(Stream, Samples, SAMPLE_BUFFER_SIZE);
                CleanBuffer(Samples, SAMPLE_BUFFER_SIZE);
            }
        }
        BeginDrawing();
        ClearBackground(BgColor);
        Vector2 Center = {(GetScreenWidth() - 30.0f), 30.0f};
        float Radius = 20.0f;
        Color CircleColor = RED;
        switch(State) {
        case REPLAY: {

            DrawRing(Center, Radius * 0.85, Radius, 0, 360, 30, WHITE);
        }
        break;
        case WAIT_UNTIL_END_OF_BAR: {
            DrawCircleV( Center, Radius, BLUE);

        }
        break;

        case RECORD: {

            DrawCircleV( Center, Radius, RED);

        }
        break;
    }

    
        for (uint32_t Y = 0; Y < BAR_BEATS; ++Y) {
            float BeatLength = (float)GetScreenWidth() / (float)BAR_BEATS;
            Vector2 StartPos = {(Y * BeatLength), 0.0f};
            Vector2 EndPos   = {(Y * BeatLength), GetScreenHeight()};
            Color LineColor  = WHITE;
            DrawLineV(StartPos, EndPos, LineColor);
        }
        // moving line
        float MLineX = fmodf(BeatTime, BAR_SECS)/BAR_SECS * GetScreenWidth();
        Vector2 MLineStartPos = { MLineX, 0.0f };
        Vector2 MLineEndPos   = {MLineX, GetScreenHeight()};

        DrawLineV(MLineStartPos,MLineEndPos, GRAY);
        EndDrawing();
    }
    UnloadAudioStream(Stream);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
