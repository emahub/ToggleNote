/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
MIDI Processor AU
*/

#ifndef __ToggleNote_h__
#define __ToggleNote_h__

#include "ToggleNoteVersion.h"
#include <CoreMIDI/CoreMIDI.h>
#include "AUMIDIEffectBase.h"
#include "LockFreeFIFO.h"

#define kNoteOn 0x90
#define kNoteOff 0x80

#ifdef DEBUG
#include <fstream>
#include <ctime>
#define DEBUGLOG_B(x) \
if (baseDebugFile.is_open()) baseDebugFile << x
#else
#define DEBUGLOG_B(x)
#endif

using namespace std;

#pragma mark - ToggleNote
class ToggleNote : public AUMIDIEffectBase
{
public:
	ToggleNote(AudioUnit component);
	virtual ~ToggleNote();

	virtual OSStatus GetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, UInt32& outDataSize, Boolean& outWritable );

	virtual OSStatus GetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, void* outData);
  
    virtual    OSStatus            GetParameterInfo(    AudioUnitScope            inScope,
                                                    AudioUnitParameterID    inParameterID,
                                                    AudioUnitParameterInfo    &outParameterInfo);
    
    virtual OSStatus SetProperty(	AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, const void* inData, UInt32 inDataSize);

 	virtual	bool SupportsTail() { return false; }

	virtual OSStatus Version() { return kToggleNoteVersion; }

    virtual OSStatus HandleMidiEvent(UInt8 status, UInt8 channel, UInt8 data1, UInt8 data2, UInt32 inOffsetSampleFrame);
  
    virtual OSStatus Render(AudioUnitRenderActionFlags &ioActionFlags, const AudioTimeStamp& inTimeStamp, UInt32 nFrames);

private:
    AUMIDIOutputCallbackStruct mMIDIOutCB;
  
    LockFreeFIFO<MIDIPacket> mOutputPacketFIFO;
  
protected:
#ifdef DEBUG
    ofstream baseDebugFile;
#endif
};

#endif
