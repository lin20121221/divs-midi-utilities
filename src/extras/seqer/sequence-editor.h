#ifndef SEQUENCE_EDITOR_INCLUDED
#define SEQUENCE_EDITOR_INCLUDED

class SequenceEditor;
class Sequence;
class EventList;
class PianoRoll;
class Row;
class Step;

#include <vector>
#include <wx/wx.h>
#include <midifile.h>
#include "seqer.h"
#include "music-math.h"

typedef enum
{
	EVENT_TYPE_INVALID = -1,
	EVENT_TYPE_NOTE,
	EVENT_TYPE_CONTROL_CHANGE,
	EVENT_TYPE_PROGRAM_CHANGE,
	EVENT_TYPE_AFTERTOUCH,
	EVENT_TYPE_PITCH_BEND,
	EVENT_TYPE_SYSTEM_EXCLUSIVE,
	EVENT_TYPE_TEXT,
	EVENT_TYPE_LYRIC,
	EVENT_TYPE_MARKER,
	EVENT_TYPE_PORT,
	EVENT_TYPE_TEMPO,
	EVENT_TYPE_TIME_SIGNATURE,
	EVENT_TYPE_KEY_SIGNATURE,
	EVENT_TYPE_HIGHEST
}
EventType_t;

class SequenceEditor: public wxScrolledCanvas
{
public:
	Window* window;
	Sequence* sequence;
	EventList* event_list;
	PianoRoll* piano_roll;
	StepSize* step_size;
	std::vector<Row> rows;
	std::vector<Step> steps;
	std::vector<int> filtered_event_types;
	std::vector<int> filtered_tracks;
	std::vector<int> filtered_channels;
	long current_row_number;
	long current_column_number;
	int insertion_track_number;
	int insertion_channel_number;
	int insertion_note_number;
	int insertion_velocity;

	SequenceEditor(Window* window);
	~SequenceEditor();
	bool Load(wxString filename);
	void Prepare();
	void OnDraw(wxDC& dc);
	long GetVisibleWidth();
	long GetVisibleHeight();
	long GetNumberOfVisibleRows();
	long GetFirstVisibleY();
	long GetLastVisibleY();
	long GetFirstRowNumberFromStepNumber(long step_number);
	long GetLastRowNumberFromStepNumber(long step_number);
	long GetStepNumberFromRowNumber(long row_number);
	long GetStepNumberFromTick(long tick);
	double GetFractionalStepNumberFromTick(long tick);
	long GetTickFromRowNumber(long row_number);
	MidiFileEvent_t GetLatestTimeSignatureEventForRowNumber(long row_number);
	bool Filter(MidiFileEvent_t event);
	void SetStepSize(StepSize* step_size, bool prepare = true);
	void ZoomIn(bool prepare = true);
	void ZoomOut(bool prepare = true);
	void ScrollToCurrentRow();
	void RowUp();
	void RowDown();
	void PageUp();
	void PageDown();
	void GoToFirstRow();
	void GoToLastRow();
	void ColumnLeft();
	void ColumnRight();
	void GoToColumn(int column_number);
	void GoToNextMarker();
	void GoToPreviousMarker();
	void GoToMarker(wxString marker_name);
	void InsertNote(int diatonic);
	wxString GetEventTypeName(EventType_t event_type);
	EventType_t GetEventType(MidiFileEvent_t event);
};

class Sequence
{
public:
	SequenceEditor* sequence_editor;
	MidiFile_t midi_file;

	Sequence(SequenceEditor* sequence_editor);
	~Sequence();
};

class EventList
{
public:
	SequenceEditor *sequence_editor;
	wxFont font;
	wxColour current_cell_border_color;
	long row_height;
	long column_widths[8];

	EventList(SequenceEditor* sequence_editor);
	void Prepare();
	void OnDraw(wxDC& dc);
	long GetVisibleWidth();
	long GetFirstVisibleRowNumber();
	long GetLastVisibleRowNumber();
	long GetLastVisiblePopulatedRowNumber();
	long GetColumnWidth(long column_number);
	long GetXFromColumnNumber(long column_number);
	long GetYFromRowNumber(long row_number);
	long GetRowNumberFromY(long y);
	wxString GetCellText(long row_number, long column_number);
};

class PianoRoll
{
public:
	SequenceEditor *sequence_editor;
	long first_note;
	long last_note;
	long key_width;
	wxColour darker_line_color;
	wxColour lighter_line_color;
	wxColour lightest_line_color;
	wxColour white_key_color;
	wxColour black_key_color;
	wxColour shadow_color;

	PianoRoll(SequenceEditor* sequence_editor);
	void Prepare();
	void OnDraw(wxDC& dc);
	long GetWidth();
	long GetYFromStepNumber(double step_number);
};

class Row
{
public:
	long step_number;
	MidiFileEvent_t event;

	Row(long step_number, MidiFileEvent_t event);
};

class Step
{
public:
	long first_row_number;
	long last_row_number;

	Step(long row_number);
};

#endif
