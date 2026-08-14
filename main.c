#include "note.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define SAMPLE_SIZE 32
#define CHANNELS    1

global_variable float Samples[SAMPLE_BUFFER_SIZE] = {};
global_variable notes KeyBoardNotes = {};
global_variable notes NotesReplay = {};
global_variable uint32_t FrameCount = 0;

#define EVENT_BUFFER_CAPACITY 500000
#define BEATS_PER_MIN  120 // Beats per minute
#define BAR_BEATS      4
#define BAR_QUANT      32
#define BEAT_SECS      (60.0f / (float)BEATS_PER_MIN)  // amount of beast in milliseoncds
#define BAR_SECS       (BAR_BEATS * BEAT_SECS)
#define QUANT_SECS     (BAR_SECS / BAR_QUANT)

typedef uint32_t quant;

typedef struct {
    quant TimeStamp;
    bool Start;
    uint32_t Semitonei; // semitone index;
} event;

typedef struct {
    event* EventsArr;
    uint32_t Count;
} events;

void PrintEvents(events* Events)
{
    if(!Events) {
        //logging
        return;
    }
    printf("Events::\n");
    for(uint32_t Index = 0; Index < Events->Count; ++Index) {
        printf("{Timestamp: %d, Start: %d, Semitone: %d}\n",
               Events->EventsArr[Index].TimeStamp,
               Events->EventsArr[Index].Start,
               Events->EventsArr[Index].Semitonei);
    }
}

void AddEvent(events* Events, event Event)
{
    if(!Events) {
        //logging
        return;
    }
    assert(Events->Count < EVENT_BUFFER_CAPACITY);
    Events->EventsArr[Events->Count++] = Event;
};

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

    // Events Buffer
    events Events = {};
    Events.EventsArr = (event*)malloc(sizeof(event)* EVENT_BUFFER_CAPACITY);

    // Adding KeyBoard notes
    AddKeyBoardNotes(&KeyBoardNotes);
    state State = {};

    Sound Bomb = LoadSound("plant-bomb.wav");
    while (!WindowShouldClose()) {
        float Quant = (BeatTime/QUANT_SECS);
        float BeatTimePrev = BeatTime;
        BeatTime += GetFrameTime();

        if(fmodf(BeatTime, BEAT_SECS) < fmodf(BeatTimePrev, BEAT_SECS)) {
            PlaySound(Bomb);
        }
        if(State == REPLAY) {

            if(Events.Count > 0) {

                uint32_t AmountOfBarsInRecording = (float)(Events.EventsArr[Events.Count - 1].TimeStamp + BAR_QUANT - 1)/BAR_QUANT;
                uint32_t QuantToPlay = (float)fmodf(Quant, (AmountOfBarsInRecording * BAR_QUANT));

                for(int i =0; i < Events.Count; ++i) {
                    if(Events.EventsArr[i].TimeStamp ==  QuantToPlay) {
                        NotesReplay.NoteKeys[Events.EventsArr[i].Semitonei].Playing = Events.EventsArr[i].Start;
                    }
                }
            }
        } else if(State == WAIT_UNTIL_END_OF_BAR) {
            if(fmodf(BeatTime, BAR_SECS) <  fmodf(BeatTimePrev, BAR_SECS)) {
                State = RECORD;
                BeatTime = 0;

                Quant = 0;
                for(uint32_t I= 0; I < KeyBoardNotes.Count; ++I ) {
                    if(KeyBoardNotes.NoteKeys[I].Playing) {
                        event Event = {.TimeStamp = Quant, .Start = true, .Semitonei = I};
                        AddEvent(&Events, Event);
                    }
                }
            }
        } else if (State == RECORD) {
        }



        if(IsKeyPressed(KEY_R)) {
            if(State == REPLAY) {
                State = WAIT_UNTIL_END_OF_BAR;


                for(uint32_t Index = 0; Index < NotesReplay.Count; Index++) {
                    NotesReplay.NoteKeys[Index].Playing = false;
                }
                Events.Count = 0; // resetting the event counts
            } else if(State == RECORD) {
//                PrintEvents(&Events);
                State = REPLAY;
            } else if( State == WAIT_UNTIL_END_OF_BAR) {
                Events.Count = 0;
                State = REPLAY;
            }
        }

        uint32_t NotesPlaying = 0;

        for(uint32_t Index = 0;  Index < KeyBoardNotes.Count; Index++) {
            if(IsKeyDown(KeyBoardNotes.NoteKeys[Index].Key)) {
                if(!KeyBoardNotes.NoteKeys[Index].Playing) {
                    KeyBoardNotes.NoteKeys[Index].Playing = true;

                    if(State == RECORD) {
                        event Event = {.TimeStamp = Quant, .Start = true, .Semitonei = Index};
                        AddEvent(&Events, Event);
                    }
                }
            }
            if(!IsKeyDown(KeyBoardNotes.NoteKeys[Index].Key) ) {
                if(KeyBoardNotes.NoteKeys[Index].Playing) {
                    KeyBoardNotes.NoteKeys[Index].Playing = false;
                    if(State == RECORD) {
                        event Event = {.TimeStamp = Quant, .Start = false, .Semitonei = Index};
                        AddEvent(&Events, Event);
                    }
                }
            }

        }

        if(IsAudioStreamProcessed(Stream)) {


            NotesPlaying = 0;
            for(int32_t Y = 0; Y <KeyBoardNotes.Count; Y++) {
                if(KeyBoardNotes.NoteKeys[Y].Playing || NotesReplay.NoteKeys[Y].Playing) {
                    NotesPlaying += 1;
                }
            }

            if(NotesPlaying) {
                for(int32_t Y = 0; Y <KeyBoardNotes.Count; Y++) {

                    if(KeyBoardNotes.NoteKeys[Y].Playing || NotesReplay.NoteKeys[Y].Playing) {

                        NoteUpdate(&KeyBoardNotes.NoteKeys[Y],Samples, FrameCount,
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

        if(State == REPLAY) {

            DrawRing(Center, Radius * 0.85, Radius, 0, 360, 30, WHITE);
        } else if(State == WAIT_UNTIL_END_OF_BAR) {

            DrawCircleV( Center, Radius, BLUE);
        } else if(State == RECORD) {

            DrawCircleV( Center, Radius, RED);
        }


        for (uint32_t Y = 0; Y < BAR_BEATS; ++Y) {
            float BeatLength = (float)GetScreenWidth() / (float)BAR_BEATS;
            Vector2 StartPos = {(Y * BeatLength), 0.0f};
            Vector2 EndPos   = {(Y * BeatLength), GetScreenHeight()};
            Color LineColor  = WHITE;
            DrawLineV(StartPos, EndPos, LineColor);
        }

        // moving line
        float MLineX = (float)(fmodf(BeatTime, BAR_SECS) / BAR_SECS) * GetScreenWidth();
        Vector2 MLineStartPos = { MLineX, 0.0f };
        Vector2 MLineEndPos   = {MLineX, GetScreenHeight()};

        DrawLineV(MLineStartPos,MLineEndPos, GRAY);
        EndDrawing();
    }
    UnloadAudioStream(Stream);
    free(Events.EventsArr);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

