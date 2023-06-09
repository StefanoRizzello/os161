/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Synchronization primitives.
 * The specifications of the functions are in synch.h.
 */

#include <types.h>
#include <lib.h>
#include <spinlock.h>
#include <wchan.h>
#include <thread.h>
#include <current.h>
#include <synch.h>

////////////////////////////////////////////////////////////
//
// Semaphore.

struct semaphore *
sem_create(const char *name, unsigned initial_count)
{
        struct semaphore *sem;

        sem = kmalloc(sizeof(*sem));
        if (sem == NULL) {
                return NULL;
        }

        sem->sem_name = kstrdup(name);
        if (sem->sem_name == NULL) {
                kfree(sem);
                return NULL;
        }

	sem->sem_wchan = wchan_create(sem->sem_name);
	if (sem->sem_wchan == NULL) {
		kfree(sem->sem_name);
		kfree(sem);
		return NULL;
	}

	spinlock_init(&sem->sem_lock);
        sem->sem_count = initial_count;

        return sem;
}

void
sem_destroy(struct semaphore *sem)
{
        KASSERT(sem != NULL);

	/* wchan_cleanup will assert if anyone's waiting on it */
	spinlock_cleanup(&sem->sem_lock);
	wchan_destroy(sem->sem_wchan);
        kfree(sem->sem_name);
        kfree(sem);
}

void
P(struct semaphore *sem)
{
        KASSERT(sem != NULL);

        /*
         * May not block in an interrupt handler.
         *
         * For robustness, always check, even if we can actually
         * complete the P without blocking.
         */
        KASSERT(curthread->t_in_interrupt == false);

	/* Use the semaphore spinlock to protect the wchan as well. */
	spinlock_acquire(&sem->sem_lock);
        while (sem->sem_count == 0) {
		/*
		 *
		 * Note that we don't maintain strict FIFO ordering of
		 * threads going through the semaphore; that is, we
		 * might "get" it on the first try even if other
		 * threads are waiting. Apparently according to some
		 * textbooks semaphores must for some reason have
		 * strict ordering. Too bad. :-)
		 *
		 * Exercise: how would you implement strict FIFO
		 * ordering?
		 */
		wchan_sleep(sem->sem_wchan, &sem->sem_lock);
        }
        KASSERT(sem->sem_count > 0);
        sem->sem_count--;
	spinlock_release(&sem->sem_lock);
}

void
V(struct semaphore *sem)
{
        KASSERT(sem != NULL);

	spinlock_acquire(&sem->sem_lock);

        sem->sem_count++;
        KASSERT(sem->sem_count > 0);
	wchan_wakeone(sem->sem_wchan, &sem->sem_lock);

	spinlock_release(&sem->sem_lock);
}

////////////////////////////////////////////////////////////
//
// Lock.

struct lock *
lock_create(const char *name)
{
        struct lock *lock;

        lock = kmalloc(sizeof(*lock));
        if (lock == NULL) {
                return NULL;
        }

        lock->lk_name = kstrdup(name);
        if (lock->lk_name == NULL) {
                kfree(lock);
                return NULL;
        }

	HANGMAN_LOCKABLEINIT(&lock->lk_hangman, lock->lk_name);

        // add stuff here as needed
        lock->wchan_create = wchan_create(lock->lk_name);	//In this way I create the wait channel
        if (lock->wchan_create == NULL){
        	kfree(lock->lk_name);
        	kfree(lock);
        	return NULL;
        }
        spinlock_init(&lock->my_spinlock);	//This is the initialization of spinlock
	lock->flag_lock = 0;	//This is the initialization of flag_lock

        return lock;
}

void
lock_destroy(struct lock *lock)
{
        KASSERT(lock != NULL);

        // add stuff here as needed
        //Here I do the opposite of lock_create
        spinlock_cleanup(&lock->my_spinlock);
        wchan_destroy(lock->wchan_create);

        kfree(lock->lk_name);
        kfree(lock);
}

void
lock_acquire(struct lock *lock)
{
	/* Call this (atomically) before waiting for a lock */
	//HANGMAN_WAIT(&curthread->t_hangman, &lock->lk_hangman);

        // Write this
        //Start of the Critical section
        KASSERT(lock != NULL);
        spinlock_acquire(&lock->my_spinlock);	//In this way I disable the interrupts
	while (lock->flag_lock == 1){
		wchan_sleep(lock->wchan_create, &lock->my_spinlock);
	}
	lock->flag_lock = 1;
	//End of the Critical section
	spinlock_release(&lock->my_spinlock);	//In this way I enable the interrupts

        //void)lock;  // suppress warning until code gets written

	/* Call this (atomically) once the lock is acquired */
	//HANGMAN_ACQUIRE(&curthread->t_hangman, &lock->lk_hangman);
}

void
lock_release(struct lock *lock)
{
	/* Call this (atomically) when the lock is released */
	//HANGMAN_RELEASE(&curthread->t_hangman, &lock->lk_hangman);

        // Write this

	//(void)lock;  // suppress warning until code gets written
        
        KASSERT(lock != NULL);
        //Start of the Critical section
        spinlock_acquire(&lock->my_spinlock);	//In this way I disable the interrupts
        lock->flag_lock = 0;
        wchan_wakeone(lock->wchan_create, &lock->my_spinlock); //I choose wakeone because if all the threads are sleeping, I wake up only a thread to continue its execution
	//End of the Critical section
	spinlock_release(&lock->my_spinlock);	//In this way I enable the interrupts
}

bool
lock_do_i_hold(struct lock *lock)
{
        // Write this

        //(void)lock;  // suppress warning until code gets written

        //return true; // dummy until code gets written
        
        KASSERT(lock != NULL);
        if(lock->flag_lock == 1){
        	return true;
        }
        else{
        	return false;
        }
}

////////////////////////////////////////////////////////////
//
// CV


struct cv *
cv_create(const char *name)
{
        struct cv *cv;

        cv = kmalloc(sizeof(*cv));
        if (cv == NULL) {
                return NULL;
        }

        cv->cv_name = kstrdup(name);
        if (cv->cv_name==NULL) {
                kfree(cv);
                return NULL;
        }

        // add stuff here as needed
        cv->wchan_create = wchan_create(cv->cv_name);	//In this way I create the wait channel
        if (cv->wchan_create == NULL){
        	kfree(cv->cv_name);
        	kfree(cv);
        	return NULL;
        }
        spinlock_init(&cv->my_spinlock);	//This is the initialization of spinlock

        return cv;
}

void
cv_destroy(struct cv *cv)
{
        KASSERT(cv != NULL);

        // add stuff here as needed
        spinlock_cleanup(&cv->my_spinlock);
        wchan_destroy(cv->wchan_create);

        kfree(cv->cv_name);
        kfree(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
        // Write this
        //(void)cv;    // suppress warning until code gets written
        //(void)lock;  // suppress warning until code gets written
        KASSERT(cv != NULL);
        KASSERT(lock_do_i_hold(lock) == true);
        spinlock_acquire(&cv->my_spinlock);
        lock_release(lock);
        wchan_sleep(cv->wchan_create, &cv->my_spinlock);
        spinlock_release(&cv->my_spinlock);
        lock_acquire(lock);
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
        // Write this
	//(void)cv;    // suppress warning until code gets written
	//(void)lock;  // suppress warning until code gets written
	KASSERT(cv != NULL);
        KASSERT(lock_do_i_hold(lock) == true);
	spinlock_acquire(&cv->my_spinlock);
	wchan_wakeone(cv->wchan_create, &cv->my_spinlock);
	spinlock_release(&cv->my_spinlock);
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	// Write this
	//(void)cv;    // suppress warning until code gets written
	//(void)lock;  // suppress warning until code gets written
	KASSERT(cv != NULL);
        KASSERT(lock_do_i_hold(lock) == true);
	spinlock_acquire(&cv->my_spinlock);
	wchan_wakeall(cv->wchan_create, &cv->my_spinlock);
	spinlock_release(&cv->my_spinlock);
}
