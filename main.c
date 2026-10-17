#define DEBUG_DA
#include "note.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define SAMPLE_SIZE 32
#define CHANNELS    1

global_variable float64 Samples[SAMPLE_BUFFER_SIZE] = {};
global_variable notes KeyBoardNotes = {};
global_variable notes NotesReplay = {};
global_variable NoteVec NotesReleased = {};
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
    instrument* Instrument;
} event;

typedef struct {
    quant TimeStamp;
    int32_t Frequency;
    bool Start;
    float32 Lifetime;
} dissipating_ring;

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
    type_info ClampType = FLOAT64;
    type Low            = {.AsF32 = -1.0f};
    type High           = {.AsF32 =  1.0f};


    // Events Buffer
    events Events = {};
    Events.EventsArr = (event*)malloc(sizeof(event)* EVENT_BUFFER_CAPACITY);

    // Adding KeyBoard notes
    AddKeyBoardNotes(&KeyBoardNotes);
    state State = {};

    // Init Capacity NotesReleased
    InitVector(&NotesReleased, 2);
    Sound Bomb = LoadSound("plant-bomb.wav");
    while (!WindowShouldClose()) {
        float32 Quant = (BeatTime/QUANT_SECS);
        float32 BeatTimePrev = BeatTime;
        BeatTime += GetFrameTime();

        if(fmodf(BeatTime, BEAT_SECS) < fmodf(BeatTimePrev, BEAT_SECS)) {
            PlaySound(Bomb);
        }
        if(State == REPLAY) {

            if(Events.Count > 0) {

                uint32_t AmountOfBarsInRecording = (float32)(Events.EventsArr[Events.Count - 1].TimeStamp + BAR_QUANT - 1)/BAR_QUANT;
                uint32_t QuantToPlay = (float32)fmodf(Quant, (AmountOfBarsInRecording * BAR_QUANT));

                for(int i =0; i < Events.Count; ++i) {
                    if(Events.EventsArr[i].TimeStamp ==  QuantToPlay) {
                        if(Events.EventsArr[i].Start) {
                            NotePress(&NotesReplay.NoteKeys[Events.EventsArr[i].Semitonei]);
                        } else {
                            NoteRelease(&NotesReplay.NoteKeys[Events.EventsArr[i].Semitonei], &NotesReleased, FrameCount);
                        }
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
                        event Event = {.TimeStamp = Quant, .Start = true, .Semitonei = KeyBoardNotes.NoteKeys[I].Frequency,
                        .Instrument = KeyBoardNotes.NoteKeys[I].Instrument
                    };
                        AddEvent(&Events, Event);
                    }
                }
            }
        } else if (State == RECORD) {
        }



        if(IsKeyPressed(KEY_R)) {
            if(State == REPLAY) {
                State = WAIT_UNTIL_END_OF_BAR;
                Events.Count = 0; // resetting the event counts
                for(uint32_t NoteIndex = 0; NoteIndex < NotesReplay.Count; NoteIndex++) {
                    NoteRelease(&NotesReplay.NoteKeys[NoteIndex], &NotesReleased, FrameCount);
                }

            } else if(State == RECORD) {
                // PrintEvents(&Events);
                State = REPLAY;
            } else if( State == WAIT_UNTIL_END_OF_BAR) {
                State = REPLAY;
            }
        }



        for(uint32_t NoteIndex = 0;  NoteIndex < KeyBoardNotes.Count; NoteIndex++) {
            if(IsKeyDown(KeyBoardNotes.NoteKeys[NoteIndex].Key)) {
                if(!KeyBoardNotes.NoteKeys[NoteIndex].Playing) {
                    NotePress(&KeyBoardNotes.NoteKeys[NoteIndex]);

                    if(State == RECORD) {
                        event Event = {.TimeStamp = Quant, .Start = true, .Semitonei = NoteIndex};
                        AddEvent(&Events, Event);
                    }
                }
            }
            if(!IsKeyDown(KeyBoardNotes.NoteKeys[NoteIndex].Key) ) {
                if(KeyBoardNotes.NoteKeys[NoteIndex].Playing) {
                    NoteRelease(&KeyBoardNotes.NoteKeys[NoteIndex], &NotesReleased, FrameCount);
                    if(State == RECORD) {
                        event Event = {.TimeStamp = Quant, .Start = false, .Semitonei = KeyBoardNotes.NotesKey[NoteIndex].Frequency,
                    .Instrument = };
                        AddEvent(&Events, Event);
                    }
                }
            }

        }

        if(IsAudioStreamProcessed(Stream)) {

            for(int32_t SampleIndex = 0; SampleIndex < SAMPLE_BUFFER_SIZE; SampleIndex++) {
                // cleaning the current sample.
                Samples[SampleIndex] = 0.0f;

                uint32_t NotesPlaying = 0;
                // checking what notes are playing for current sample
                // Keyboard notes currently playing
                for(int32_t NoteIndex = 0; NoteIndex <KeyBoardNotes.Count; NoteIndex++) {
                    if(KeyBoardNotes.NoteKeys[NoteIndex].Playing) {
                        NotesPlaying += 1;
                    }
                }

                // replay notes currently playing
                for(int32_t NoteIndex = 0; NoteIndex <NotesReplay.Count; NoteIndex++) {
                    if(NotesReplay.NoteKeys[NoteIndex].Playing) {
                        NotesPlaying += 1;
                    }
                }

                if(NotesReleased.Count > 0) {
                    for(uint32_t NoteIndex = NotesReleased.Count; NoteIndex > 0; --NoteIndex) {
                        if(NoteReleaseDone(&NotesReleased.Data[NoteIndex])) {
                            UnorderedRemoveByIndex(&NotesReleased, NoteIndex);

                        } else {
                            // released notes currently playing
                            NotesPlaying += 1;
                        }
                    }
                }



                // if there are any keyboardnotes playing for sample, update the note and add it to sample
                if (NotesPlaying) {
                    float32 Amplitude = 1.0f/NotesPlaying;

                    for(int32_t NoteIndex = 0; NoteIndex <KeyBoardNotes.Count; NoteIndex++) {
                        if(KeyBoardNotes.NoteKeys[NoteIndex].Playing) {

                            Samples[SampleIndex] += NoteUpdate(&KeyBoardNotes.NoteKeys[NoteIndex]) * Amplitude;
                        }
                    }
                    // if there are any replaynote playing for sample, update the note and add it to sample
                    for(int32_t NoteIndex = 0; NoteIndex <NotesReplay.Count; NoteIndex++) {
                        if(NotesReplay.NoteKeys[NoteIndex].Playing) {
                            Samples[SampleIndex] += NoteUpdate(&NotesReplay.NoteKeys[NoteIndex]) * Amplitude;
                        }
                    }

                    for(int32_t NoteIndex = 0; NoteIndex <NotesReleased.Count; NoteIndex++) {
                        Samples[SampleIndex] += NoteReleasedUpdate(&NotesReleased.Data[NoteIndex]) * Amplitude;
                    }
                    // i don't see much of diffence for now not clap
                    // dont to fix clipping clamp the smaple
                    // Clamp2(&Samples[SampleIndex], Low, High, FLOAT64);
                    Clamp64(&Samples[SampleIndex], Low, High, FLOAT64);
                }

            }
            UpdateAudioStream(Stream, Samples, SAMPLE_BUFFER_SIZE);
        }

        BeginDrawing();
        ClearBackground(BgColor);
        Vector2 Center = {(GetScreenWidth() - 30.0f), 30.0f};
        float32 Radius = 20.0f;
        Color CircleColor = RED;

        if(State == REPLAY) {

            DrawRing(Center, Radius * 0.85, Radius, 0, 360, 30, WHITE);
        } else if(State == WAIT_UNTIL_END_OF_BAR) {

            DrawCircleV( Center, Radius, BLUE);
        } else if(State == RECORD) {

            DrawCircleV( Center, Radius, RED);
        }
        uint32_t AmountOfBarsInRecording  =1;

        if(Events.Count > 0 && Events.EventsArr[Events.Count - 1].TimeStamp > 0) {

            AmountOfBarsInRecording = (Events.EventsArr[Events.Count - 1].TimeStamp + BAR_QUANT - 1)/BAR_QUANT;

        }
        float32 QuantLength = (float32)GetScreenWidth() / (float32)(BAR_QUANT * AmountOfBarsInRecording);
        float32 SemitoneHeight = (float32)GetScreenHeight()/(float32)KEYBOARD_NOTES_CAPACITY;
        for(int i =0; i < Events.Count; ++i) {
            Vector2 Center = {Events.EventsArr[i].TimeStamp * QuantLength, Events.EventsArr[i].Semitonei * SemitoneHeight};
            if(Events.EventsArr[i].Start) {
                DrawCircleV(Center, 10, RED);
            } else {
                DrawCircleV(Center, 10, BLUE);
            }
        }

        float32 BeatLength = (float32)GetScreenWidth() / (float32)(BAR_BEATS * AmountOfBarsInRecording);
        for (uint32_t Y = 0; Y < BAR_BEATS * AmountOfBarsInRecording; ++Y) {

            Vector2 StartPos = {(Y * BeatLength), 0.0f};
            Vector2 EndPos   = {(Y * BeatLength), GetScreenHeight()};
            Color LineColor  = WHITE;
            DrawLineV(StartPos, EndPos, LineColor);
        }

        // moving line
        float32 MLineX =
            (fmodf(BeatTime, BAR_SECS * AmountOfBarsInRecording) / (float32)(BAR_SECS * AmountOfBarsInRecording) )* GetScreenWidth();
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

