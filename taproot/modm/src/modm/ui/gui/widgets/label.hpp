/*
 * Copyright (c) 2014, Daniel Krebs
 * Copyright (c) 2014, Niklas Hauser
 * Copyright (c) 2014, Sascha Schade
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#ifndef MODM_GUI_LABEL_HPP
#define MODM_GUI_LABEL_HPP

#include "widget.hpp"

namespace modm
{

namespace gui
{

/**
 * @ingroup modm_ui_gui
 * @author	Daniel Krebs
 */
class Label : public Widget
{
public:
	Label(const char* lbl, modm::color::Rgb565 color) :
		Widget(Dimension(0,0), false),
		label(lbl),
		color(color)
	{
		this->updateDimension();
	}

	void
	render(View* view);

	void
	setColor(modm::color::Rgb565 color)
	{
		this->color = color;
		this->markDirty();
	}

	void
	setLabel(const char* lbl)
	{
		this->label = lbl;
		this->updateDimension();
		this->markDirty();
	}

	void
	setFont(const modm::accessor::Flash<uint8_t> *font)
	{
		this->font = *font;
		this->updateDimension();
	}

	void
	setFont(const uint8_t *newFont)
	{
		this->font = modm::accessor::asFlash(newFont);
		this->updateDimension();
	}

private:
	void
	updateDimension()
	{
		// Update label dimension
		if(this->font.isValid())
		{
			this->dimension.width = modm::ColorGraphicDisplay::getStringWidth(this->label, &(this->font));
			this->dimension.height = modm::ColorGraphicDisplay::getFontHeight(&(this->font));
		}
	}

private:
	const char* label;
	modm::color::Rgb565 color;
};

}	// namespace gui

}	// namespace modm

#endif  // MODM_GUI_LABEL_HPP
