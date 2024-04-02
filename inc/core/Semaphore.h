/*
 * Copyright (c) 2015-2016 Wind River Systems, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/sys/printk.h>

/* specify delay between greetings (in ms); compute equivalent in ticks */
#define SLEEPTIME  500
#define SEMAPHORE_MAX_TAKE 1
/*
 * @class cpp_semaphore
 * @brief Semaphore
 *
 * Class derives from the pure virtual semaphore class and
 * implements it's methods for the semaphore
 */
class Semaphore {
protected:
	struct k_sem _sema_internal;
public:
	Semaphore();
	Semaphore(int initial_take_count);
	~Semaphore() {}
	int wait(void);
	int wait(int timeout);
	void give(void);
};