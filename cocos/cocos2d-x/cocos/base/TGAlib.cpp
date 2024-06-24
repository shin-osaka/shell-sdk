/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "base/TGAlib.h"
#include "base/CCData.h"
#include "platform/CCFileUtils.h"

NS_CC_BEGIN

static bool tgaLoadRLEImageData(unsigned char* Buffer, unsigned long bufSize, tImageTGA *info);
void tgaFlipImage( tImageTGA *info );

bool tgaLoadHeader(unsigned char* buffer, unsigned long bufSize, tImageTGA *info)
{
    bool ret = false;

    do
    {
        size_t step = sizeof(unsigned char) * 2;
        CC_BREAK_IF((step + sizeof(unsigned char)) > bufSize);
        memcpy(&info->type, buffer + step, sizeof(unsigned char));

        step += sizeof(unsigned char) * 2;
        step += sizeof(signed short) * 4;
        CC_BREAK_IF((step + sizeof(signed short) * 2 + sizeof(unsigned char)) > bufSize);
        memcpy(&info->width, buffer + step, sizeof(signed short));
        memcpy(&info->height, buffer + step + sizeof(signed short), sizeof(signed short));
        memcpy(&info->pixelDepth, buffer + step + sizeof(signed short) * 2, sizeof(unsigned char));

        step += sizeof(unsigned char);
        step += sizeof(signed short) * 2;
        CC_BREAK_IF((step + sizeof(unsigned char)) > bufSize);
        unsigned char cGarbage;
        memcpy(&cGarbage, buffer + step, sizeof(unsigned char));

        info->flipped = 0;
        if ( cGarbage & 0x20 )
        {
            info->flipped = 1;
        }
        ret = true;
    } while (0);

    return ret;
}

bool tgaLoadImageData(unsigned char *Buffer, unsigned long bufSize, tImageTGA *info)
{
    bool ret = false;

    do
    {
        int mode,total,i;
        unsigned char aux;
        size_t step = (sizeof(unsigned char) + sizeof(signed short)) * 6;

        mode = info->pixelDepth / 8;
        total = info->height * info->width * mode;

        size_t dataSize = sizeof(unsigned char) * total;
        CC_BREAK_IF((step + dataSize) > bufSize);
        memcpy(info->imageData, Buffer + step, dataSize);

        if (mode >= 3)
        {
            for (i=0; i < total; i+= mode)
            {
                aux = info->imageData[i];
                info->imageData[i] = info->imageData[i+2];
                info->imageData[i+2] = aux;
            }
        }

        ret = true;
    } while (0);

    return ret;
}

static bool tgaLoadRLEImageData(unsigned char* buffer, unsigned long bufSize, tImageTGA *info)
{
    unsigned int mode,total,i, index = 0;
    unsigned char aux[4], runlength = 0;
    unsigned int skip = 0, flag = 0;
    size_t step = (sizeof(unsigned char) + sizeof(signed short)) * 6;

    mode = info->pixelDepth / 8;
    total = info->height * info->width;

    for( i = 0; i < total; i++ )
    {
        if ( runlength != 0 )
        {
            runlength--;
            skip = (flag != 0);
        }
        else
        {
            CC_BREAK_IF((step + sizeof(unsigned char)) > bufSize);
            memcpy(&runlength, buffer + step, sizeof(unsigned char));
            step += sizeof(unsigned char);

            flag = runlength & 0x80;
            if ( flag )
            {
                runlength -= 128;
            }
            skip = 0;
        }

        if ( !skip )
        {
            CC_BREAK_IF((step + sizeof(unsigned char) * mode) > bufSize);

            memcpy(aux, buffer + step, sizeof(unsigned char) * mode);
            step += sizeof(unsigned char) * mode;

            if ( mode >= 3 )
            {
                unsigned char tmp;

                tmp = aux[0];
                aux[0] = aux[2];
                aux[2] = tmp;
            }
        }

        memcpy(&info->imageData[index], aux, mode);
        index += mode;
    }

    return true;
}

void tgaFlipImage( tImageTGA *info )
{
    int mode = info->pixelDepth / 8;
    int rowbytes = info->width*mode;
    unsigned char *row = (unsigned char *)malloc(rowbytes);
    int y;

    if (row == nullptr) return;

    for( y = 0; y < (info->height/2); y++ )
    {
        memcpy(row, &info->imageData[y*rowbytes],rowbytes);
        memcpy(&info->imageData[y*rowbytes], &info->imageData[(info->height-(y+1))*rowbytes], rowbytes);
        memcpy(&info->imageData[(info->height-(y+1))*rowbytes], row, rowbytes);
    }

    free(row);
    info->flipped = 0;
}

tImageTGA* tgaLoadBuffer(unsigned char* buffer, long size)
{
    int mode,total;
    tImageTGA *info = nullptr;

    do
    {
        CC_BREAK_IF(! buffer);
        info = (tImageTGA *)malloc(sizeof(tImageTGA));

        if (! tgaLoadHeader(buffer, size, info))
        {
            info->status = TGA_ERROR_MEMORY;
            break;
        }

        if (info->type == 1)
        {
            info->status = TGA_ERROR_INDEXED_COLOR;
            break;
        }

        if ((info->type != 2) && (info->type !=3) && (info->type !=10) )
        {
            info->status = TGA_ERROR_COMPRESSED_FILE;
            break;
        }

        mode = info->pixelDepth / 8;
        total = info->height * info->width * mode;
        info->imageData = (unsigned char *)malloc(sizeof(unsigned char) * total);

        if (info->imageData == nullptr)
        {
            info->status = TGA_ERROR_MEMORY;
            break;
        }

        bool bLoadImage = false;
        if ( info->type == 10 )
        {
            bLoadImage = tgaLoadRLEImageData(buffer, size, info);
        }
        else
        {
            bLoadImage = tgaLoadImageData(buffer, size, info);
        }

        if (! bLoadImage)
        {
            info->status = TGA_ERROR_READING_FILE;
            break;
        }
        info->status = TGA_OK;

        if ( info->flipped )
        {
            tgaFlipImage( info );
            if ( info->flipped )
            {
                info->status = TGA_ERROR_MEMORY;
            }
        }
    } while(0);

    return info;
}

tImageTGA * tgaLoad(const char *filename)
{
    Data data = FileUtils::getInstance()->getDataFromFile(filename);

    if (!data.isNull())
    {
        return tgaLoadBuffer(data.getBytes(), data.getSize());
    }

    return nullptr;
}

void tgaRGBtogreyscale(tImageTGA *info) {

    int mode,i,j;

    unsigned char *newImageData;

    if (info->pixelDepth == 8)
        return;

    mode = info->pixelDepth / 8;

    newImageData = (unsigned char *)malloc(sizeof(unsigned char) *
                                           info->height * info->width);
    if (newImageData == nullptr) {
        return;
    }

    for (i = 0,j = 0; j < info->width * info->height; i +=mode, j++)
        newImageData[j] =
        (unsigned char)(0.30 * info->imageData[i] +
                        0.59 * info->imageData[i+1] +
                        0.11 * info->imageData[i+2]);


    free(info->imageData);

    info->pixelDepth = 8;
    info->type = 3;
    info->imageData = newImageData;
}

void tgaDestroy(tImageTGA *info) {

    if (info != nullptr) {
        if (info->imageData != nullptr)
        {
            free(info->imageData);
        }

        free(info);
    }
}
NS_CC_END
