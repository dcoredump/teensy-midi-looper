#include <vector>
#include <functional>

#ifdef build_for_arduino
#include "Arduino.h"
#else
#include "tests/mock_arduino.h"
#endif

#include "midi/MIDI.h"
#include "MidiLoopSequencer.h"
#include "Delegate.h"

MidiLoopSequencer::MidiLoopSequencer(midi::MidiInterface<HardwareSerial> *midiPort) {
  _midi_port = midiPort;
}

void MidiLoopSequencer::initialize() {
  _midi_port->begin(MIDI_CHANNEL_OMNI);
  _beats_per_minute = 120.0;
  float seconds_per_beat = 60 / _beats_per_minute;
  float millis_per_beat = seconds_per_beat * 1000;
  _millis_per_16th = millis_per_beat/16;
  
  _milliseconds = 0;
  _previousMilliseconds = 0;
  _lastEventMillis = 0;
}

void MidiLoopSequencer::tick(unsigned long millisecs) {
  updateBarAndBeat(millisecs);
  processNewIncomingMidiMessages();
}

bool MidiLoopSequencer::isNoteEvent(byte msgType) {
  switch (msgType) {
    case midi::AfterTouchPoly:                
    case midi::AfterTouchChannel:    
    case midi::PitchBend:             
    case midi::ControlChange:        
    case midi::ProgramChange: 
    case midi::NoteOn:
    case midi::NoteOff: 
      return true;
      
    default:
      return false;
  }
}

void MidiLoopSequencer::processNewIncomingMidiMessages() {
  if (_midi_port->read()) {
     
    if (_en_midiThru) {
      _midi_port->send(
        _midi_port->getType(),
        _midi_port->getData1(),
        _midi_port->getData2(),
        _midi_port->getChannel());
    }

    if (isNoteEvent(_midi_port->getType())) {
        
      unsigned long delta = _milliseconds - _lastEventMillis;
      _lastEventMillis = _milliseconds; 
      
      unsigned long deltaTicks = delta / _millis_per_16th;
  
      switch (_midi_port->getType () ) {
        case midi::NoteOn: {
          //if (_sdCardFound)
          //  midi_writer.addEvent(q, midiA.getType(), midiA.getData1(), midiA.getData2(), midiA.getChannel());
        
          //piano.keyDown(midiA.getData1());
          //piano2.keyDown(midiA.getData1());

          onKeyChanged(true, _midi_port->getData1(), _midi_port->getData2(), _midi_port->getChannel() );

          //if (onKeyChanged) 
          //  onKeyChanged(true, _midi_port->getData1(), _midi_port->getData2(), _midi_port->getChannel() );

          break;
        } 
        
        case midi::NoteOff: {
          //if (_sdCardFound)
          //  midi_writer.addEvent(q, midiA.getType(), midiA.getData1(), midiA.getData2(), midiA.getChannel());         
          //piano.keyUp(midiA.getData1());
          //piano2.keyUp(midiA.getData1());
          onKeyChanged(false, _midi_port->getData1(), _midi_port->getData2(), _midi_port->getChannel() );
          break;
        }  
        
        case midi::AfterTouchPoly:       //= 0xA0    //# Polyphonic AfterTouch         
        case midi::AfterTouchChannel:    //= 0xD0    //# Channel (monophonic) AfterTouch
        case midi::PitchBend:            //= 0xE0    //# Pitch Bend  
        case midi::ControlChange:        //= 0xB0    //# Control Change / Channel Mode
        case midi::ProgramChange: {
          //if (_sdCardFound)
          //  midi_writer.addEvent(q, midiA.getType(), midiA.getData1(), midiA.getData2(), midiA.getChannel());
                          
          break;
        }   
         
        default:
          break;
        
      }
    }
  }
}

void MidiLoopSequencer::updateBarAndBeat(unsigned long millisecs) {
  _previousMilliseconds = _milliseconds;
  _milliseconds = millisecs;
  
  if (_previousMilliseconds > _milliseconds) {     
    // overflow occurred
    unsigned long underflow = (0xffffffff - _previousMilliseconds);
    _sixtyFourth = (underflow + _milliseconds) / _millis_per_16th;
  } else {
    _sixtyFourth = _milliseconds / _millis_per_16th;
  }

    unsigned long beat = (_sixtyFourth / 16);
    int newBar = beat / 4;
    long newBeat = beat % 3;
    if (newBar != _position.bar || newBeat != _position.beat) {
        _position.bar = newBar;
        _position.beat = newBeat;
        onPositionChanged(_position);
    }
}

SongPosition MidiLoopSequencer::getSongPosition() {
  return _position;
}


void MidiLoopSequencer::setSongPosition(int bar, byte beat) {
  _position.bar = bar;
  _position.beat = beat;
}


void MidiLoopSequencer::setLoopEnabled(bool loop_enabled) {
  _en_loop = loop_enabled;
}


int MidiLoopSequencer::getLoopDuration() {
  return _en_loop;
}


void MidiLoopSequencer::setLoopDuration(int bars) {
  _loop_duration_bars = bars;
}


bool MidiLoopSequencer::getOverdubEnabled() {
  return _en_overdub;
}


void MidiLoopSequencer::setOverdubEnabled(bool overdub_enabled) {
  _en_overdub = overdub_enabled;
}


char* MidiLoopSequencer::getFilename() {
  return _fileName;
}


void MidiLoopSequencer::setFilename(char* filename) {
  _fileName = filename;
}


bool MidiLoopSequencer::getPlayEnable() {
  return _en_play;
}


void MidiLoopSequencer::setPlayEnable(bool play_enabled) {
  _en_play = play_enabled;
}


bool MidiLoopSequencer::getRecordEnabled() {
  return _en_rec;
}


void MidiLoopSequencer::setRecordEnabled(bool record_enabled) {
  _en_rec = record_enabled;
}


bool MidiLoopSequencer::getStepRecordEnabled() {
  return _en_step_record;
}


void MidiLoopSequencer::setStepRecordEnabled(bool step_record_enabled) {
  _en_step_record = step_record_enabled;
}

// private

void MidiLoopSequencer::allNotesOff() {
}
