/*
 * Copyright (c) 2018, Raphael Lehmann
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <modm/platform.hpp>
#include <modm/debug/logger.hpp>

#include <modm/driver/storage/block_device_heap.hpp>
#include <stdlib.h>

// Set the log level
#undef	MODM_LOG_LEVEL
#define	MODM_LOG_LEVEL modm::log::INFO

void printMemoryContent(const uint8_t* address, std::size_t size) {
	for (std::size_t i = 0; i < size; i++) {
		MODM_LOG_INFO.printf("%x", address[i]);
	}
}

int
main()
{
	/**
	 * This example/test writes alternating patterns into
	 * a `modm::BdHeap` block device with a size of 1M.
	 * The memory content is afterwards read and compared
	 * to the pattern.
	 * Write and read operations are done on 64 byte blocks.
	 */

	constexpr uint32_t BlockSize = 64;
	constexpr uint32_t MemorySize = 1024*1024;

	uint8_t bufferA[BlockSize];
	uint8_t bufferB[BlockSize];
	uint8_t bufferC[BlockSize];
	std::memset(bufferA, 0xAA, BlockSize);
	std::memset(bufferB, 0x55, BlockSize);

	modm::BdHeap<MemorySize> storageDevice;
	if(!RF_CALL_BLOCKING(storageDevice.initialize())) {
		MODM_LOG_INFO << "Error: Unable to initialize device.";
		exit(1);
	}
	RF_CALL_BLOCKING(storageDevice.erase(0, MemorySize));

	MODM_LOG_INFO << "Starting memory test!" << modm::endl;

	for(uint16_t iteration = 0; iteration < 1000; iteration++) {
		uint8_t* pattern = (iteration % 2 == 0) ? bufferA : bufferB;

		for(uint32_t i = 0; i < MemorySize; i += BlockSize) {
			if(!RF_CALL_BLOCKING(storageDevice.write(pattern, i, BlockSize))) {
				MODM_LOG_INFO << "Error: Unable to write data.";
				exit(1);
			}
		}

		for(uint32_t i = 0; i < MemorySize; i += BlockSize) {
			if(!RF_CALL_BLOCKING(storageDevice.read(bufferC, i, BlockSize))) {
				MODM_LOG_INFO << "Error: Unable to read data.";
				exit(1);
			}
			else if(std::memcmp(pattern, bufferC, BlockSize)) {
				MODM_LOG_INFO << "Error: Read '";
				printMemoryContent(bufferC, BlockSize);
				MODM_LOG_INFO << "', expected: '";
				printMemoryContent(pattern, BlockSize);
				MODM_LOG_INFO << "'." << modm::endl;
				exit(1);
			}
		}
		MODM_LOG_INFO << ".";
	}

	MODM_LOG_INFO << modm::endl << "Finished!" << modm::endl;

	return 0;
}
