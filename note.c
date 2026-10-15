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
static float Min(float A, float B)
{
    if(A < B) {
        return(A);
    } else {
        return(B);
    }
}


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

internal note GetNote(float Semitone)
{
    return(note) {
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

void NotePress(note* Note, uint32_t FrameCount)
{
    if(!Note) {
        printf("NotePlay: Not a valid note\n");
        return;
    }
    Note->Playing = true;
    Note->FrameStamp = FrameCount;
}

void NoteRelease(note* Note, NoteVec* Notes, uint32_t FrameCount)
{
    if(!Note) {
        printf("NotePlay: Not a valid note\n");
        return;
    }

    if(!Note->Playing) {
        return;
    }
    Note->Playing = false;
    float UnclippedVolume = (float)(FrameCount - Note->FrameStamp)/(float)(ATTACK_FRAME);
    float Volume = Min(UnclippedVolume, 1.0f); // Volume in terms of sound
    note_released RNote = {.FrameStamp = FrameCount, .Key = Note->Key, .Frequency = Note->Frequency, .Volume = Volume};
    AppendItem(Notes, RNote);
}

bool NoteReleaseDone(note_released* Note, uint32_t FrameCount)
{
    return(FrameCount - Note->FrameStamp) >= RELEASE_FRAME;
}

float NoteReleasedUpdate(note_released* Note, uint32_t FrameCount)
{
    float Sample = (0.0f);
    if(!Note) {
        // logging
        printf("NoteUpdate: Failed to generate Sample for given Frame Count");
        return (Sample);
    }
    float UnclippedVolume = (float)(FrameCount - Note->FrameStamp)/(float)(RELEASE_FRAME);
    float Volume = (1.0f - Min(UnclippedVolume, 1.0f)) * Note->Volume; // Volume in terms of sound
    float Time = (float)(FrameCount)/(float)SAMPLE_RATE;
    Sample = sinf(2 * PI * Time * Note->Frequency) * Volume;
    return (Sample);
}

// return sample for specific framecount
float NoteUpdate(note* Note, uint32_t FrameCount)
{
    float Sample = (0.0f);
    if(!Note) {
        // logging
        printf("NoteUpdate: Failed to generate Sample for given Frame Count");
        return (Sample);
    }
    float UnclippedVolume = (float)(FrameCount - Note->FrameStamp)/(float)(ATTACK_FRAME);
    float Volume = Min(UnclippedVolume, 1.0f); // Volume in terms of sound
    float Time = (float)(FrameCount)/(float)SAMPLE_RATE;
    Sample = sinf(2 * PI * Time * Note->Frequency) * Volume;
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
