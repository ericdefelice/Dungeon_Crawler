/*!
  @file
  audio.h

  @brief
  Engine audio functionality.

  @detail
  Handles audio using Direct Audio.
*/

#pragma once

//--------------------------------------------
// Linking
//--------------------------------------------
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <stdio.h>


namespace Gumshoe {

//--------------------------------------------
// Audio class definition
//--------------------------------------------
class Audio
{
private:
	struct wavHeader_t
	{
		uint8   chunkId[4];
		uint32  chunkSize;
		uint8   format[4];
		uint8   subChunkId[4];
		uint32  subChunkSize;
		uint16  audioFormat;
		uint16  numChannels;
		uint32  sampleRate;
		uint32  bytesPerSecond;
		uint16  blockAlign;
		uint16  bitsPerSample;
		uint8   dataChunkId[4];
		uint32  dataSize;
	};

public:
	Audio();
	~Audio();

	bool Init(HWND);
	void Shutdown();

	bool PlayWaveInBuffer();

private:
	bool InitDirectSound(HWND);
	void ShutdownDirectSound();

	bool LoadWaveFile(char*, IDirectSoundBuffer8**);
	void ShutdownWaveFile(IDirectSoundBuffer8**);

	bool PlayWaveFile();

private:
	IDirectSound8* m_DirectSound;
	IDirectSoundBuffer* m_primaryBuffer;
	IDirectSoundBuffer8* m_secondaryBuffer1;
};

} // end of namespace Gumshoe
