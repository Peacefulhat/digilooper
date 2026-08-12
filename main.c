#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include <SDL3/SDL.h>
#include <assert.h>
#include "note.h"

global_variable float Samples[SAMPLE_BUFFER_SIZE] = {}; // buffer where we write our data before playing
global_variable notes KeyboardNotes = {};               // Buffer to store available keyboard notes
global_variable uint32_t FrameCount = 0;

int main()
{
    srand(time(0));
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_Window* Window     =  SDL_CreateWindow("Digilooper", 800, 600, SDL_WINDOW_OPENGL);
    SDL_Renderer* Renderer = SDL_CreateRenderer(Window, NULL);

    SDL_AudioSpec Spec;
    Spec.channels          = 1;
    Spec.format            = SDL_AUDIO_F32;
    Spec.freq              = 44100;

    SDL_AudioStream *Stream  = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &Spec, NULL, NULL);
    SDL_ResumeAudioStreamDevice(Stream);
    SDL_Event event;


    type_info ClampType = FLOAT;
    type Low            = {.AsF32 = -1.0f};
    type High           = {.AsF32 =  1.0f};
    KeyBoardNotes(&KeyboardNotes);

    while (event.type!=SDL_EVENT_QUIT) {
        SDL_PollEvent(&event);
        const bool *KeyBoardState = SDL_GetKeyboardState(NULL);
        uint32_t NotesPlaying = 0;
        for(uint32_t Index = 0;  Index < KeyboardNotes.Count; Index++) {

            if(KeyBoardState[KeyboardNotes.NoteKeys[Index].Key]) {
                KeyboardNotes.NoteKeys[Index].Playing = true;
                NotesPlaying += 1;
            }
            if(!KeyBoardState[KeyboardNotes.NoteKeys[Index].Key]) {
                KeyboardNotes.NoteKeys[Index].Playing = false;
            }
        }

        if(SDL_GetAudioStreamQueued(Stream) < MINIMUN_AUDIO) {
            if(NotesPlaying) {
                for(int32_t Y = 0; Y <KeyboardNotes.Count; Y++) {
                    if(KeyboardNotes.NoteKeys[Y].Playing) {
                        NoteUpdate(&KeyboardNotes.NoteKeys[Y],Samples, FrameCount,
                                   1.0f/NotesPlaying, SAMPLE_BUFFER_SIZE);
                    }
                    for(int32_t X = 0; X < SAMPLE_BUFFER_SIZE; X++) {
                        Clamp(&Samples[X], Low, High, FLOAT);
                    }
                }

                FrameCount += SAMPLE_BUFFER_SIZE; // updating the frame count
                SDL_PutAudioStreamData(Stream, Samples,(sizeof(float) * SAMPLE_BUFFER_SIZE));
                CleanBuffer(Samples, SAMPLE_BUFFER_SIZE);
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
