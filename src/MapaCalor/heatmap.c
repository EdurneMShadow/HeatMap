/* heatmap - High performance heatmap creation in C.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 Lucas Beyer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
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

#include "heatmap.h"

#include <stdlib.h> /* malloc, calloc, free */
#include <string.h> /* memcpy, memset */
#include <math.h>   /* sqrtf */
#include <assert.h> /* assert, #define NDEBUG to ignore. */

/* Having a default stamp ready makes it easier for simple usage of the library
 * since there is no need to create a new stamp.
 */
static float stamp_default_4_data[] = {
    0.0f      , 0.0f      , 0.1055728f, 0.1753789f, 0.2f, 0.1753789f, 0.1055728f, 0.0f      , 0.0f      ,
    0.0f      , 0.1514719f, 0.2788897f, 0.3675445f, 0.4f, 0.3675445f, 0.2788897f, 0.1514719f, 0.0f      ,
    0.1055728f, 0.2788897f, 0.4343146f, 0.5527864f, 0.6f, 0.5527864f, 0.4343146f, 0.2788897f, 0.1055728f,
    0.1753789f, 0.3675445f, 0.5527864f, 0.7171573f, 0.8f, 0.7171573f, 0.5527864f, 0.3675445f, 0.1753789f,
    0.2f      , 0.4f      , 0.6f      , 0.8f      , 1.0f, 0.8f      , 0.6f      , 0.4f      , 0.2f      ,
    0.1753789f, 0.3675445f, 0.5527864f, 0.7171573f, 0.8f, 0.7171573f, 0.5527864f, 0.3675445f, 0.1753789f,
    0.1055728f, 0.2788897f, 0.4343146f, 0.5527864f, 0.6f, 0.5527864f, 0.4343146f, 0.2788897f, 0.1055728f,
    0.0f      , 0.1514719f, 0.2788897f, 0.3675445f, 0.4f, 0.3675445f, 0.2788897f, 0.1514719f, 0.0f      ,
    0.0f      , 0.0f      , 0.1055728f, 0.1753789f, 0.2f, 0.1753789f, 0.1055728f, 0.0f      , 0.0f      ,
};

static heatmap_stamp_t stamp_default_4 = {
    stamp_default_4_data, 9, 9
};

void heatmap_init(heatmap_t* hm, unsigned w, unsigned h){
    memset(hm, 0, sizeof(heatmap_t));
    hm->buf = (float*)calloc(w*h, sizeof(float));
    hm->w = w;
    hm->h = h;
}

heatmap_t* heatmap_new(unsigned w, unsigned h)
{
    heatmap_t* hm = (heatmap_t*)malloc(sizeof(heatmap_t));
    heatmap_init(hm, w, h);
    return hm;
}

void heatmap_free(heatmap_t* h)
{
    free(h->buf);
    free(h);
}

void heatmap_add_point(heatmap_t* h, unsigned x, unsigned y)
{
    heatmap_add_point_with_stamp(h, x, y, &stamp_default_4);
}

void heatmap_add_point_with_stamp(heatmap_t* h, unsigned x, unsigned y, const heatmap_stamp_t* stamp)
{
    /* I'm still unsure whether we want this to be an assert or not... */
    if(x >= h->w || y >= h->h)
        return;

    /* I hate you, C */
    {
        /* Note: the order of operations is important, since we're computing with unsigned! */

        /* These are [first, last) pairs in the STAMP's pixels. */
        const unsigned x0 = x < stamp->w/2 ? (stamp->w/2 - x) : 0;
        const unsigned y0 = y < stamp->h/2 ? (stamp->h/2 - y) : 0;
        const unsigned x1 = (x + stamp->w/2) < h->w ? stamp->w : stamp->w/2 + (h->w - x);
        const unsigned y1 = (y + stamp->h/2) < h->h ? stamp->h : stamp->h/2 + (h->h - y);

        unsigned iy;

        for(iy = y0 ; iy < y1 ; ++iy) {
            /* TODO: could it be clearer by using separate vars and computing a ystep? */
            float* line = h->buf + ((y + iy) - stamp->h/2)*h->w + (x + x0) - stamp->w/2;
            const float* stampline = stamp->buf + iy*stamp->w + x0;

            unsigned ix;
            for(ix = x0 ; ix < x1 ; ++ix, ++line, ++stampline) {
                /* TODO: Let's actually accept negatives and try out funky stamps. */
                /* Note that that might mess with the max though. */
                /* And that we'll have to clamp the bottom to 0 when rendering. */
                assert(*stampline >= 0.0f);

                *line += *stampline;
                if(*line > h->max) {h->max = *line;}

                assert(*line >= 0.0f);
            }
        }
    } /* I hate you very much! */
}

void heatmap_add_weighted_point(heatmap_t* h, unsigned x, unsigned y, float w)
{
    heatmap_add_weighted_point_with_stamp(h, x, y, w, &stamp_default_4);
}

/* Initial timings do show a difference large enough (~10% slower without FMA)
 * that we do care about splitting the implementation,
 * even though JUST A SINGLE LINE OF CODE has changed!
 * And I don't want to spoil the readability by using macro-trickery to avoid duplication.
 * sad :-(
 */
void heatmap_add_weighted_point_with_stamp(heatmap_t* h, unsigned x, unsigned y, float w, const heatmap_stamp_t* stamp)
{
    /* I'm still unsure whether we want this to be an assert or not... */
    if(x >= h->w || y >= h->h)
        return;

    /* Currently, negative weights are not supported as they mess with the max. */
    assert(w >= 0.0f);

    /* I hate you, C */
    {
        /* Note: the order of operations is important, since we're computing with unsigned! */

        /* These are [first, last) pairs in the STAMP's pixels. */
        const unsigned x0 = x < stamp->w/2 ? (stamp->w/2 - x) : 0;
        const unsigned y0 = y < stamp->h/2 ? (stamp->h/2 - y) : 0;
        const unsigned x1 = (x + stamp->w/2) < h->w ? stamp->w : stamp->w/2 + (h->w - x);
        const unsigned y1 = (y + stamp->h/2) < h->h ? stamp->h : stamp->h/2 + (h->h - y);

        unsigned iy;

        for(iy = y0 ; iy < y1 ; ++iy) {
            /* TODO: could it be clearer by using separate vars and computing a ystep? */
            float* line = h->buf + ((y + iy) - stamp->h/2)*h->w + (x + x0) - stamp->w/2;
            const float* stampline = stamp->buf + iy*stamp->w + x0;

            unsigned ix;
            for(ix = x0 ; ix < x1 ; ++ix, ++line, ++stampline) {
                /* TODO: see unweighted function */
                assert(*stampline >= 0.0f);

                *line += *stampline * w;
                if(*line > h->max) {h->max = *line;}

                assert(*line >= 0.0f);
            }
        }
    } /* I hate you very much! */
}

unsigned char* heatmap_render_default_to(const heatmap_t* h, unsigned char* colorbuf)
{
    return heatmap_render_to(h, heatmap_cs_default, colorbuf);
}

unsigned char* heatmap_render_to(const heatmap_t* h, const heatmap_colorscheme_t* colorscheme, unsigned char* colorbuf)
{
    /* TODO: Time whether it makes a noticeable difference to inline that code
     * here and drop the saturation step.
     */
    /* If the heatmap is empty, h->max (and thus the saturation value) is 0.0, resulting in a 0-by-0 division.
     * In that case, we should set the saturation to anything but 0, since we want the result of the division to be 0.
     * Also, a comparison to exact 0.0f (as opposed to 1e-14) is OK, since we only do division.
     */
    return heatmap_render_saturated_to(h, colorscheme, h->max > 0.0f ? h->max : 1.0f, colorbuf);
}

unsigned char* heatmap_render_saturated_to(const heatmap_t* h, const heatmap_colorscheme_t* colorscheme, float saturation, unsigned char* colorbuf)
{
    unsigned y;
    assert(saturation > 0.0f);

    /* For convenience, if no buffer is given, malloc a new one. */
    if(!colorbuf) {
        colorbuf = (unsigned char*)malloc(h->w*h->h*4);
        if(!colorbuf) {
            return 0;
        }
    }

    /* TODO: could actually even flatten this loop before parallelizing it. */
    /* I.e., to go i = 0 ; i < h*w since I don't have any padding! (yet?) */
    for(y = 0 ; y < h->h ; ++y) {
        float* bufline = h->buf + y*h->w;
        unsigned char* colorline = colorbuf + 4*y*h->w;

        unsigned x;
        for(x = 0 ; x < h->w ; ++x, ++bufline) {
            /* Saturate the heat value to the given saturation, and then
             * normalize by that.
             */
            const float val = (*bufline > saturation ? saturation : *bufline)/saturation;

            /* We add 0.5 in order to do real rounding, not just dropping the
             * decimal part. That way we are certain the highest value in the
             * colorscheme is actually used.
             */
            const size_t idx = (size_t)((float)(colorscheme->ncolors-1)*val + 0.5f);

            /* This is probably caused by a negative entry in the stamp! */
            assert(val >= 0.0f);

            /* This should never happen. It is likely a bug in this library. */
            assert(idx < colorscheme->ncolors);

            /* Just copy over the color from the colorscheme. */
            memcpy(colorline, colorscheme->colors + idx*4, 4);
            colorline += 4;
        }
    }

    return colorbuf;
}

void heatmap_stamp_init(heatmap_stamp_t* stamp, unsigned w, unsigned h, float* data)
{
    if(stamp) {
        memset(stamp, 0, sizeof(heatmap_stamp_t));
        stamp->w = w;
        stamp->h = h;
        stamp->buf = data;
    }
}

heatmap_stamp_t* heatmap_stamp_new_with(unsigned w, unsigned h, float* data)
{
    heatmap_stamp_t* stamp = (heatmap_stamp_t*)malloc(sizeof(heatmap_stamp_t));
    heatmap_stamp_init(stamp, w, h, data);
    return stamp;
}

heatmap_stamp_t* heatmap_stamp_load(unsigned w, unsigned h, float* data)
{
    float* copy = (float*)malloc(sizeof(float)*w*h);
    memcpy(copy, data, sizeof(float)*w*h);
    return heatmap_stamp_new_with(w, h, copy);
}

static float linear_dist(float dist)
{
    return dist;
}

heatmap_stamp_t* heatmap_stamp_gen(unsigned r)
{
    return heatmap_stamp_gen_nonlinear(r, linear_dist);
}

heatmap_stamp_t* heatmap_stamp_gen_nonlinear(unsigned r, float (*distshape)(float))
{
    unsigned y;
    unsigned d = 2*r+1;

    float* stamp = (float*)calloc(d*d, sizeof(float));
    if(!stamp)
        return 0;

    for(y = 0 ; y < d ; ++y) {
        float* line = stamp + y*d;
        unsigned x;
        for(x = 0 ; x < d ; ++x, ++line) {
            const float dist = sqrtf((float)((x-r)*(x-r) + (y-r)*(y-r)))/(float)(r+1);
            const float ds = (*distshape)(dist);
            /* This doesn't generate optimal assembly, but meh, it's readable. */
            const float clamped_ds = ds > 1.0f ? 1.0f
                                   : ds < 0.0f ? 0.0f
                                   :             ds;
            *line = 1.0f - clamped_ds;
        }
    }

    return heatmap_stamp_new_with(d, d, stamp);
}

void heatmap_stamp_free(heatmap_stamp_t* s)
{
    free(s->buf);
    free(s);
}

heatmap_colorscheme_t* heatmap_colorscheme_load(const unsigned char* in_colors, size_t ncolors)
{
    heatmap_colorscheme_t* cs = (heatmap_colorscheme_t*)calloc(1, sizeof(heatmap_colorscheme_t));
    unsigned char* colors = (unsigned char*)malloc(4*ncolors);

    if(!cs || !colors) {
        free(cs);
        free(colors);
        return 0;
    }

    memcpy(colors, in_colors, 4*ncolors);

    cs->colors = colors;
    cs->ncolors = ncolors;
    return cs;
}

void heatmap_colorscheme_free(heatmap_colorscheme_t* cs)
{
    /* ehhh, const_cast<>! */
    free((void*)cs->colors);
    free(cs);
}

/* Sorry dynamic wordwarp editor users! But you deserve no better anyways... */
static const unsigned char mixed_data[] = {0, 0, 0, 0, 94, 79, 162, 0, 93, 79, 162, 7, 93, 80, 162, 14, 92, 80, 163, 22, 92, 81, 163, 29, 91, 81, 164, 37, 91, 82, 164, 44, 90, 82, 164, 52, 90, 83, 165, 59, 89, 83, 165, 67, 89, 84, 166, 74, 88, 84, 166, 82, 88, 85, 166, 89, 87, 85, 167, 97, 87, 86, 167, 104, 86, 86, 167, 112, 86, 87, 168, 119, 85, 87, 168, 127, 85, 88, 168, 134, 84, 88, 169, 141, 84, 89, 169, 149, 83, 89, 169, 156, 83, 90, 170, 164, 83, 90, 170, 171, 82, 91, 170, 179, 82, 91, 171, 186, 81, 92, 171, 194, 81, 92, 171, 201, 80, 93, 172, 209, 80, 93, 172, 216, 79, 94, 172, 224, 79, 94, 172, 231, 78, 95, 173, 239, 78, 95, 173, 246, 77, 95, 173, 254, 77, 96, 174, 255, 76, 96, 174, 255, 76, 97, 174, 255, 75, 97, 174, 255, 75, 98, 175, 255, 74, 98, 175, 255, 74, 99, 175, 255, 73, 99, 175, 255, 73, 100, 176, 255, 72, 100, 176, 255, 72, 101, 176, 255, 72, 101, 176, 255, 71, 101, 176, 255, 71, 102, 177, 255, 70, 102, 177, 255, 70, 103, 177, 255, 69, 103, 177, 255, 60, 115, 183, 255, 60, 115, 183, 255, 59, 116, 183, 255, 59, 116, 183, 255, 58, 117, 184, 255, 58, 117, 184, 255, 58, 118, 184, 255, 57, 118, 184, 255, 57, 118, 184, 255, 56, 119, 184, 255, 56, 119, 185, 255, 56, 120, 185, 255, 55, 120, 185, 255, 55, 121, 185, 255, 55, 121, 185, 255, 54, 121, 185, 255, 54, 122, 186, 255, 54, 122, 186, 255, 53, 123, 186, 255, 53, 123, 186, 255, 53, 124, 186, 255, 52, 124, 186, 255, 52, 124, 186, 255, 52, 125, 186, 255, 52, 125, 187, 255, 51, 126, 187, 255, 51, 126, 187, 255, 51, 126, 187, 255, 51, 127, 187, 255, 50, 127, 187, 255, 50, 128, 187, 255, 50, 128, 187, 255, 50, 128, 187, 255, 50, 129, 187, 255, 50, 129, 188, 255, 50, 130, 188, 255, 49, 130, 188, 255, 49, 130, 188, 255, 49, 131, 188, 255, 49, 131, 188, 255, 49, 132, 188, 255, 49, 132, 188, 255, 49, 132, 188, 255, 49, 133, 188, 255, 49, 133, 188, 255, 49, 133, 188, 255, 49, 134, 188, 255, 49, 134, 188, 255, 49, 135, 188, 255, 49, 135, 188, 255, 49, 135, 188, 255, 49, 136, 189, 255, 47, 136, 189, 255, 46, 137, 189, 255, 45, 138, 189, 255, 43, 138, 189, 255, 42, 139, 190, 255, 41, 139, 190, 255, 39, 140, 190, 255, 38, 140, 190, 255, 36, 141, 190, 255, 35, 141, 190, 255, 33, 142, 190, 255, 31, 142, 190, 255, 30, 143, 190, 255, 28, 143, 190, 255, 26, 144, 191, 255, 24, 145, 191, 255, 22, 145, 191, 255, 20, 146, 191, 255, 17, 146, 191, 255, 15, 147, 191, 255, 12, 147, 191, 255, 10, 148, 191, 255, 10, 148, 191, 255, 10, 149, 191, 255, 10, 149, 191, 255, 10, 150, 191, 255, 10, 150, 190, 255, 10, 151, 190, 255, 10, 151, 190, 255, 10, 152, 190, 255, 10, 152, 190, 255, 10, 153, 190, 255, 10, 153, 190, 255, 10, 154, 190, 255, 10, 154, 190, 255, 10, 155, 190, 255, 10, 155, 189, 255, 10, 156, 189, 255, 10, 156, 189, 255, 10, 157, 189, 255, 10, 157, 189, 255, 10, 158, 189, 255, 10, 158, 188, 255, 10, 158, 188, 255, 10, 159, 188, 255, 10, 159, 188, 255, 10, 160, 188, 255, 10, 160, 187, 255, 10, 161, 187, 255, 10, 161, 187, 255, 20, 173, 182, 255, 22, 174, 182, 255, 25, 174, 181, 255, 28, 175, 181, 255, 30, 175, 181, 255, 33, 176, 180, 255, 35, 176, 180, 255, 37, 176, 180, 255, 39, 177, 180, 255, 41, 177, 179, 255, 43, 178, 179, 255, 45, 178, 179, 255, 46, 179, 178, 255, 48, 179, 178, 255, 50, 179, 178, 255, 51, 180, 177, 255, 53, 180, 177, 255, 54, 181, 177, 255, 56, 181, 176, 255, 58, 182, 176, 255, 59, 182, 176, 255, 61, 182, 175, 255, 62, 183, 175, 255, 64, 183, 175, 255, 65, 184, 174, 255, 66, 184, 174, 255, 68, 184, 174, 255, 69, 185, 173, 255, 71, 185, 173, 255, 72, 186, 173, 255, 74, 186, 172, 255, 75, 186, 172, 255, 76, 187, 171, 255, 78, 187, 171, 255, 79, 187, 171, 255, 80, 188, 170, 255, 82, 188, 170, 255, 83, 189, 170, 255, 85, 189, 169, 255, 86, 189, 169, 255, 87, 190, 169, 255, 89, 190, 168, 255, 90, 190, 168, 255, 91, 191, 167, 255, 93, 191, 167, 255, 94, 191, 167, 255, 95, 192, 166, 255, 97, 192, 166, 255, 98, 193, 166, 255, 99, 193, 165, 255, 100, 193, 165, 255, 102, 194, 164, 255, 102, 194, 164, 255, 103, 194, 164, 255, 103, 194, 164, 255, 104, 194, 164, 255, 104, 195, 164, 255, 105, 195, 164, 255, 105, 195, 164, 255, 106, 195, 164, 255, 106, 196, 164, 255, 107, 196, 164, 255, 108, 196, 164, 255, 108, 196, 164, 255, 109, 196, 164, 255, 109, 197, 164, 255, 110, 197, 164, 255, 110, 197, 164, 255, 111, 197, 164, 255, 111, 198, 164, 255, 112, 198, 164, 255, 112, 198, 164, 255, 113, 198, 164, 255, 113, 198, 164, 255, 114, 199, 164, 255, 115, 199, 164, 255, 115, 199, 164, 255, 116, 199, 164, 255, 116, 200, 164, 255, 117, 200, 164, 255, 117, 200, 164, 255, 118, 200, 164, 255, 118, 200, 164, 255, 119, 201, 164, 255, 119, 201, 164, 255, 120, 201, 164, 255, 120, 201, 164, 255, 121, 201, 164, 255, 122, 202, 164, 255, 122, 202, 164, 255, 123, 202, 164, 255, 123, 202, 164, 255, 124, 203, 164, 255, 124, 203, 164, 255, 125, 203, 164, 255, 125, 203, 164, 255, 126, 203, 164, 255, 126, 204, 164, 255, 127, 204, 164, 255, 127, 204, 163, 255, 128, 204, 163, 255, 129, 204, 163, 255, 143, 210, 163, 255, 143, 210, 163, 255, 144, 210, 163, 255, 144, 211, 163, 255, 145, 211, 163, 255, 146, 211, 163, 255, 146, 211, 163, 255, 147, 212, 163, 255, 147, 212, 163, 255, 148, 212, 163, 255, 148, 212, 163, 255, 149, 212, 163, 255, 149, 213, 163, 255, 150, 213, 163, 255, 150, 213, 163, 255, 151, 213, 163, 255, 151, 213, 163, 255, 152, 214, 163, 255, 153, 214, 163, 255, 153, 214, 163, 255, 154, 214, 163, 255, 154, 214, 163, 255, 155, 215, 163, 255, 155, 215, 163, 255, 156, 215, 163, 255, 156, 215, 163, 255, 157, 215, 163, 255, 157, 216, 163, 255, 158, 216, 163, 255, 158, 216, 163, 255, 159, 216, 163, 255, 160, 216, 163, 255, 160, 217, 163, 255, 161, 217, 163, 255, 161, 217, 163, 255, 162, 217, 163, 255, 162, 217, 163, 255, 163, 218, 163, 255, 163, 218, 163, 255, 164, 218, 163, 255, 164, 218, 163, 255, 165, 218, 163, 255, 166, 219, 163, 255, 166, 219, 163, 255, 167, 219, 163, 255, 167, 219, 163, 255, 168, 219, 163, 255, 168, 220, 163, 255, 169, 220, 163, 255, 169, 220, 163, 255, 170, 220, 163, 255, 170, 220, 163, 255, 171, 221, 163, 255, 171, 221, 163, 255, 172, 221, 163, 255, 172, 221, 163, 255, 172, 222, 163, 255, 173, 222, 163, 255, 173, 222, 163, 255, 173, 222, 163, 255, 174, 222, 163, 255, 174, 223, 163, 255, 175, 223, 163, 255, 175, 223, 163, 255, 175, 223, 162, 255, 176, 223, 162, 255, 176, 224, 162, 255, 177, 224, 162, 255, 177, 224, 162, 255, 177, 224, 162, 255, 178, 224, 162, 255, 178, 225, 162, 255, 179, 225, 162, 255, 179, 225, 162, 255, 179, 225, 162, 255, 180, 225, 161, 255, 180, 226, 161, 255, 181, 226, 161, 255, 181, 226, 161, 255, 182, 226, 161, 255, 182, 226, 161, 255, 182, 227, 161, 255, 183, 227, 161, 255, 183, 227, 161, 255, 184, 227, 161, 255, 184, 227, 160, 255, 185, 228, 160, 255, 185, 228, 160, 255, 185, 228, 160, 255, 186, 228, 160, 255, 186, 228, 160, 255, 187, 229, 160, 255, 187, 229, 160, 255, 188, 229, 160, 255, 188, 229, 160, 255, 189, 229, 159, 255, 189, 230, 159, 255, 189, 230, 159, 255, 190, 230, 159, 255, 190, 230, 159, 255, 191, 230, 159, 255, 191, 231, 159, 255, 192, 231, 159, 255, 204, 236, 156, 255, 205, 236, 156, 255, 205, 236, 156, 255, 205, 236, 156, 255, 206, 236, 156, 255, 206, 237, 156, 255, 207, 237, 156, 255, 207, 237, 156, 255, 208, 237, 156, 255, 208, 237, 155, 255, 209, 238, 155, 255, 209, 238, 155, 255, 210, 238, 155, 255, 210, 238, 155, 255, 211, 238, 155, 255, 211, 238, 155, 255, 212, 239, 155, 255, 212, 239, 155, 255, 213, 239, 155, 255, 213, 239, 154, 255, 214, 239, 154, 255, 214, 240, 154, 255, 215, 240, 154, 255, 215, 240, 154, 255, 216, 240, 154, 255, 216, 240, 154, 255, 217, 240, 154, 255, 217, 241, 154, 255, 218, 241, 154, 255, 218, 241, 153, 255, 219, 241, 153, 255, 219, 241, 153, 255, 220, 241, 153, 255, 220, 242, 153, 255, 221, 242, 153, 255, 221, 242, 153, 255, 222, 242, 153, 255, 222, 242, 153, 255, 223, 242, 153, 255, 223, 243, 153, 255, 224, 243, 152, 255, 224, 243, 152, 255, 225, 243, 152, 255, 225, 243, 152, 255, 226, 243, 152, 255, 227, 244, 152, 255, 227, 244, 152, 255, 228, 244, 152, 255, 228, 244, 152, 255, 229, 244, 152, 255, 229, 244, 152, 255, 230, 244, 151, 255, 230, 244, 151, 255, 230, 244, 151, 255, 230, 244, 151, 255, 230, 244, 151, 255, 230, 244, 151, 255, 230, 244, 151, 255, 230, 244, 151, 255, 230, 244, 151, 255, 231, 244, 151, 255, 231, 244, 151, 255, 231, 244, 151, 255, 231, 243, 151, 255, 231, 243, 151, 255, 231, 243, 151, 255, 231, 243, 151, 255, 231, 243, 150, 255, 231, 243, 150, 255, 232, 243, 150, 255, 232, 243, 150, 255, 232, 243, 150, 255, 232, 243, 150, 255, 232, 243, 150, 255, 232, 243, 150, 255, 232, 243, 150, 255, 232, 242, 150, 255, 232, 242, 150, 255, 233, 242, 150, 255, 233, 242, 150, 255, 233, 242, 150, 255, 233, 242, 150, 255, 233, 242, 150, 255, 233, 242, 150, 255, 233, 242, 150, 255, 233, 242, 149, 255, 233, 242, 149, 255, 234, 242, 149, 255, 234, 241, 149, 255, 234, 241, 149, 255, 234, 241, 149, 255, 234, 241, 149, 255, 234, 241, 149, 255, 234, 241, 149, 255, 234, 241, 149, 255, 234, 241, 149, 255, 235, 241, 149, 255, 235, 241, 149, 255, 235, 241, 149, 255, 235, 241, 149, 255, 235, 240, 149, 255, 235, 240, 149, 255, 235, 240, 149, 255, 235, 240, 149, 255, 235, 240, 149, 255, 236, 240, 148, 255, 236, 240, 148, 255, 236, 240, 148, 255, 236, 240, 148, 255, 236, 240, 148, 255, 236, 240, 148, 255, 236, 240, 148, 255, 236, 239, 148, 255, 236, 239, 148, 255, 236, 239, 148, 255, 237, 239, 148, 255, 237, 239, 148, 255, 237, 239, 148, 255, 237, 239, 148, 255, 237, 239, 148, 255, 237, 239, 148, 255, 237, 239, 148, 255, 237, 239, 148, 255, 237, 239, 148, 255, 237, 238, 148, 255, 238, 238, 148, 255, 238, 238, 148, 255, 238, 238, 148, 255, 238, 238, 148, 255, 238, 238, 147, 255, 238, 238, 147, 255, 238, 238, 147, 255, 238, 238, 147, 255, 238, 238, 147, 255, 238, 238, 147, 255, 239, 238, 147, 255, 239, 237, 147, 255, 239, 237, 147, 255, 239, 237, 147, 255, 239, 237, 147, 255, 239, 237, 147, 255, 239, 237, 147, 255, 239, 237, 147, 255, 239, 237, 147, 255, 239, 237, 147, 255, 240, 237, 147, 255, 240, 237, 147, 255, 240, 237, 147, 255, 240, 236, 147, 255, 240, 236, 147, 255, 240, 236, 147, 255, 240, 236, 147, 255, 240, 236, 147, 255, 245, 232, 145, 255, 245, 232, 145, 255, 245, 232, 145, 255, 245, 232, 145, 255, 245, 232, 145, 255, 246, 231, 145, 255, 246, 231, 145, 255, 246, 231, 145, 255, 246, 231, 145, 255, 246, 231, 145, 255, 246, 231, 145, 255, 246, 231, 145, 255, 246, 231, 145, 255, 246, 231, 145, 255, 246, 231, 145, 255, 246, 231, 145, 255, 247, 231, 145, 255, 247, 230, 145, 255, 247, 230, 145, 255, 247, 230, 145, 255, 247, 230, 145, 255, 247, 230, 144, 255, 247, 230, 144, 255, 247, 230, 144, 255, 247, 230, 144, 255, 247, 230, 144, 255, 247, 230, 144, 255, 248, 230, 144, 255, 248, 230, 144, 255, 248, 229, 144, 255, 248, 229, 144, 255, 248, 229, 144, 255, 248, 229, 144, 255, 248, 229, 144, 255, 248, 229, 144, 255, 248, 229, 144, 255, 248, 229, 144, 255, 248, 229, 144, 255, 248, 229, 144, 255, 249, 229, 144, 255, 249, 229, 144, 255, 249, 229, 144, 255, 249, 228, 144, 255, 249, 228, 144, 255, 249, 228, 144, 255, 249, 228, 144, 255, 249, 228, 144, 255, 249, 228, 144, 255, 249, 228, 144, 255, 249, 228, 144, 255, 249, 228, 144, 255, 250, 228, 144, 255, 250, 228, 144, 255, 250, 228, 144, 255, 250, 227, 144, 255, 250, 227, 144, 255, 250, 227, 144, 255, 250, 227, 144, 255, 250, 227, 144, 255, 250, 227, 144, 255, 250, 227, 144, 255, 250, 227, 144, 255, 250, 227, 144, 255, 251, 227, 144, 255, 251, 227, 144, 255, 251, 227, 144, 255, 251, 226, 144, 255, 251, 226, 144, 255, 251, 226, 144, 255, 251, 226, 144, 255, 251, 226, 144, 255, 251, 226, 144, 255, 251, 226, 144, 255, 251, 226, 144, 255, 251, 226, 144, 255, 251, 226, 144, 255, 252, 226, 144, 255, 252, 226, 144, 255, 252, 225, 144, 255, 252, 225, 144, 255, 252, 225, 144, 255, 252, 225, 144, 255, 252, 225, 144, 255, 252, 225, 144, 255, 252, 225, 144, 255, 252, 225, 144, 255, 252, 225, 144, 255, 252, 225, 144, 255, 252, 225, 144, 255, 253, 225, 144, 255, 253, 225, 144, 255, 253, 224, 144, 255, 253, 224, 144, 255, 253, 224, 144, 255, 253, 224, 144, 255, 253, 224, 144, 255, 253, 224, 144, 255, 253, 224, 144, 255, 253, 224, 144, 255, 253, 224, 144, 255, 253, 224, 144, 255, 253, 224, 144, 255, 253, 224, 144, 255, 253, 223, 143, 255, 253, 223, 143, 255, 253, 223, 142, 255, 253, 222, 142, 255, 253, 222, 141, 255, 253, 221, 141, 255, 253, 221, 141, 255, 253, 221, 140, 255, 253, 220, 140, 255, 253, 220, 139, 255, 253, 220, 139, 255, 253, 219, 138, 255, 253, 219, 138, 255, 253, 218, 138, 255, 253, 218, 137, 255, 253, 218, 137, 255, 253, 217, 136, 255, 253, 217, 136, 255, 253, 217, 135, 255, 253, 216, 135, 255, 253, 216, 135, 255, 253, 215, 134, 255, 253, 215, 134, 255, 253, 215, 133, 255, 253, 214, 133, 255, 253, 214, 133, 255, 253, 214, 132, 255, 253, 213, 132, 255, 253, 213, 131, 255, 253, 212, 131, 255, 253, 212, 131, 255, 253, 212, 130, 255, 253, 211, 130, 255, 253, 211, 129, 255, 253, 211, 129, 255, 253, 210, 129, 255, 253, 210, 128, 255, 253, 209, 128, 255, 253, 209, 127, 255, 253, 209, 127, 255, 253, 208, 127, 255, 253, 208, 126, 255, 253, 207, 126, 255, 253, 207, 125, 255, 253, 207, 125, 255, 253, 206, 125, 255, 253, 206, 124, 255, 253, 205, 124, 255, 253, 205, 123, 255, 253, 205, 123, 255, 253, 204, 123, 255, 253, 194, 113, 255, 253, 194, 113, 255, 253, 193, 112, 255, 253, 193, 112, 255, 253, 192, 112, 255, 253, 192, 111, 255, 253, 192, 111, 255, 253, 191, 110, 255, 253, 191, 110, 255, 253, 190, 110, 255, 253, 190, 109, 255, 253, 190, 109, 255, 253, 189, 109, 255, 253, 189, 108, 255, 253, 188, 108, 255, 253, 188, 108, 255, 253, 188, 107, 255, 253, 187, 107, 255, 253, 187, 107, 255, 253, 186, 106, 255, 253, 186, 106, 255, 253, 186, 106, 255, 253, 185, 105, 255, 253, 185, 105, 255, 253, 184, 105, 255, 253, 184, 104, 255, 253, 184, 104, 255, 253, 183, 104, 255, 253, 183, 103, 255, 253, 182, 103, 255, 253, 182, 103, 255, 253, 182, 102, 255, 253, 181, 102, 255, 253, 181, 102, 255, 253, 180, 101, 255, 253, 180, 101, 255, 253, 180, 101, 255, 253, 179, 100, 255, 253, 179, 100, 255, 253, 178, 100, 255, 253, 178, 100, 255, 253, 178, 99, 255, 253, 177, 99, 255, 253, 177, 99, 255, 253, 176, 98, 255, 253, 176, 98, 255, 253, 175, 98, 255, 253, 175, 98, 255, 253, 175, 97, 255, 253, 174, 97, 255, 253, 174, 97, 255, 252, 173, 96, 255, 252, 173, 96, 255, 252, 172, 96, 255, 252, 172, 95, 255, 252, 172, 95, 255, 252, 171, 95, 255, 252, 171, 94, 255, 252, 170, 94, 255, 252, 170, 94, 255, 252, 169, 93, 255, 252, 169, 93, 255, 252, 168, 93, 255, 252, 168, 92, 255, 252, 167, 92, 255, 252, 167, 92, 255, 252, 166, 91, 255, 252, 166, 91, 255, 252, 165, 91, 255, 251, 165, 90, 255, 251, 164, 90, 255, 251, 164, 90, 255, 251, 163, 89, 255, 251, 163, 89, 255, 251, 162, 89, 255, 251, 162, 89, 255, 251, 162, 88, 255, 251, 161, 88, 255, 251, 161, 88, 255, 251, 160, 87, 255, 251, 160, 87, 255, 251, 159, 87, 255, 251, 159, 87, 255, 251, 158, 86, 255, 251, 158, 86, 255, 251, 157, 86, 255, 250, 157, 85, 255, 250, 156, 85, 255, 250, 156, 85, 255, 250, 155, 85, 255, 250, 155, 84, 255, 250, 154, 84, 255, 250, 154, 84, 255, 250, 153, 84, 255, 250, 153, 83, 255, 250, 152, 83, 255, 250, 152, 83, 255, 250, 151, 83, 255, 250, 151, 82, 255, 250, 150, 82, 255, 250, 150, 82, 255, 249, 149, 82, 255, 248, 136, 75, 255, 248, 135, 75, 255, 247, 135, 75, 255, 247, 134, 75, 255, 247, 134, 74, 255, 247, 133, 74, 255, 247, 133, 74, 255, 247, 132, 74, 255, 247, 132, 74, 255, 247, 131, 73, 255, 247, 131, 73, 255, 247, 130, 73, 255, 247, 130, 73, 255, 247, 129, 72, 255, 247, 129, 72, 255, 247, 128, 72, 255, 246, 128, 72, 255, 246, 127, 72, 255, 246, 126, 71, 255, 246, 126, 71, 255, 246, 125, 71, 255, 246, 125, 71, 255, 246, 124, 71, 255, 246, 124, 71, 255, 246, 123, 70, 255, 246, 123, 70, 255, 246, 122, 70, 255, 246, 122, 70, 255, 246, 121, 70, 255, 245, 121, 70, 255, 245, 120, 69, 255, 245, 120, 69, 255, 245, 119, 69, 255, 245, 119, 69, 255, 245, 118, 69, 255, 245, 117, 69, 255, 245, 117, 68, 255, 245, 116, 68, 255, 245, 116, 68, 255, 245, 115, 68, 255, 245, 115, 68, 255, 244, 114, 68, 255, 244, 114, 68, 255, 244, 113, 67, 255, 244, 113, 67, 255, 244, 112, 67, 255, 244, 111, 67, 255, 244, 111, 67, 255, 244, 110, 67, 255, 244, 110, 67, 255, 244, 109, 67, 255, 244, 109, 67, 255, 243, 108, 67, 255, 243, 108, 67, 255, 243, 107, 67, 255, 243, 107, 67, 255, 243, 107, 67, 255, 242, 106, 67, 255, 242, 106, 67, 255, 242, 106, 67, 255, 242, 105, 67, 255, 242, 105, 67, 255, 241, 104, 68, 255, 241, 104, 68, 255, 241, 104, 68, 255, 241, 103, 68, 255, 241, 103, 68, 255, 240, 102, 68, 255, 240, 102, 68, 255, 240, 102, 68, 255, 240, 101, 68, 255, 240, 101, 68, 255, 239, 101, 69, 255, 239, 100, 69, 255, 239, 100, 69, 255, 239, 99, 69, 255, 238, 99, 69, 255, 238, 99, 69, 255, 238, 98, 69, 255, 238, 98, 69, 255, 238, 98, 69, 255, 237, 97, 69, 255, 237, 97, 70, 255, 237, 96, 70, 255, 237, 96, 70, 255, 236, 96, 70, 255, 236, 95, 70, 255, 236, 95, 70, 255, 236, 95, 70, 255, 236, 94, 70, 255, 235, 94, 70, 255, 235, 94, 70, 255, 235, 93, 71, 255, 235, 93, 71, 255, 234, 92, 71, 255, 234, 92, 71, 255, 234, 92, 71, 255, 234, 91, 71, 255, 233, 91, 71, 255, 233, 91, 71, 255, 233, 90, 71, 255, 233, 90, 71, 255, 233, 89, 72, 255, 226, 80, 74, 255, 226, 79, 74, 255, 226, 79, 74, 255, 225, 79, 74, 255, 225, 78, 74, 255, 225, 78, 74, 255, 225, 77, 75, 255, 224, 77, 75, 255, 224, 77, 75, 255, 224, 76, 75, 255, 224, 76, 75, 255, 223, 76, 75, 255, 223, 75, 75, 255, 223, 75, 75, 255, 223, 75, 75, 255, 222, 74, 75, 255, 222, 74, 75, 255, 222, 73, 76, 255, 222, 73, 76, 255, 221, 73, 76, 255, 221, 72, 76, 255, 221, 72, 76, 255, 220, 72, 76, 255, 220, 71, 76, 255, 220, 71, 76, 255, 220, 71, 76, 255, 219, 70, 76, 255, 219, 70, 76, 255, 219, 70, 77, 255, 219, 69, 77, 255, 218, 69, 77, 255, 218, 68, 77, 255, 218, 68, 77, 255, 217, 68, 77, 255, 217, 67, 77, 255, 217, 67, 77, 255, 217, 67, 77, 255, 216, 66, 77, 255, 216, 66, 77, 255, 216, 66, 78, 255, 216, 65, 78, 255, 215, 65, 78, 255, 215, 65, 78, 255, 215, 64, 78, 255, 214, 64, 78, 255, 214, 63, 78, 255, 214, 63, 78, 255, 214, 63, 78, 255, 213, 62, 78, 255, 213, 62, 78, 255, 213, 62, 78, 255, 212, 61, 78, 255, 212, 61, 78, 255, 211, 61, 78, 255, 211, 60, 78, 255, 211, 60, 78, 255, 210, 59, 78, 255, 210, 59, 78, 255, 209, 59, 78, 255, 209, 58, 78, 255, 209, 58, 78, 255, 208, 57, 78, 255, 208, 57, 77, 255, 207, 57, 77, 255, 207, 56, 77, 255, 206, 56, 77, 255, 206, 56, 77, 255, 206, 55, 77, 255, 205, 55, 77, 255, 205, 54, 77, 255, 204, 54, 77, 255, 204, 54, 77, 255, 203, 53, 77, 255, 203, 53, 76, 255, 203, 52, 76, 255, 202, 52, 76, 255, 202, 52, 76, 255, 201, 51, 76, 255, 201, 51, 76, 255, 200, 50, 76, 255, 200, 50, 76, 255, 200, 50, 76, 255, 199, 49, 76, 255, 199, 49, 76, 255, 198, 48, 75, 255, 198, 48, 75, 255, 198, 48, 75, 255, 197, 47, 75, 255, 197, 47, 75, 255, 196, 46, 75, 255, 196, 46, 75, 255, 195, 46, 75, 255, 195, 45, 75, 255, 195, 45, 75, 255, 194, 44, 74, 255, 194, 44, 74, 255, 193, 43, 74, 255, 193, 43, 74, 255, 192, 43, 74, 255, 192, 42, 74, 255, 192, 42, 74, 255, 191, 41, 74, 255, 180, 29, 71, 255, 179, 28, 71, 255, 179, 28, 71, 255, 178, 27, 71, 255, 178, 27, 71, 255, 177, 27, 71, 255, 177, 26, 70, 255, 177, 26, 70, 255, 176, 25, 70, 255, 176, 25, 70, 255, 175, 24, 70, 255, 175, 24, 70, 255, 174, 23, 70, 255, 174, 23, 70, 255, 174, 23, 70, 255, 173, 22, 70, 255, 173, 22, 69, 255, 172, 21, 69, 255, 172, 21, 69, 255, 171, 20, 69, 255, 171, 20, 69, 255, 171, 19, 69, 255, 170, 19, 69, 255, 170, 18, 69, 255, 169, 18, 69, 255, 169, 17, 68, 255, 168, 17, 68, 255, 168, 16, 68, 255, 168, 16, 68, 255, 167, 15, 68, 255, 167, 14, 68, 255, 166, 14, 68, 255, 166, 13, 68, 255, 165, 13, 68, 255, 165, 12, 67, 255, 164, 12, 67, 255, 164, 11, 67, 255, 164, 10, 67, 255, 163, 10, 67, 255, 163, 9, 67, 255, 162, 8, 67, 255, 162, 7, 67, 255, 161, 7, 67, 255, 161, 6, 66, 255, 161, 5, 66, 255, 160, 5, 66, 255, 160, 4, 66, 255, 159, 3, 66, 255, 159, 2, 66, 255, 158, 2, 66, 255, 158, 1, 66, 255};
static const heatmap_colorscheme_t cs_spectral_mixed = { mixed_data, sizeof(mixed_data)/sizeof(mixed_data[0])/4 };
const heatmap_colorscheme_t* heatmap_cs_default = &cs_spectral_mixed;

