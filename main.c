#include "note.h"

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

int main(void)
{
    InitWindow(800, 450, "raylib example - basic window");
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(SAMPLE_BUFFER_SIZE);
    AudioStream Stream = LoadAudioStream(SAMPLE_RATE, SAMPLE_SIZE, CHANNELS);
    PlayAudioStream(Stream);
    SetTargetFPS(60);

    float BeatTime = 0.0;
    type_info ClampType = FLOAT;
    type Low            = {.AsF32 = -1.0f};
    type High           = {.AsF32 =  1.0f};
    KeyBoardNotes(&KeyboardNotes);

    while (!WindowShouldClose()) {
        float Quant = (GetFrameTime()/(float)QUANT_SECS);
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
        ClearBackground(BLACK);
        EndDrawing();
    }
    UnloadAudioStream(Stream);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}


