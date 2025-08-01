/**
 * 2025-05-20
 *
 * The MIT License (MIT)
 * Copyright (c) 2025 K. Suwatchai (Mobizt)
 *
 *
 * Permission is hereby granted, free of charge, to any person returning a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef DATABASE_DATA_OPTIONS_H
#define DATABASE_DATA_OPTIONS_H

#include <Arduino.h>
#include "./database/DatabaseFilter.h"
#include "./core/Utils/StringUtil.h"

#if defined(ENABLE_DATABASE)
class DatabaseOptions
{
    friend class RealtimeDatabase;

private:
    StringUtil sut;

public:
    uint32_t readTimeout = 0;
    String writeSizeLimit;
    bool shallow = false, silent = false;
    DatabaseFilter filter;

    void copy(const DatabaseOptions &rhs)
    {
        this->readTimeout = rhs.readTimeout;
        this->writeSizeLimit = rhs.writeSizeLimit;
        this->shallow = rhs.shallow;
        this->filter.copy(rhs.filter);
    }

    void clear()
    {
        readTimeout = 0;
        sut.clear(writeSizeLimit);
        shallow = false;
        silent = false;
        filter.clear();
    }

private:
    bool base64 = false, ota = false;
};
#endif
#endif