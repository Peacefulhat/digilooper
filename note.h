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

typedef float float32;
typedef double float64;
#define SAMPLE_BUFFER_SIZE (2048 * 2)
#define SAMPLE_RATE 44100
#define MINIMUN_AUDIO (SAMPLE_BUFFER_SIZE * sizeof(float))
#define KEYBOARD_NOTES_CAPACITY 15
#define NEXT_SEMI_TONE 1.0594631f // 2^(1/12)
#define ROOT_NOTE 440
#define ATTACK_FRAME 500
#define RELEASE_FRAME 10000

//typedef  float (*instrument_callback)(float Semitone, void* Data);
typedef  float (*instrument_callback)(double X);
typedef struct {
//    void* Data;
    instrument_callback Function; // period of this funciton is 1, and we are using turn angles.
} instrument;


// note key on keyboard and its playing state
typedef struct {
    bool Playing;
    int32_t FrameCount;
    int32_t FrameStamp; // current global fram count at which it started playing.
    // frame count here is in terms of sound like 44000 sample per second,
    // frame count is where are we in these 44000 sample.
    float Frequency;
    KeyboardKey Key;
    instrument *Instrument;
} note;

typedef struct {
    int32_t FrameCount; // current global fram count at which it started playing.
    float Frequency;
    KeyboardKey Key;
    int32_t StoppedAtFrame;
    float StoppedAtVolume;
    float Volume;
    instrument* Instrument;
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
    INT32,
    FLOAT32,
    INT64,
    FLOAT64,
    UNKNOWN
} type_info;

typedef union {
    int32_t AsI32;
    float   AsF32;
} type;

// utils
float Lerp(float A, float B, float T);
// Instrument Play
float InstrumentRun (instrument* Instrument, float X);

// notes
typedef note (*note_callback)(float Semitone);
bool NoteReleaseDone(note_released* Note);
float64 NoteReleasedUpdate(note_released* Note);
float64 NoteUpdate(note* Note);
void NotePress2(note* Note, instrument* Instrument);
void NotePress(note* Note);
void NoteRelease(note* Note, NoteVec* Notes, uint32_t FrameCount);
float64 GetFreqFromSemiTone(float Semitone);
void CleanBuffer(float* Buffer, uint32_t BufferSize);
void AddNote(notes* Notes, note_callback Notefn, float Semitone, KeyboardKey Key);
void AddKeyBoardNotes(notes* Notes);
void Clamp32(void* Data, type Low, type High, type_info Info);
void Clamp64(void* Data, type Low, type High, type_info Info);
float32 Lerp32(float32 A, float32 B, float32 T);
float64 Lerp64(float64 A, float64 B, float64 T);

// Debug Dynamic Array

#define DA_INIT_CAP 64


void InitVector(NoteVec* Notes, uint32_t InitCapacity);
void AppendItem(NoteVec* Notes, note_released Note);
void PopItem(NoteVec* Notes);
void FreeNoteVecData(NoteVec* Notes);
void UnorderedRemoveByIndex(NoteVec* Notes, uint32_t Index);

#endif
