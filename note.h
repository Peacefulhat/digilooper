#ifndef NOTE_H
#define NOTE_H

#include <stdio.h>
#include <SDL3/SDL.h>
#include <math.h>
#include <assert.h>

#define  global_variable static
#define  local_persist   static
#define  internal        static

#define SAMPLE_BUFFER_SIZE 1024
#define SAMPLE_RATE 44100
#define MINIMUN_AUDIO (SAMPLE_BUFFER_SIZE * sizeof(float))
#define KEYBOARD_NOTES_CAPACITY 15
#define NEXT_SEMI_TONE 1.0594631f // 2^(1/12)
#define ROOT_NOTE 440

// note key on keyboard and its playing state
typedef struct {
    bool Playing;
    uint32_t Frequency;
    SDL_Scancode Key;
} note;

 // store state of a added notes
typedef struct {
    note NoteKeys[KEYBOARD_NOTES_CAPACITY];
    uint32_t Count;
} notes;

typedef enum {
    INT,
    FLOAT,
    UNKNOWN
} type_info;

typedef union {
    int32_t AsI32;
    float   AsF32;
} type;

typedef note (*note_callback)(float Semitone);

void NoteUpdate(note* Note, float* Buffer, uint32_t FrameCount, float Amplitude, uint32_t BufferSize);
float GetFreqFromSemiTone(float Semitone);
void CleanBuffer(float* Buffer, uint32_t BufferSize);
void AddNote(notes* Notes, note_callback Notefn, float Semitone, SDL_Scancode Key);
void KeyBoardNotes(notes* Notes);
void Clamp(void* Data, type Low, type High, type_info Info);

#endif
