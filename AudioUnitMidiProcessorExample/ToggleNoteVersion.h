/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
MIDI Processor AU
*/

#ifndef __ToggleNoteVersion_h__
#define __ToggleNoteVersion_h__

#ifdef DEBUG
	#define kToggleNoteVersion    0xFFFFFFFF
#else
	#define kToggleNoteVersion    0x00010000
#endif

#define ToggleNote_COMP_TYPE      'aumi' // Audio Units Midi Plugin
#define ToggleNote_COMP_SUBTYPE   'tgnt' // Change for your product - product code
#define ToggleNote_COMP_MANF      'emah' // Change for your product - company code

#endif

