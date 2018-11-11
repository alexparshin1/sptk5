/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CPngImage.cpp - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/sptk.h>

#include <FL/fl_draw.H>
#include <sptk5/gui/CPngImage.h>
#include <png.h>

using namespace sptk;
using namespace std;

typedef struct
{
    unsigned read_offset;
    const Buffer* buffer;
} CMemHandle;

static void png_read(png_structp pp, png_bytep buf, png_size_t len)
{
    auto p = (CMemHandle*) png_get_io_ptr(pp);
    const Buffer* buffer = p->buffer;
    png_size_t tail = buffer->bytes() - p->read_offset;
    if (len > tail)
        len = tail;
    memcpy(buf, p->buffer->data() + p->read_offset, len);
    p->read_offset += (int) len;
}

void CPngImage::load(const Buffer& imagedata)
{
    int i;
    int channels;
    png_structp pp; // PNG read pointer
    png_infop info; // PNG info pointer
    png_bytep* rows; // PNG row pointers
    CMemHandle p;
    //png_byte junk[16];

    pp = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    info = png_create_info_struct(pp);

    p.read_offset = 0;
    p.buffer = &imagedata;

    png_set_read_fn(pp, (void*) &p, png_read);
    png_read_info(pp, info);

    int color_type = png_get_color_type(pp, info);
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_expand(pp);
    if (color_type & PNG_COLOR_MASK_COLOR)
        channels = 3;
    else
        channels = 1;

    //bool            alpha = false;
    int num_trans = 0;
    png_color_16p trans_color;
    png_bytep trans_alpha;
    png_get_tRNS(pp, info, &trans_alpha, &num_trans, &trans_color);
    if ((color_type & PNG_COLOR_MASK_ALPHA) || num_trans) {
        channels++;
        //alpha = true;
    }

    w((int) png_get_image_width(pp, info));
    h((int) png_get_image_height(pp, info));
    d(channels);

    int bit_depth = png_get_bit_depth(pp, info);
    if (bit_depth < 8) {
        png_set_packing(pp);
        png_set_expand(pp);
    } else if (bit_depth == 16)
        png_set_strip_16(pp);

#if defined(HAVE_PNG_GET_VALID) && defined(HAVE_PNG_SET_TRNS_TO_ALPHA)
    // Handle transparency...
    if (png_get_valid(pp, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(pp);
#endif // HAVE_PNG_GET_VALID && HAVE_PNG_SET_TRNS_TO_ALPHA

    array = new uchar[w() * h() * d()];
    alloc_array = 1;

    // Allocate pointers...
    rows = new png_bytep[h()];

    for (i = 0; i < h(); i++)
        rows[i] = (png_bytep) (array + i * w() * d());

    // Read the image, handling interlacing as needed...
    for (i = png_set_interlace_handling(pp); i > 0; i--)
        png_read_rows(pp, rows, nullptr, h());

#ifdef WIN32
    // Some Windows graphics drivers don't honor transparency when RGB == white
    if (channels == 4) {
        // Convert RGB to 0 when alpha == 0...
    uchar *ptr = (uchar *)array;
    for (i = w() * h(); i > 0; i --, ptr += 4)
        if (!ptr[3])
            ptr[0] = ptr[1] = ptr[2] = 0;
    }
#endif // WIN32

    // Free memory and return...
    delete[] rows;

    png_destroy_read_struct(&pp, &info, nullptr);
}

CPngImage::CPngImage(const Buffer& imagedata)
        : Fl_RGB_Image(nullptr, 0, 0)
{
    load(imagedata);
}

CPngImage::CPngImage(const Fl_RGB_Image* image)
        : Fl_RGB_Image(nullptr, 0, 0)
{
    unsigned arraySize = unsigned(image->w() * image->h() * image->d());
    array = new uchar[arraySize];
    alloc_array = 1;
    memcpy((void*) array, image->array, arraySize);
    w(image->w());
    h(image->h());
    d(image->d());
    ld(image->ld());
}

CPngImage::CPngImage(const String& fileName)
: Fl_RGB_Image(nullptr, 0, 0)
{
    try {
        Buffer imageData;
        imageData.loadFromFile(fileName);
        load(imageData);
    } catch (...) {}
}

static Fl_RGB_Image*
subRGBImage(Fl_RGB_Image* image, unsigned offsetX, unsigned offsetY, unsigned width, unsigned height)
{
    if (offsetX > (unsigned) image->w())
        offsetX = (unsigned) image->w();
    if (offsetY > (unsigned) image->h())
        offsetY = (unsigned) image->h();
    if (offsetX + width > (unsigned) image->w())
        width = (unsigned) image->w() - offsetX;
    if (offsetY + height > (unsigned) image->h())
        height = (unsigned) image->h() - offsetY;
    unsigned bytesPerRow = image->w() * image->d();
    unsigned newBytesPerRow = width * image->d();
    unsigned totalBytes = height * newBytesPerRow;
    const uchar* imageArray = image->array;
    auto newArray = new uchar[totalBytes];
    auto newImage = new Fl_RGB_Image(newArray, width, height, image->d());
    newImage->alloc_array = 1;
    //unsigned maxRow = offsetY + height;
    //unsigned maxCol = offsetX + width;

    const uchar* imageRow = imageArray + offsetY * bytesPerRow + offsetX * image->d();
    for (unsigned row = 0; row < height; row++) {
        memcpy(newArray, imageRow, newBytesPerRow);
        newArray += newBytesPerRow;
        imageRow += bytesPerRow;
    }
    return newImage;
}

void CPngImage::cutStretchDraw(
        CPngImage* sourceImage, int srcX, int srcY, int srcW, int srcH, int destX, int destY, int destW, int destH)
{
    if (destW <= 0 || destH <= 0) return;
    Fl_RGB_Image* img = subRGBImage(sourceImage, srcX, srcY, srcW, srcH);
    auto stretched = (Fl_RGB_Image*) img->copy(destW, destH);
    stretched->draw(destX, destY);
    delete img;
    delete stretched;
}

void CPngImage::cutTileDraw(
        CPngImage* sourceImage, int srcX, int srcY, int srcW, int srcH, int destX, int destY, int destW, int destH)
{
    if (destW <= 0 || destH <= 0) return;
    Fl_RGB_Image* img = subRGBImage(sourceImage, srcX, srcY, srcW, srcH);
    fl_push_clip(destX, destY, destW, destH);
    for (int y = destY; y < destY + destH; y += srcH)
        for (int x = destX; x < destX + destW; x += srcW)
            img->draw(x, y);
    fl_pop_clip();
    delete img;
}

void
CPngImage::drawResized(int xx, int yy, int ww, int hh, int cornerWidth, CPatternDrawMode drawMode, bool drawBackground)
{
    int xSideSpace = w() - cornerWidth * 2;
    if (xSideSpace < 2)
        xSideSpace = 2;
    int cornerSizeX = (w() - xSideSpace) / 2;

    int ySideSpace = h() - cornerWidth * 2;
    if (ySideSpace < 2)
        ySideSpace = 2;
    int cornerSizeY = (h() - ySideSpace) / 2;

    if (cornerSizeX > ww / 2)
        cornerSizeX = ww / 2;
    if (cornerSizeY > hh / 2)
        cornerSizeY = hh / 2;

    /// Draw corners
    int imageHeight = h(), imageWidth = w();
    for (int destY = yy, srcY = 0; destY < yy + hh; destY += hh - cornerSizeY, srcY += imageHeight - cornerSizeY)
        for (int destX = xx, srcX = 0; destX < xx + ww; destX += ww - cornerSizeX, srcX += imageWidth - cornerSizeX)
            draw(destX, destY, cornerSizeX, cornerSizeY, srcX, srcY);

    resized_draw_function resizedDraw = cutTileDraw;
    if (drawMode == PDM_STRETCH)
        resizedDraw = cutStretchDraw;

    resizedDraw(this, cornerSizeX, 0, w() - cornerSizeX * 2, cornerSizeY, xx + cornerSizeX, yy, ww - cornerSizeX * 2,
                cornerSizeY);
    resizedDraw(this, cornerSizeX, h() - cornerSizeY, w() - cornerSizeX * 2, cornerSizeY, xx + cornerSizeX,
                yy + hh - cornerSizeY, ww - cornerSizeX * 2, cornerSizeY);
    resizedDraw(this, 0, cornerSizeY, cornerSizeX, h() - cornerSizeY * 2, xx, yy + cornerSizeY, cornerSizeX,
                hh - cornerSizeY * 2);
    resizedDraw(this, w() - cornerSizeX, cornerSizeY, cornerSizeX, h() - cornerSizeY * 2, xx + ww - cornerSizeX,
                yy + cornerSizeY, cornerSizeX, hh - cornerSizeY * 2);
    if (drawBackground)
        resizedDraw(this, cornerSizeX, cornerSizeY, w() - cornerSizeX * 2, h() - cornerSizeY * 2, xx + cornerSizeX,
                    yy + cornerSizeY, ww - cornerSizeX * 2, hh - cornerSizeY * 2);
}

void
CPngImage::drawResized(int xx, int yy, int ww, int hh, int border[], CPatternDrawMode drawMode, bool drawBackground)
{
    int xBorderSpace = border[BORDER_LEFT] + border[BORDER_RIGHT];
    int xSideSpace = w() - xBorderSpace;

    int yBorderSpace = border[BORDER_TOP] + border[BORDER_BOTTOM];
    int ySideSpace = h() - yBorderSpace;

    /*
    int cornerSizeX = border[BORDER_LEFT];
    if (cornerSizeX > ww/2)
        cornerSizeX = ww/2;
    int cornerSizeY = border[BORDER_TOP];
    if (cornerSizeY > hh/2)
        cornerSizeY = hh/2;
    */

    /// Draw corners
    int imageHeight = h(), imageWidth = w();
    draw(xx, yy, border[BORDER_LEFT], border[BORDER_TOP], 0, 0);
    draw(xx, yy + hh - border[BORDER_BOTTOM], border[BORDER_LEFT], border[BORDER_BOTTOM], 0,
         imageHeight - border[BORDER_BOTTOM]);
    draw(xx + ww - border[BORDER_RIGHT], yy, border[BORDER_LEFT], border[BORDER_TOP], imageWidth - border[BORDER_RIGHT],
         0);
    draw(xx + ww - border[BORDER_RIGHT], yy + hh - border[BORDER_BOTTOM], border[BORDER_LEFT], border[BORDER_BOTTOM],
         imageWidth - border[BORDER_RIGHT], imageHeight - border[BORDER_BOTTOM]);

    resized_draw_function resizedDraw = cutTileDraw;
    if (drawMode == PDM_STRETCH)
        resizedDraw = cutStretchDraw;

    /// Draw frame parts 
    if (xSideSpace > 0) {
        resizedDraw(this, border[BORDER_LEFT], 0, xSideSpace, border[BORDER_TOP], xx + border[BORDER_LEFT], yy,
                    ww - xBorderSpace, border[BORDER_TOP]);
        resizedDraw(this, border[BORDER_LEFT], h() - border[BORDER_BOTTOM], xSideSpace, border[BORDER_BOTTOM],
                    xx + border[BORDER_LEFT], yy + hh - border[BORDER_BOTTOM], ww - xBorderSpace,
                    border[BORDER_BOTTOM]);
    }
    if (ySideSpace > 0) {
        resizedDraw(this, 0, border[BORDER_TOP], border[BORDER_LEFT], ySideSpace, xx, yy + border[BORDER_TOP],
                    border[BORDER_LEFT], hh - yBorderSpace);
        resizedDraw(this, w() - border[BORDER_RIGHT], border[BORDER_TOP], border[BORDER_RIGHT], ySideSpace,
                    xx + ww - border[BORDER_RIGHT], yy + border[BORDER_TOP], border[BORDER_RIGHT], hh - yBorderSpace);
    }
    if (drawBackground && xSideSpace > 0 && ySideSpace > 0)
        resizedDraw(this, border[BORDER_LEFT], border[BORDER_TOP], xSideSpace, ySideSpace, xx + border[BORDER_LEFT],
                    yy + border[BORDER_TOP], ww - xBorderSpace, hh - yBorderSpace);
}
