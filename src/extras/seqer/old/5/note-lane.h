#ifndef NOTE_LANE_INCLUDED
#define NOTE_LANE_INCLUDED

class ControlChaneLane;

#include <wx/wx.h>
#include "lane.h"
#include "midifile.h"
#include "window.h"

class NoteLane: public Lane
{
public:
	int pixels_per_note;
	int scroll_y;

	NoteLane(Window* window);
	~NoteLane();
	virtual void OnPaint(wxPaintEvent& event);
	virtual void PaintBackground(wxDC& dc, int width, int height);
	virtual void PaintNotes(wxDC& dc, int width, int height);
	virtual MidiFileEvent_t GetEventFromXY(int x, int y);
	virtual wxRect GetRectFromEvent(MidiFileEvent_t midi_event);
	virtual int GetYFromNote(int note);
	virtual int GetNoteFromY(int y);
};

#endif
