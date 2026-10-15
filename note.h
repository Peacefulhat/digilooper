#ifndef NOTE_H
#define NOTE_H
/*TODO: for now printf() is used for logging thing, do something better*/
#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdint.h>

#define  global_variable static
#define  local_persist   static
#define  internal        static


#define SAMPLE_BUFFER_SIZE 2048
#define SAMPLE_RATE 44100
#define MINIMUN_AUDIO (SAMPLE_BUFFER_SIZE * sizeof(float))
#define KEYBOARD_NOTES_CAPACITY 15
#define NEXT_SEMI_TONE 1.0594631f // 2^(1/12)
#define ROOT_NOTE 440
#define ATTACK_FRAME 10000
#define RELEASE_FRAME 10000


// note key on keyboard and its playing state
typedef struct {
    bool Playing;
    int32_t FrameStamp; // current global fram count at which it started playing.
    float Frequency;
    KeyboardKey Key;
} note;

typedef struct {
    int32_t FrameStamp; // current global fram count at which it started playing.
    float Frequency;
    KeyboardKey Key;
    float Volume;
} note_released;


typedef struct {
    note_released* Data;
    uint32_t Count;
    uint32_t Capacity;
} NoteVec;

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
bool NoteReleaseDone(note_released* Note, uint32_t FrameCount);
float NoteReleasedUpdate(note_released* Note, uint32_t FrameCount);
float NoteUpdate(note* Note, uint32_t FrameCount);
void NotePress(note* Note, uint32_t FrameCount);
void NoteRelease(note* Note, NoteVec* Notes, uint32_t FrameCount);
float GetFreqFromSemiTone(float Semitone);
void CleanBuffer(float* Buffer, uint32_t BufferSize);
void AddNote(notes* Notes, note_callback Notefn, float Semitone, KeyboardKey Key);
void AddKeyBoardNotes(notes* Notes);
void Clamp2(void* Data, type Low, type High, type_info Info);


// Debug Dynamic Array

#define DA_INIT_CAP 64


void InitVector(NoteVec* Notes, uint32_t InitCapacity);
void AppendItem(NoteVec* Notes, note_released Note);
void PopItem(NoteVec* Notes);
void FreeNoteVecData(NoteVec* Notes);
void UnorderedRemoveByIndex(NoteVec* Notes, uint32_t Index);

#endif
