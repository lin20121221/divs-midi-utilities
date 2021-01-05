
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtmidi_c.h>
#include <midifile.h>
#include <midiutil.h>

static char *midi_in_port = NULL;
static char *prefix = "brainstorm-";
static int timeout_msecs = 5000;
static char *confirmation_command_pattern = NULL;
static MidiUtilAlarm_t alarm;
static RtMidiInPtr midi_in;
static MidiFile_t midi_file = NULL;
static MidiFileTrack_t track = NULL;
static long start_time_msecs;

static void create_midi_file_for_first_event(void)
{
	if (midi_file != NULL) return;
	midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, 960);
	track = MidiFile_createTrack(midi_file); /* conductor track */
	MidiFileTrack_createTimeSignatureEvent(track, 0, 4, 4);
	MidiFileTrack_createKeySignatureEvent(track, 0, 0, 0);
	MidiFileTrack_createTempoEvent(track, 0, 100.0);
	track = MidiFile_createTrack(midi_file); /* main track */
	start_time_msecs = MidiUtil_getCurrentTimeMsecs();
}

static void handle_alarm(void *user_data)
{
	char current_time_string[16];
	char filename[1024];

	MidiUtil_getCurrentTimeString(&current_time_string);
	sprintf(filename, "%s%s.mid", prefix, current_time_string);
	MidiFile_save(midi_file, filename);
	MidiFile_free(midi_file);
	midi_file = NULL;

	if (confirmation_command_pattern != NULL)
	{
		char confirmation_command[1024], *p1, *p2, *p3;

		for (p1 = confirmation_command_pattern, p2 = confirmation_command; *p1 != '\0'; p1++)
		{
			if (*p1 == '%')
			{
				p1++;

				if (*p1 == 's')
				{
					for (p3 = filename; *p3 != '\0'; p3++)
					{
						*(p2++) = *p3;
					}
				}
				else
				{
					*(p2++) = *p1;
				}
			}
			else
			{
				*(p2++) = *p1;
			}
		}

		*p2 = '\0';

		system(confirmation_command);
	}
}

static void handle_midi_message(double timestamp, const unsigned char *message, size_t message_size, void *user_data)
{
	long tick = MidiFile_getTickFromTime(midi_file, (float)(MidiUtil_getCurrentTimeMsecs() - start_time_msecs) / 1000.0);

	switch (message[0] & 0xF0)
	{
		case 0x80:
		{
			int channel = message[0] & 0x0F;
			int note = message[1];
			int velocity = message[2];
			create_midi_file_for_first_event();
			MidiFileTrack_createNoteOffEvent(track, tick, channel, note, velocity);
			MidiUtilAlarm_set(alarm, timeout_msecs, handle_alarm, NULL);
			break;
		}
		case 0x90:
		{
			int channel = message[0] & 0x0F;
			int note = message[1];
			int velocity = message[2];
			create_midi_file_for_first_event();
			MidiFileTrack_createNoteOnEvent(track, tick, channel, note, velocity);
			MidiUtilAlarm_set(alarm, timeout_msecs, handle_alarm, NULL);
			break;
		}
		case 0xA0:
		{
			int channel = message[0] & 0x0F;
			int note = message[1];
			int amount = message[2];
			create_midi_file_for_first_event();
			MidiFileTrack_createKeyPressureEvent(track, tick, channel, note, amount);
			MidiUtilAlarm_set(alarm, timeout_msecs, handle_alarm, NULL);
			break;
		}
		case 0xB0:
		{
			int channel = message[0] & 0x0F;
			int number = message[1];
			int value = message[2];
			create_midi_file_for_first_event();
			MidiFileTrack_createControlChangeEvent(track, tick, channel, number, value);
			MidiUtilAlarm_set(alarm, timeout_msecs, handle_alarm, NULL);
			break;
		}
		case 0xC0:
		{
			int channel = message[0] & 0x0F;
			int number = message[1];
			create_midi_file_for_first_event();
			MidiFileTrack_createProgramChangeEvent(track, tick, channel, number);
			MidiUtilAlarm_set(alarm, timeout_msecs, handle_alarm, NULL);
			break;
		}
		case 0xD0:
		{
			int channel = message[0] & 0x0F;
			int amount = message[1];
			create_midi_file_for_first_event();
			MidiFileTrack_createChannelPressureEvent(track, tick, channel, amount);
			MidiUtilAlarm_set(alarm, timeout_msecs, handle_alarm, NULL);
			break;
		}
		case 0xE0:
		{
			int channel = message[0] & 0x0F;
			int value = (message[1] << 7) | message[2];
			create_midi_file_for_first_event();
			MidiFileTrack_createPitchWheelEvent(track, tick, channel, value);
			MidiUtilAlarm_set(alarm, timeout_msecs, handle_alarm, NULL);
			break;
		}
		default:
		{
			/* ignore everything else */
			break;
		}
	}
}

static void usage(char *program_name)
{
	fprintf(stderr, "Usage: %s --in <port> [ --prefix <filename prefix> ] [ --timeout <seconds> ] [ --confirmation <command line> ]\n", program_name);
	exit(1);
}

int main(int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (++i == argc) usage(argv[0]);
			midi_in_port = argv[i];
		}
		else if (strcmp(argv[i], "--prefix") == 0)
		{
			if (++i == argc) usage(argv[0]);
			prefix = argv[i];
		}
		else if (strcmp(argv[i], "--timeout") == 0)
		{
			if (++i == argc) usage(argv[0]);
			timeout_msecs = atol(argv[i]) * 1000;
		}
		else if (strcmp(argv[i], "--confirmation") == 0)
		{
			if (++i == argc) usage(argv[0]);
			confirmation_command_pattern = argv[i];
		}
		else
		{
			usage(argv[0]);
		}
	}

	alarm = MidiUtilAlarm_new();

	if ((midi_in = rtmidi_open_in_port("brainstorm", midi_in_port, "brainstorm", handle_midi_message, NULL)) == NULL)
	{
		fprintf(stderr, "Error:  Cannot open MIDI input port \"%s\".\n", midi_in_port);
		exit(1);
	}

	MidiUtil_waitForInterrupt();
	rtmidi_close_port(midi_in);
	MidiUtilAlarm_free(alarm);
	return 0;
}
