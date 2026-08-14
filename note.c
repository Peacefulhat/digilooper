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

void AddNote(notes* Notes, note_callback Notefn, float Semitone, KeyboardKey Key)
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
        Buffer[Index] += Amplitude * sinf(2 * PI * Time * Note->Frequency);
    }
}

void AddKeyBoardNotes(notes* Notes)
{
    if(!Notes) {
        return;
    }
    AddNote(Notes, GetNote, 0, KEY_Z);
    AddNote(Notes, GetNote, 1, KEY_S);
    AddNote(Notes, GetNote, 2, KEY_X);
    AddNote(Notes, GetNote, 3, KEY_D);
    AddNote(Notes, GetNote, 4, KEY_C);
    AddNote(Notes, GetNote, 5, KEY_V);
    AddNote(Notes, GetNote, 6, KEY_G);
    AddNote(Notes, GetNote, 7, KEY_B);
    AddNote(Notes, GetNote, 8, KEY_H);
    AddNote(Notes, GetNote, 9, KEY_N);
    AddNote(Notes, GetNote, 10, KEY_J);
    AddNote(Notes, GetNote, 11, KEY_M);
    AddNote(Notes, GetNote, 12, KEY_COMMA);
}

void Clamp2(void* Data, type Low, type High, type_info Info)
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
