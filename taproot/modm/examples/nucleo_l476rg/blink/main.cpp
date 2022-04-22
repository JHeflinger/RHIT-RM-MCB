/*
 * Copyright (c) 2017, Niklas Hauser
 * Copyright (c) 2017, Sascha Schade
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <modm/board.hpp>

using namespace Board;

// ----------------------------------------------------------------------------
int
main()
{
	initialize();

	// Output MSI Clock at MCO
	// 48 MHz / 16 = 3 MHz expected at PA8
	Rcc::enableClockOutput(
		Rcc::ClockOutputSource::MultiSpeedInternalClock,
		Rcc::ClockOutputPrescaler::Div16);

	GpioConnector<Peripheral::Rcc, GpioA8::Mco>::connect();

	LedGreen::set();

	while (true)
	{
		LedGreen::toggle();
		modm::delay(Button::read() ? 250ms : 500ms);
	}

	return 0;
}
