#include "core/Semaphore.h"

/*
 * @brief Semaphore basic constructor
 */
Semaphore::Semaphore()
{
	printk("Create Semaphore %p\n", this);
	k_sem_init(&_sema_internal, 0, SEMAPHORE_MAX_TAKE);
}

Semaphore::Semaphore(int initial_take_count)
{
	printk("Create Semaphore %p\n", this);
	k_sem_init(&_sema_internal, initial_take_count, SEMAPHORE_MAX_TAKE);
}
/*
 * @brief wait for a Semaphore
 *
 * Test a Semaphore to see if it has been signaled.  If the signal
 * count is greater than zero, it is decremented.
 *
 * @return 1 when Semaphore is available
 */
int Semaphore::wait(void)
{
	k_sem_take(&_sema_internal, K_FOREVER);
	return 1;
}

/*
 * @brief wait for a Semaphore within a specified timeout
 *
 * Test a Semaphore to see if it has been signaled.  If the signal
 * count is greater than zero, it is decremented. The function
 * waits for timeout specified
 *
 * @param timeout the specified timeout in ticks
 *
 * @return 1 if Semaphore is available, 0 if timed out
 */
int Semaphore::wait(int timeout)
{
	return k_sem_take(&_sema_internal, K_MSEC(timeout));
}

/**
 * @brief Signal a Semaphore
 *
 * This routine signals the specified Semaphore.
 */
void Semaphore::give(void)
{
	k_sem_give(&_sema_internal);
}