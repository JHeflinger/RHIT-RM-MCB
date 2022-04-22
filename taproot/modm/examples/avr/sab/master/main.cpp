/*
 * Copyright (c) 2010-2011, Fabian Greif
 * Copyright (c) 2013, 2016-2017, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <modm/platform.hpp>
#include <modm/communication/sab/interface.hpp>

using namespace modm::platform;

int
main()
{
	modm::sab::Interface< Uart0 > interface;

	// set baudrate etc.
	interface.initialize();

	// enable interrupts
	enableInterrupts();

	while (true)
	{
		// decode received messages
		interface.update();

		// ... TODO ...
	}
}
