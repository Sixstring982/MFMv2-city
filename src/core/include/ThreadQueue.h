/*                                              -*- mode:C++ -*-
  ThreadQueue.h Thread-Safe byte buffer
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file ThreadQueue.h Thread-Safe byte buffer
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef QUE_H
#define QUE_H

#include "itype.h"
#include <pthread.h>

#define THREADQUEUE_MAX_BYTES (1536 + 1)  // non-multiple of 16 for testing

namespace MFM
{
  /**
   * A thread-safe queue implementation, backed by a byte buffer of
   * fixed length.
   */
  class ThreadQueue
  {
  private:

    /**
     * The pthread_mutex_t required to lock this ThreadQueue upon
     * reading and writing.
     */
    pthread_mutex_t m_lock;

    /**
     * The pthread_cond_t required to wake a waiting thread during an
     * IO conflict.
     */
    pthread_cond_t m_cond;

    /**
     * The actual data held within this ThreadQueue.
     */
    u8 m_queueData[THREADQUEUE_MAX_BYTES];

    /**
     * The location within m_queueData which the next byte received
     * through Write() will be written to.
     *
     * @sa Write
     */
    u32 m_writeHead;

    /**
     * The location within m_queueData which the next byte asked for
     * by Read() will be read from.
     *
     * @sa Read
     */
    u32 m_readHead;

    /**
     * The number of bytes currently available to be read from
     * m_queueData.
     */
    u32 m_heldBytes;

    /**
     * Reads a specified number of bytes from this ThreadQueue into a
     * specified buffer. These bytes are taken directly from the front
     * of the underlying queue. This does not block, and instead reads
     * as many bytes as possible from the underlying queue, up to the
     * specified limit, before returning.
     *
     * ****************NOTICE*******************
     * This function ignores the mutex and is meant to be for internal
     * use only!
     * *****************NOTICE*******************
     *
     * @param bytes The buffer to read bytes into. This buffer will be
     *              overwritten during this call.
     *
     * @param length The number of bytes to read from the front of the
     *               underlying queue.
     *
     */
    u32 UnsafeRead(u8* bytes, u32 length);

  public:

    /**
     * Constructs a new ThreadQueue, setting up all mutex related
     * structures.
     */
    ThreadQueue();

    /**
     * Deconstructs a ThreadQueue, destroying all mutexes.
     */
    ~ThreadQueue();

    /**
     * Writes a specified number of bytes to this ThreadQueue from a
     * specified buffer. If writing is not available, this thread
     * waits until it is.
     *
     * @param bytes The buffer where bytes will be taken from during
     *              writing.
     *
     * @param length The number of bytes which will be taken from
     *               bytes. The caller must ensure that this number is
     *               less than or equal to the length of bytes.
     */
    void Write(u8* bytes, u32 length);

    /**
     * Reads a specified number of bytes from this ThreadQueue into a
     * specified buffer. These bytes are taken directly from the front
     * of the underlying queue. This blocks the calling thread until
     * all bytes have been read.
     *
     * @param bytes The buffer to read bytes into. This buffer will be
     *              overrwritten during this call.
     *
     * @param length The number of bytes to read from the front of the
     *               underlying queue.
     */
    void ReadBlocking(u8* bytes, u32 length);

    /**
     * Reads a specified number of bytes from this ThreadQueue into a
     * specified buffer. These bytes are taken directly from the front
     * of the underlying queue. This does not block, and instead reads
     * as many bytes as possible from the underlying queue, up to the
     * specified limit, before returning.
     *
     * @param bytes The buffer to read bytes into. This buffer will be
     *              overrwritten during this call.
     *
     * @param length The number of bytes to read from the front of the
     *               underlying queue.
     *
     * @returns The number of bytes read successfully from this call.
     */
    u32 Read(u8* bytes, u32 length);

    /**
     * Writes a series of held bytes to a specified buffer. This FAILs
     * with ARRAY_INDEX_OUT_OF_BOUNDS if reading will go over the
     * bounds of this ThreadQueue .
     *
     * @param toBuffer The buffer to read bytes from this ThreadQueue
     *                 into.
     *
     * @param index The byte index to begin reading from.
     *
     * @param length The number of bytes to read from this ThreadQueue .
     */
    void PeekRead(u8* toBuffer, u32 index, u32 length);

    /**
     * Gets the number of bytes currently ready to be read from the
     * underlying queue.
     *
     * @returns The number of bytes ready to be read from the
     *          underlying queue.
     */
    u32 BytesAvailable() ;

    /**
     * Erases all held data within this TheadQueue. This should only
     * be called in erronous cases where the ThreadQueue should no
     * longer need to hold any data inside. This should NEVER be used
     * in tandem with a call to ReadBlocking; this will create a
     * deadlock.
     */
    void Flush();

  };
}

#endif /* QUE_H */
