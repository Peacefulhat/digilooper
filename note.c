#include "note.h"

void InitVector(NoteVec* Notes, uint32_t InitCapacity)
{
    if(!Notes) {
        printf("InitVector: not a valid NoteVec ");
        return;
    }
    Notes->Data = NULL;
    Notes->Count = 0;
    Notes->Capacity = InitCapacity;
}

void AppendItem(NoteVec* ReleasedNotes, note_released Note)
{
    if(!ReleasedNotes) {
        printf("InitVector: not a valid NoteVec ");
        return;
    }

    while(ReleasedNotes->Count >= ReleasedNotes->Capacity) {

        ReleasedNotes->Capacity *= 2;
    }
    ReleasedNotes->Data = realloc(ReleasedNotes->Data, sizeof(note_released) * ReleasedNotes->Capacity);
    assert(ReleasedNotes->Data != NULL && "Buy More ram");
    ReleasedNotes->Data[ReleasedNotes->Count++] = Note;
}

void PopItem(NoteVec* Notes)
{
    if (Notes == NULL) {
        printf("PopItem: invalid NoteVec\n");
        return;
    }

    if (Notes->Count > 0) {
        Notes->Count--;
    }

}
/*void PopItem(NoteVec* Notes)
{
    if(!Notes) {
        printf("InitVector: not a valid NoteVec ");
        return;
    }
    if(Notes->Count > 0) {
        note_released EmptyNote = {};
        Notes->Data[Notes->Count--] = EmptyNote;
    }
}

*/
void FreeNoteVecData(NoteVec* Notes)
{
    if(!Notes) {
        printf("FreeNotesData: not a valid NoteVec");
        return;
    }
    if(Notes->Data == NULL) {
        return;
    }
    free(Notes->Data);
    Notes->Capacity = DA_INIT_CAP;
    Notes->Count = 0;
}

void UnorderedRemoveByIndex(NoteVec* Note, uint32_t Index)
{
    if(!Note) {
        printf("UnorderedRemoveByIndex: not a valid NoteVec");
        return;
    }

    if((Note->Count > 0) && (Index <= Note->Count - 1)) {
        Note->Data[Index] = Note->Data[Note->Count - 1];
        PopItem(Note);
    }
}


//Todo:: Error Logging
static float32 Min(float32 A, float32 B)
{
    if(A < B) {
        return(A);
    } else {
        return(B);
    }
}

// instrument funcitons
// for now not using the void* Data Signature
//internal float32 FunctionSine(float32 X, void* Data)
internal float32 FunctionSine(float64 X)
{
    return sin(X * 2 * PI);
}
//internal float32 FunctionSquareWave(float32 X, void* Data)
internal float32 FunctionSquareWave(float64 X) // here x is in period of 1.
{
    float64 P = 0.5f; // parametrize square wave
    X = truncf(X);
    if(X <= P) {
        return (1.0f);
    } else {
        return (-1.0f);
    }
}

internal float32 FunctionSawTooth(float64 X)
{
    float64 P = 0.5f; // parametrize square wave
    X = truncf(X);
    if(X <= P) {
        return Lerp(-1, 1, X/P);
    } else {
        return Lerp(1, -1, (X - P)/(1.0f - P));
    }
}

// instruments
global_variable instrument InstrumentSine = {
    .Function = FunctionSine
};

global_variable instrument InstrumentSquareWave = {
    .Function = FunctionSine
};

global_variable instrument InstrumentSawTooth = {
    .Function = FunctionSawTooth
};

float32 InstrumentRun (instrument* Instrument, float32 X)
{
    return Instrument->Function(X);
}

float64 GetFreqFromSemiTone(float32 Semitone)
{
    return(ROOT_NOTE * pow(NEXT_SEMI_TONE, Semitone));
}

// Clean the Buffer
void CleanBuffer(float32* Buffer, uint32_t BufferSize)
{
    if(!Buffer) {
        return;
    }
    for(int32_t Index = 0; Index < BufferSize; Index++) {
        Buffer[Index] = 0;
    }
}

internal note GetNote(float32 Semitone)
{
    return(note) {
        .Frequency = GetFreqFromSemiTone(Semitone)
    };
}

void AddNote(notes* Notes, note_callback Notefn, float32 Semitone, KeyboardKey Key)
{
    if(!Notes || !Notefn) {
        return;
    }
    assert(Notes->Count < KEYBOARD_NOTES_CAPACITY);
    Notes->NoteKeys[Notes->Count] = Notefn(Semitone);
    Notes->NoteKeys[Notes->Count].Key = Key;
    Notes->Count++;
}

void NotePress(note* Note)
{
    if(!Note) {
        printf("NotePlay: Not a valid note\n");
        return;
    }
    Note->Playing = true;
    Note->FrameCount = 0;
}

void NoteRelease(note* Note, NoteVec* Notes, uint32_t FrameCount)
{
    if(!Note) {
        printf("NotePlay: Not a valid note\n");
        return;
    }

    if(Note->Playing) {
        Note->Playing = false;
        float32 UnclippedVolume = (float32)(FrameCount - Note->FrameStamp)/(float32)(ATTACK_FRAME);
        float32 Volume = Min(UnclippedVolume, 1.0f); // Volume in terms of sound
        note_released RNote = {.FrameCount = Note->FrameCount,
                               .Key = Note->Key,
                               .Frequency = Note->Frequency,
                               .StoppedAtFrame= Note->FrameCount,
                               .StoppedAtVolume = Volume
                              };
        AppendItem(Notes, RNote);
    }
}

bool NoteReleaseDone(note_released* Note)
{
    return(Note->FrameCount - Note->StoppedAtFrame) >= RELEASE_FRAME;
}

float64 NoteReleasedUpdate(note_released* Note)
{
    float64 Sample = (0.0f);
    if(!Note) {
        // logging
        printf("NoteUpdate: Failed to generate Sample for given Frame Count");
        return (Sample);
    }
    float64 UnclippedVolume = (float64)(Note->FrameCount - Note->StoppedAtFrame)/(float64)(RELEASE_FRAME);
    float64 Volume = (1.0f - Min(UnclippedVolume, 1.0f)) * Note->StoppedAtVolume; // Volume in terms of sound
    float64 Time = (float64)(Note->FrameCount)/(float64)SAMPLE_RATE;
    Note->FrameCount += 1;
    Sample = InstrumentSine.Function(Time * Note->Frequency) * Volume;
    return (Sample);
}

// return sample for specific framecount
float64 NoteUpdate(note* Note)
{
    float64 Sample = (0.0f);
    if(!Note) {
        // logging
        printf("NoteUpdate: Failed to generate Sample for given Frame Count");
        return (Sample);
    }
    // TODO: use Note, Frame Count and remove frame stamp
    float64 UnclippedVolume = (float64)(Note->FrameCount)/(float64)(ATTACK_FRAME);
    float64 Volume = Min(UnclippedVolume, 1.0f); // Volume in terms of sound
    float64 Time = (float64)(Note->FrameCount)/(float64)SAMPLE_RATE;
    Note->FrameCount += 1;
    Sample = InstrumentSine.Function(Time * Note->Frequency) * Volume;
    return (Sample);
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

void Clamp32(void* Data, type Low, type High, type_info Info)
{
    if(!Data) {
        return;
    }

    if(Info == INT32) {
        if(*((int32_t*)Data) < Low.AsI32) {
            *((int32_t*)Data) = Low.AsI32;
        }

        if(*((int32_t*)Data) > High.AsI32) {
            *((int32_t*)Data) = High.AsI32;
        }

    } else if(Info == FLOAT32) {

        if(*((float32*)Data) < Low.AsF32) {
            *((float32*)Data) = Low.AsF32;
        }

        if(*((float32*)Data) > High.AsF32) {
            *((float32*)Data) = High.AsF32;
        }
    } else {
        printf("Unknown type");
    }
}


void Clamp64(void* Data, type Low, type High, type_info Info)
{
    if(!Data) {
        return;
    }

    if(Info == INT64) {
        if(*((int64_t*)Data) < Low.AsI32) {
            *((int64_t*)Data) = Low.AsI32;
        }

        if(*((int64_t*)Data) > High.AsI32) {
            *((int64_t*)Data) = High.AsI32;
        }

    } else if(Info == FLOAT64) {

        if(*((float64*)Data) < Low.AsF32) {
            *((float64*)Data) = Low.AsF32;
        }

        if(*((float64*)Data) > High.AsF32) {
            *((float64*)Data) = High.AsF32;
        }
    } else {
        printf("Unknown type");
    }
}

float32 Lerp32(float32 A, float32 B, float32 T)
{
    return A + T * (B - A);
}

float64 Lerp64(float64 A, float64 B, float64 T)
{
    return A + T * (B - A);
}
