/*
 * Copyright (c) 2010-2011, Fabian Greif
 * Copyright (c) 2013-2014, 2016-2017, Niklas Hauser
 * Copyright (c) 2014, Sascha Schade
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <modm/platform.hpp>
#include <modm/architecture/interface/interrupt.hpp>
#include <modm/ui/button_group.hpp>

using namespace modm::platform;

static modm::ButtonGroup<> buttons(0);
enum
{
	BUTTON_1 = 0x01
};
typedef GpioOutputB0 Led;
typedef GpioInputB3 Button;

// Timer interrupt used to query the button status
MODM_ISR(TIMER2_COMPA)
{
	buttons.update(Button::read() ? BUTTON_1 : 0);
}

int
main()
{
	Led::setOutput();
	Led::reset();

	TCCR2A = (1 << WGM21);
	TCCR2B = (1 << CS22);
	TIMSK2 = (1 << OCIE2A);
	OCR2A = 23;			// 10 ms at 14.7456 MHz

	// set PB3 as input with pullup
	Button::setInput(Gpio::InputType::PullUp);

	enableInterrupts();

	while (true)
	{
		if (buttons.isPressed(BUTTON_1))
		{
			Led::toggle();
		}
	}
}
