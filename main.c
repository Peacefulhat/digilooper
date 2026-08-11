#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include <SDL3/SDL.h>


#define SAMPLE_BUFFER_SIZE 1024
#define SAMPLE_RATE 44100
#define MINIMUN_AUDIO (SAMPLE_BUFFER_SIZE * sizeof(float))
#define NEXT_SEMI_TONE 1.0594631f // 2^(1/12)
#define ROOT_NOTE 440
uint32_t CurrentSineSample = 0;
//where are in the sine wave
float SampleBuffer[SAMPLE_BUFFER_SIZE] = {};

typedef struct {
    float Frequency;
    uint32_t FrameCount; // where are in the sine wave
}note;

void NoteUpdate(note* Note, float* Buffer, uint32_t BufferSize){
    
}

float GetFreqFromSemiTone(float Semitone)
{
    return(ROOT_NOTE * pow(NEXT_SEMI_TONE, Semitone));
}

void SineWave(SDL_AudioStream *Stream, float* Samples, uint32_t SampleSize, uint32_t Freq)
{
    if(SDL_GetAudioStreamQueued(Stream) < MINIMUN_AUDIO) {
        for (uint32_t Index = 0; Index < SampleSize; Index++) {

            float Time = (float)CurrentSineSample/(float)SAMPLE_RATE;
            Samples[Index] = 1.0f * SDL_sinf(2 * SDL_PI_F * Time * Freq);
            CurrentSineSample++;
        }
        CurrentSineSample %= SAMPLE_RATE;
        SDL_PutAudioStreamData(Stream, Samples, (sizeof(float) * SampleSize));
    }
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
    uint32_t Freq = 440;
    while (event.type!=SDL_EVENT_QUIT) {
        SDL_PollEvent(&event);
        const bool *KeyBoardState = SDL_GetKeyboardState(NULL);

        if(KeyBoardState[SDL_SCANCODE_Z]) {
            Freq = GetFreqFromSemiTone(0);
        } else  if(KeyBoardState[SDL_SCANCODE_S]) {
            Freq = GetFreqFromSemiTone(1);
        } else if(KeyBoardState[SDL_SCANCODE_X]) {
            Freq = GetFreqFromSemiTone(2);
        } else if(KeyBoardState[SDL_SCANCODE_D]) {
            Freq = GetFreqFromSemiTone(3);
        } else {
            Freq = 0;
        }
        SineWave(Stream, SampleBuffer, SAMPLE_BUFFER_SIZE, Freq);
        SDL_SetRenderDrawColorFloat(Renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(Renderer);
        SDL_RenderPresent(Renderer);
    }
    SDL_DestroyWindow(Window);
    SDL_Quit();
    return(0);
}
