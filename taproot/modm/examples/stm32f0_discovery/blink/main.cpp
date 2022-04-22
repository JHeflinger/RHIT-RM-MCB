/*
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

int
main()
{
	Board::initialize();

	LedGreen::set();
	LedBlue::reset();

	while (true) {
		LedGreen::toggle();
		LedBlue::toggle();
		modm::delay(Board::Button::read() ? 0.5s : 1s);
	}
}
