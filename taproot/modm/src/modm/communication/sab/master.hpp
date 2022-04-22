/*
 * Copyright (c) 2009-2011, Fabian Greif
 * Copyright (c) 2010, Martin Rosekeit
 * Copyright (c) 2012-2013, Sascha Schade
 * Copyright (c) 2012-2013, 2015, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#ifndef MODM_SAB_MASTER_HPP
#define MODM_SAB_MASTER_HPP

#include <stdint.h>
#include <modm/processing/timer.hpp>

#include "interface.hpp"

namespace modm
{
	namespace sab
	{
		/**
		 *
		 * Requires modm::Clock to be implemented.
		 *
		 * \see	modm::Clock
		 *
		 * \author	Fabian Greif
		 * \ingroup modm_communication_sab
		 */
		template <typename Interface>
		class Master
		{
		public:
			static void
			initialize();

			/**
			 * \brief	Start a new query with a payload
			 *
			 * \param slaveAddress
			 * \param command
			 * \param payload
			 * \param responseLength	Expected payload length of the response
			 */
			template <typename T>
			static void
			query(uint8_t slaveAddress, uint8_t command,
					const T& payload, uint8_t responseLength);


			static void
			query(uint8_t slaveAddress, uint8_t command,
				  const void *payload, uint8_t payloadLength, uint8_t responseLength);

			/**
			 * \brief	Start a new query without any payload
			 *
			 */
			static void
			query(uint8_t slaveAddress, uint8_t command, uint8_t responseLength);

			static bool
			isQueryCompleted();

			/**
			 * \brief	Check if the last query could be performed successfully
			 *
			 * Only valid if isQueryCompleted() returns \c true.
			 *
			 * \return	\c true if the query was successful. Use getResponse() to
			 * 			access the result.
			 */
			static bool
			isSuccess();

			/**
			 * \brief	Check error code
			 *
			 * Only valid if isQueryCompleted() returns \c true while
			 * isSuccess() returns \c false.
			 *
			 * \return	Error code
			 * \see		modm::sab::Error
			 */
			static uint8_t
			getErrorCode();


			template <typename T>
			static inline const T *
			getResponse();

			static inline const uint8_t *
			getResponse();


			static void
			update();

		protected:
			enum QueryStatus
			{
				IN_PROGRESS,			///< Query in progress
				SUCCESS,				///< Response successfully received
				ERROR_RESPONSE = 0x40,	///< Error in the received message
				ERROR_TIMEOUT = 0x41,	///< No message received within the timeout window
				ERROR_PAYLOAD = 0x42,	///< Wrong payload size
			};

			static Interface interface;

			static QueryStatus queryStatus;
			static uint8_t expectedResponseLength;
			static modm::ShortTimeout timer;
			/// timeout value in milliseconds
			static constexpr std::chrono::milliseconds timeout{10};
		};
	}
}

#include "master_impl.hpp"

#endif	// MODM_SAB_MASTER_HPP
