/*
 * Copyright (c) 2011, Georgi Grinshpun
 * Copyright (c) 2011-2012, Fabian Greif
 * Copyright (c) 2012, 2014, Sascha Schade
 * Copyright (c) 2013, 2015, Kevin Läufer
 * Copyright (c) 2013, 2015-2017, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <modm/board.hpp>

// ----------------------------------------------------------------------------
/**
 * Very basic example of USART usage.
 * The ASCII sequence 'A', 'B', 'C, ... , 'Z', 'A', 'B', 'C', ...
 * is printed with 9600 baud, 8N1 at pin PA9.
 */
int
main()
{
	Board::initialize();

	Board::LedUp::set();

	// Enable USART 1
	Usart1::connect<GpioOutputA9::Tx>();
	Usart1::initialize<Board::SystemClock, 9600_Bd>();

	while (true)
	{
		static uint8_t c = 'A';
		Board::LedUp::toggle();
		Board::LedDown::toggle();
		Usart1::write(c);
		++c;
		if (c > 'Z') {
			c = 'A';
		}
		modm::delay(500ms);
	}

	return 0;
}
