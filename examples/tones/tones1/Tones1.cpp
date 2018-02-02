/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

//TODO improve play to make it asynchronous?
// Imperial march tones thanks:
// http://processors.wiki.ti.com/index.php/Playing_The_Imperial_March

// Example of square wave generation, using CTC mode and COM toggle
#include <fastarduino/tones.h>
#include <fastarduino/time.h>
#include <fastarduino/utilities.h>

// Board-dependent settings
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
static constexpr const board::DigitalPin OUTPUT = board::PWMPin::D9_PB1_OC1A;

using devices::audio::Tone;

using GENERATOR = devices::audio::ToneGenerator<NTIMER, OUTPUT>;

struct TonePlay
{
	Tone tone;
	uint16_t ms;
};

static TonePlay music[] =
{
	// First part
	{Tone::A1, 500},
	{Tone::A1, 500},
	{Tone::A1, 500},
	{Tone::F1, 350},
	{Tone::C2, 150},
	{Tone::A1, 500},
	{Tone::F1, 350},
	{Tone::C2, 150},
	{Tone::A1, 650},
	{Tone::NONE, 150},

	// Second part
	{Tone::E2, 500},
	{Tone::E2, 500},
	{Tone::E2, 500},
	{Tone::F2, 350},
	{Tone::C2, 150},
	{Tone::Gs1, 500},
	{Tone::F1, 350},
	{Tone::C2, 150},
	{Tone::A1, 650},
	{Tone::NONE, 150},

	// Third part (repeated once)
	{Tone::REPEAT_START, 0},
	{Tone::A2, 500},
	{Tone::A1, 300},
	{Tone::A1, 150},
	{Tone::A2, 400},
	{Tone::Gs2, 200},
	{Tone::G2, 200},
	{Tone::Fs2, 125},
	{Tone::F2, 125},
	{Tone::Fs2, 250},
	{Tone::NONE, 250},

	{Tone::As1, 250},
	{Tone::Ds2, 400},
	{Tone::D2, 200},
	{Tone::Cs2, 200},
	{Tone::C2, 125},
	{Tone::B1, 125},
	{Tone::C2, 250},
	{Tone::NONE, 250},

	{Tone::F1, 125},
	{Tone::Gs1, 500},
	{Tone::F1, 375},
	{Tone::A1, 125},
	{Tone::C2, 500},
	{Tone::A1, 375},
	{Tone::C2, 125},
	{Tone::E2, 650},
	{Tone::NONE, 250},
	{Tone::REPEAT_END, 1},
};

static constexpr const size_t NUM_TONES = sizeof music / sizeof music[0];

int main() __attribute__((OS_main));
int main()
{
	sei();
	time::delay_ms(5000);

	GENERATOR generator;

	int16_t repeat_index = -1;
	int8_t repeat_times;
	for (size_t i = 0; i < NUM_TONES; ++i)
	{
		TonePlay tone = music[i];
		if (tone.tone == Tone::NONE)
			time::delay_ms(tone.ms);
		else if (tone.tone == Tone::REPEAT_START)
		{
			repeat_index = i;
			repeat_times = -1;
		}
		else if (tone.tone == Tone::REPEAT_END)
		{
			if (repeat_index != -1)
			{
				if (repeat_times == -1)
					repeat_times = tone.ms;
				if (repeat_times--)
					i = repeat_index;
				else
					repeat_index = -1;
			}
		}
		else
			generator.tone(tone.tone, tone.ms);
	}
}