#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include <SDL3/SDL.h>
#include <assert.h>

#define SAMPLE_BUFFER_SIZE 1024
#define SAMPLE_RATE 44100
#define MINIMUN_AUDIO (SAMPLE_BUFFER_SIZE * sizeof(float))
#define NOTES_CAPACITY 1024
#define NEXT_SEMI_TONE 1.0594631f // 2^(1/12)
#define ROOT_NOTE 440

//where are in the sine wave
float SampleBuffer[SAMPLE_BUFFER_SIZE] = {};


typedef struct {
    bool Playing;
    float Frequency;
    SDL_Scancode NoteKey; // note key on keyboard
    uint32_t FrameCount; // where are in the sine wave
} note;


typedef struct {
    note NoteSet[NOTES_CAPACITY];
    uint32_t Count;
} notes;
// array of notes
notes NotesBuffer = {};

typedef enum {
    INT,
    FLOAT,
    UNKNOWN
} type_info;

typedef union {
    int32_t AsI32;
    float   AsF32;
} type;


void Clamp(void* Data, type Low, type High, type_info Info)
{
    if(Info == INT) {
        if(*((int32_t*)Data) < Low.AsI32) {
            *((int32_t*)Data) = Low.AsI32;
        }

        if(*((int32_t*)Data) > High.AsI32) {
            *((int32_t*)Data) = High.AsI32;
        }

    } else if(Info == FLOAT) {

        if(*((float*)Data) < Low.AsF32) {
            *((float*)Data) = Low.AsF32;
        }

        if(*((float*)Data) > High.AsF32) {
            *((float*)Data) = High.AsF32;
        }
    } else {
        printf("Unknown type");
    }

}
void NoteUpdate(note* Note, float* Buffer,float Amplitude, uint32_t BufferSize)
{
    if(!Note->Playing) {
        return;
    }
    for (uint32_t Index = 0; Index < BufferSize; Index++) {

        float Time = (float)Note->FrameCount/(float)SAMPLE_RATE;
        Buffer[Index] += Amplitude * SDL_sinf(2 * SDL_PI_F * Time * Note->Frequency);
        Note->FrameCount++;
    }
}

float GetFreqFromSemiTone(float Semitone)
{
    return(ROOT_NOTE * pow(NEXT_SEMI_TONE, Semitone));
}

note GetNote(float Semitone)
{
    return (note) {
        .Frequency = GetFreqFromSemiTone(Semitone)
    };
}

typedef note (*GetNoteCallback) (float Semitone);

void AddNote(notes* Notes, GetNoteCallback Notefun, float Semitone,SDL_Scancode NoteKey)
{
    assert(Notes->Count < NOTES_CAPACITY);
    Notes->NoteSet[Notes->Count] = Notefun(Semitone);
    Notes->NoteSet[Notes->Count].NoteKey = NoteKey;
    Notes->Count++;

}

void KeyBoardNotes(notes* Notes)
{
    AddNote(Notes, GetNote, 0, SDL_SCANCODE_Z);
    AddNote(Notes, GetNote, 1, SDL_SCANCODE_S);
    AddNote(Notes, GetNote, 2, SDL_SCANCODE_X);
    AddNote(Notes, GetNote, 3, SDL_SCANCODE_D);
    AddNote(Notes, GetNote, 4, SDL_SCANCODE_C);
    AddNote(Notes, GetNote, 5, SDL_SCANCODE_V);
    AddNote(Notes, GetNote, 6, SDL_SCANCODE_G);
    AddNote(Notes, GetNote, 7, SDL_SCANCODE_B);
    AddNote(Notes, GetNote, 8, SDL_SCANCODE_H);
    AddNote(Notes, GetNote, 9, SDL_SCANCODE_N);
    AddNote(Notes, GetNote, 10, SDL_SCANCODE_J);
    AddNote(Notes, GetNote, 11, SDL_SCANCODE_M);
    AddNote(Notes, GetNote, 12, SDL_SCANCODE_COMMA);
}

int main()
{
    srand(time(0));
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_Window* Window =  SDL_CreateWindow("DiMooper", 800, 600, SDL_WINDOW_OPENGL);
    SDL_Renderer* Renderer = SDL_CreateRenderer(Window, NULL);

    SDL_AudioSpec Spec;
    Spec.channels = 1;
    Spec.format = SDL_AUDIO_F32;
    Spec.freq = 44100;
    SDL_AudioStream *Stream  = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &Spec, NULL, NULL);
    SDL_ResumeAudioStreamDevice(Stream);

    SDL_Event event;
    // adding notes to notes struct;

    KeyBoardNotes(&NotesBuffer);



//    AddNote(&NotesBuffer, GetNote, 7);

    type_info ClampType = FLOAT;
    // low
    type Low = {.AsF32 = -1.0f};
    // high
    type High = {.AsF32 = 1.0f};

    uint32_t Freq = 440;
    while (event.type!=SDL_EVENT_QUIT) {
        SDL_PollEvent(&event);
        const bool *KeyBoardState = SDL_GetKeyboardState(NULL);
        uint32_t NotesPlaying = 0;
        for(uint32_t Key = 0; Key < NotesBuffer.Count; Key++) {
            
            if(KeyBoardState[NotesBuffer.NoteSet[Key].NoteKey]) {
                NotesBuffer.NoteSet[Key].Playing = true;
                NotesPlaying += 1;
            }
            if(!KeyBoardState[NotesBuffer.NoteSet[Key].NoteKey]) {
                NotesBuffer.NoteSet[Key].Playing = false;
            }
        }

        if(SDL_GetAudioStreamQueued(Stream) < MINIMUN_AUDIO) {
            if(NotesPlaying) {
                for(int32_t Y = 0; Y <NotesBuffer.Count; Y++) {
                    NoteUpdate(&NotesBuffer.NoteSet[Y], SampleBuffer, 1.0f/NotesPlaying, SAMPLE_BUFFER_SIZE);
                    for(int32_t Index2 = 0; Index2 < SAMPLE_BUFFER_SIZE; Index2++) {
                        Clamp(&SampleBuffer[Index2], Low, High, FLOAT);
                    }
                }
                SDL_PutAudioStreamData(Stream, SampleBuffer, (sizeof(float) * SAMPLE_BUFFER_SIZE));
                // cleaning the buffer
                for(int32_t Index2 = 0; Index2 < SAMPLE_BUFFER_SIZE; Index2++) {
                    SampleBuffer[Index2] = 0;
                }
            }
        }

        SDL_SetRenderDrawColorFloat(Renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(Renderer);
        SDL_RenderPresent(Renderer);
    }
    SDL_DestroyWindow(Window);
    SDL_Quit();
    return(0);
}
