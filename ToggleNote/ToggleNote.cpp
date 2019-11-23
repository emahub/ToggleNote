/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
MIDI Processor AU
*/

#include "ToggleNote.h"

using namespace std;

static const int kMIDIPacketListSize = 2048;
static bool toggleStatus = false;

AUDIOCOMPONENT_ENTRY(AUMIDIEffectFactory, ToggleNote)

enum {
    kParameter_Ch = 0,
    kParameter_CC = 1,
    kParameter_NoteMin = 2,
    kParameter_NoteMax = 3,
    kParameter_Velocity = 4,
    kNumberOfParameters = 5
};

static const CFStringRef kParamName_Ch = CFSTR("Ch: ");
static const CFStringRef kParamName_CC = CFSTR("CC: ");
static const CFStringRef kParamName_NoteMin = CFSTR("Note Min: ");
static const CFStringRef kParamName_NoteMax = CFSTR("Note Max: ");
static const CFStringRef kParamName_Velocity = CFSTR("Velocity: ");

#pragma mark ToggleNote

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	ToggleNote::SetProperty
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ToggleNote::ToggleNote(AudioUnit component) : AUMIDIEffectBase(component), mOutputPacketFIFO(LockFreeFIFO<MIDIPacket>(32))
{
    #ifdef DEBUG
        string bPath, bFullFileName;
        bPath = getenv("HOME");
        if (!bPath.empty()) {
            bFullFileName = bPath + "/Desktop/" + "Debug.log";
        } else {
            bFullFileName = "Debug.log";
        }
        
        baseDebugFile.open(bFullFileName.c_str());
        DEBUGLOG_B("Plug-in constructor invoked with parameters:" << endl);
    #endif

	CreateElements();
    
    Globals()->UseIndexedParameters(kNumberOfParameters);
    Globals()->SetParameter(kParameter_Ch, 1);
    Globals()->SetParameter(kParameter_CC, 64);
    Globals()->SetParameter(kParameter_NoteMin, 60);
    Globals()->SetParameter(kParameter_NoteMax, 72);
    Globals()->SetParameter(kParameter_Velocity, 127);
    
    mMIDIOutCB.midiOutputCallback = nullptr;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//    ToggleNote::SetProperty
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus ToggleNote::GetParameterInfo(
                                        AudioUnitScope inScope, AudioUnitParameterID inParameterID,
                                        AudioUnitParameterInfo &outParameterInfo) {
    
#ifdef DEBUG
    DEBUGLOG_B("GetParameterInfo - inScope: " << inScope << endl);
    DEBUGLOG_B("GetParameterInfo - inParameterID: " << inParameterID << endl);
#endif
    
    if (inScope != kAudioUnitScope_Global) return kAudioUnitErr_InvalidScope;
    
    outParameterInfo.flags += kAudioUnitParameterFlag_IsWritable;
    outParameterInfo.flags += kAudioUnitParameterFlag_IsReadable;
    
    switch (inParameterID) {
        case kParameter_Ch:
            AUBase::FillInParameterName(outParameterInfo, kParamName_Ch, false);
            outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
            outParameterInfo.minValue = 1;
            outParameterInfo.maxValue = 16;
            break;
        case kParameter_CC:
            AUBase::FillInParameterName(outParameterInfo, kParamName_CC,
                                        false);
            outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
            outParameterInfo.minValue = 1;
            outParameterInfo.maxValue = 127;
            break;
        case kParameter_NoteMin:
            AUBase::FillInParameterName(outParameterInfo, kParamName_NoteMin,
                                        false);
            outParameterInfo.unit = kAudioUnitParameterUnit_MIDINoteNumber;
            outParameterInfo.minValue = 1;
            outParameterInfo.maxValue = 127;
            break;
        case kParameter_NoteMax:
            AUBase::FillInParameterName(outParameterInfo, kParamName_NoteMax,
                                        false);
            outParameterInfo.unit = kAudioUnitParameterUnit_MIDINoteNumber;
            outParameterInfo.minValue = 1;
            outParameterInfo.maxValue = 127;
            break;
        case kParameter_Velocity:
            AUBase::FillInParameterName(outParameterInfo, kParamName_Velocity,
                                        false);
            outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
            outParameterInfo.minValue = 1;
            outParameterInfo.maxValue = 127;
            break;
        default:
            return kAudioUnitErr_InvalidParameter;
    }

    return noErr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	ToggleNote::SetProperty
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ToggleNote::~ToggleNote()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	ToggleNote::SetProperty
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus ToggleNote::GetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, UInt32 & outDataSize, Boolean & outWritable)
{
    if (inScope == kAudioUnitScope_Global)
    {
        switch( inID )
        {
            case kAudioUnitProperty_MIDIOutputCallbackInfo:
                outWritable = false;
                outDataSize = sizeof(CFArrayRef);
                return noErr;
                
            case kAudioUnitProperty_MIDIOutputCallback:
                outWritable = true;
                outDataSize = sizeof(AUMIDIOutputCallbackStruct);
                return noErr;
        }
	}
	return AUMIDIEffectBase::GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	ToggleNote::SetProperty
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus ToggleNote::GetProperty( AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, void* outData )
{
	if (inScope == kAudioUnitScope_Global) 
	{
        switch (inID)
		{
            case kAudioUnitProperty_MIDIOutputCallbackInfo:
                CFStringRef string = CFSTR("midiOut");
                CFArrayRef array = CFArrayCreate(kCFAllocatorDefault, (const void**)&string, 1, nullptr);
                CFRelease(string);
                *((CFArrayRef*)outData) = array;
                return noErr;
		}
	}
	return AUMIDIEffectBase::GetProperty(inID, inScope, inElement, outData);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	ToggleNote::SetProperty
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus ToggleNote::SetProperty(	AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, const void* inData, UInt32 inDataSize)
{
    if (inScope == kAudioUnitScope_Global)
    {
        switch (inID)
        {
            case kAudioUnitProperty_MIDIOutputCallback:
            mMIDIOutCB = *((AUMIDIOutputCallbackStruct*)inData);
            return noErr;
        }
    }
	return AUMIDIEffectBase::SetProperty(inID, inScope, inElement, inData, inDataSize);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	ToggleNote::HandleMidiEvent
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus ToggleNote::HandleMidiEvent(UInt8 status, UInt8 channel, UInt8 data1, UInt8 data2, UInt32 inOffsetSampleFrame)
{
    if (!IsInitialized()) return kAudioUnitErr_Uninitialized;
  
    MIDIPacket* packet = mOutputPacketFIFO.WriteItem();
    mOutputPacketFIFO.AdvanceWritePtr();
    
    if (packet == NULL)
        return kAudioUnitErr_FailedInitialization;
    
    memset(packet->data, 0, sizeof(Byte)*256);
    packet->length = 3;
    packet->data[0] = status | channel;
    packet->data[1] = data1;
    packet->data[2] = data2;
    packet->timeStamp = inOffsetSampleFrame;
    
	return noErr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	ToggleNote::Render
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus ToggleNote::Render (AudioUnitRenderActionFlags &ioActionFlags, const AudioTimeStamp& inTimeStamp, UInt32 nFrames)
{
    Byte listBuffer[kMIDIPacketListSize];
    MIDIPacketList* packetList = (MIDIPacketList*)listBuffer;
    MIDIPacket* packetListIterator = MIDIPacketListInit(packetList);
  
    MIDIPacket* packet = mOutputPacketFIFO.ReadItem();
    while (packet != NULL)
    {
        //----------------------------------------------------------------------//
        // This is where the midi packets get processed
        
        int _status = packet->data[0] & 0b11110000;
        int _ch = packet->data[0] & 0b1111;
         #ifdef DEBUG
            DEBUGLOG_B("status: " << _status <<
                       ", ch: " << _ch <<
                       ", packet->data[0]: " << (int)packet->data[0] <<
                       ", packet->data[1]: " << (int)packet->data[1] <<
                       ", packet->data[2]: " << (int)packet->data[2] << endl);
        #endif
        
        if(_status == kNoteOn && _ch == 0){
            if(packet->data[1] == 60 && packet->data[2] > 0){
                if(!toggleStatus){
                    DEBUGLOG_B("start" << endl);
                    packet->data[1] = 48;
                    packet->data[2] = 100;
                    toggleStatus = true;
                } else {
                    DEBUGLOG_B("ignore" << endl);
                    packet->data[0] = 0;
                    packet->data[1] = 0;
                    packet->data[2] = 0;
                }
                    
            }else if(packet->data[1] == 61 && packet->data[2] > 0){
                DEBUGLOG_B("stop" << endl);
                packet->data[1] = 48;
                packet->data[2] = 0;
                toggleStatus = false;
            }
        }
        
        //----------------------------------------------------------------------//
        
        if (packetListIterator == NULL) return noErr;
        packetListIterator = MIDIPacketListAdd(packetList, kMIDIPacketListSize, packetListIterator, packet->timeStamp, packet->length, packet->data);
        mOutputPacketFIFO.AdvanceReadPtr();
        packet = mOutputPacketFIFO.ReadItem();
    }
    
    if (mMIDIOutCB.midiOutputCallback != NULL && packetList->numPackets > 0)
    {
        mMIDIOutCB.midiOutputCallback(mMIDIOutCB.userData, &inTimeStamp, 0, packetList);
    }
      
    return noErr;
}



