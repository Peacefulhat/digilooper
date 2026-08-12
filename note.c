#include "note.h"

//Todo:: Error Logging

float GetFreqFromSemiTone(float Semitone)
{
    return(ROOT_NOTE * pow(NEXT_SEMI_TONE, Semitone));
}

// Clean the Buffer
void CleanBuffer(float* Buffer, uint32_t BufferSize)
{
    if(!Buffer) {
        return;
    }
    for(int32_t Index = 0; Index < BufferSize; Index++) {
        Buffer[Index] = 0;
    }
}

internal note GetNote(float Semitone){
    return(note){
        .Frequency = GetFreqFromSemiTone(Semitone)
    };
}

void AddNote(notes* Notes, note_callback Notefn, float Semitone, SDL_Scancode Key)
{
    if(!Notes || !Notefn) {
        return;
    }
    assert(Notes->Count < KEYBOARD_NOTES_CAPACITY);
    Notes->NoteKeys[Notes->Count] = Notefn(Semitone);
    Notes->NoteKeys[Notes->Count].Key = Key;
    Notes->Count++;
}

void NoteUpdate(note* Note, float* Buffer, uint32_t FrameCount, float Amplitude, uint32_t BufferSize)
{
    if(!Note || !Buffer){
        return;
    }
    for (uint32_t Index = 0; Index < BufferSize; Index++) {
        float Time = (float)(FrameCount + Index)/(float)SAMPLE_RATE;
        Buffer[Index] += Amplitude * SDL_sinf(2 * SDL_PI_F * Time * Note->Frequency);
    }
}

void KeyBoardNotes(notes* Notes)
{
    if(!Notes) {
        return;
    }
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

void Clamp(void* Data, type Low, type High, type_info Info)
{
    if(!Data) {
        return;
    }
    
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
