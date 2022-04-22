/*
 * Copyright (c) 2017, Carl Treudler
 * Copyright (c) 2017, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <modm/board.hpp>

int main()
{
	Board::initialize();
	Board::LedD3::setOutput();


	while (true)
	{
		Board::LedD3::toggle();
		modm::delay(200ms);
	}
	return 0;
}
