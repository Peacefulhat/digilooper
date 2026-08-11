#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <SDL3/SDL.h>

typedef struct {
    float Red;
    float Green;
    float Blue;
    float Alpha;
} color;

#define SAMPLE_BUFFER_SIZE 1024
#define SAMPLE_RATE 44100
#define MINIMUN_AUDIO (SAMPLE_BUFFER_SIZE * sizeof(float))
uint32_t CurrentSineSample = 0;

float   SampleBuffer[SAMPLE_BUFFER_SIZE] = {};

void SineWave(SDL_AudioStream *Stream, float* Samples, uint32_t SampleSize, float Amplitude)
{
    int32_t Freq = 440;
    for (uint32_t Index = 0; Index < SampleSize; Index++) {

        float Time = (float)CurrentSineSample/(float)SAMPLE_RATE;
        Samples[Index] = Amplitude * SDL_sinf(2 * SDL_PI_F * Time * Freq);
        CurrentSineSample++;
    }
    CurrentSineSample %= SAMPLE_RATE;
    SDL_PutAudioStreamData(Stream, Samples, (sizeof(float) * SampleSize));
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
    Spec.freq =44100;
    SDL_AudioStream *Stream  = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &Spec, NULL, NULL);
    SDL_ResumeAudioStreamDevice(Stream);

    SDL_Event event;
    while (1) {
        SDL_PollEvent(&event);
        int32_t Playing = 0;
        const bool *KeyBoardState = SDL_GetKeyboardState(NULL);
        if (event.type == SDL_EVENT_QUIT) {
            break;
        }
        if(KeyBoardState[SDL_SCANCODE_Z]) {


            if(SDL_GetAudioStreamQueued(Stream) < MINIMUN_AUDIO) {
                SineWave(Stream, SampleBuffer, SAMPLE_BUFFER_SIZE, 0.5);
            }


        }
        color Color = {0x18, 0x18, 0x18, 0x1};
        SDL_SetRenderDrawColorFloat(Renderer, Color.Red, Color.Green, Color.Blue, Color.Alpha);
        SDL_RenderClear(Renderer);
        SDL_RenderPresent(Renderer);
    }
    SDL_DestroyWindow(Window);
    SDL_Quit();
    return(0);
}
