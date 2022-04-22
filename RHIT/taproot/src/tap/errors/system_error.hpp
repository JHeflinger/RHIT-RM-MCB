/*
 * Copyright (c) 2020-2021 Advanced Robotics at the University of Washington <robomstr@uw.edu>
 *
 * This file is part of Taproot.
 *
 * Taproot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Taproot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Taproot.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SYSTEM_ERROR_HPP_
#define SYSTEM_ERROR_HPP_

namespace tap::errors
{
class SystemError
{
public:
    constexpr SystemError() : lineNumber(0), description("default"), filename("none") {}

    constexpr SystemError(const char *desc, int line, const char *file)
        : lineNumber(line),
          description(desc),
          filename(file)
    {
    }

    constexpr int getLineNumber() const { return lineNumber; }

    const char *getDescription() const { return description; }

    const char *getFilename() const { return filename; }

private:
    int lineNumber;

    const char *description;

    const char *filename;
};  // class SystemError
}  // namespace tap::errors

#endif  // SYSTEM_ERROR_HPP_
