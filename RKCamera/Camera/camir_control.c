/*
 * Copyright (C) 2019 Rockchip Electronics Co., Ltd.
 * author: Zhihua Wang, hogan.wang@rock-chips.com
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL), available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <pthread.h>
#include <stdint.h>

#ifdef CAMERA_ENGINE_RKISP
#include <camera_engine_rkisp/interface/rkisp_api.h>
#endif
//#ifdef CAMERA_ENGINE_RKAIQ
#include <rkaiq/rkisp_api.h>
//#endif
//#include "rga_control.h"
#include <linux/media-bus-format.h>
#include "camir_control.h"

#ifndef ISC_TRUE
#define ISC_TRUE (1)
#endif

#ifndef ISC_FALSE
#define ISC_FALSE (0)
#endif

#ifndef _HAL_TRANSFORM_
/* flip source image horizontally (around the vertical axis) */
#define HAL_TRANSFORM_FLIP_H     0x01
/* flip source image vertically (around the horizontal axis)*/
#define HAL_TRANSFORM_FLIP_V     0x02
/* rotate source image 90 degrees clockwise */
#define HAL_TRANSFORM_ROT_90     0x04
/* rotate source image 180 degrees */
#define HAL_TRANSFORM_ROT_180    0x03
/* rotate source image 270 degrees clockwise */
#define HAL_TRANSFORM_ROT_270    0x07
#endif

static const struct rkisp_api_ctx *ctx;
static const struct rkisp_api_buf *buf;
static int volatile g_run;
static pthread_t g_tid;

static int g_ir_en;
static int g_ir_width;
static int g_ir_height;
static display_ircallback g_display_cb = NULL;
static pthread_mutex_t g_display_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_rotation = HAL_TRANSFORM_ROT_270;
static display_ircallback g_process_cb = NULL;

void set_ir_rotation(int angle)
{
    if (angle == 90)
        g_rotation = HAL_TRANSFORM_ROT_90;
    else if (angle == 270)
        g_rotation = HAL_TRANSFORM_ROT_270;
    else if(angle == 180)
        g_rotation = HAL_TRANSFORM_ROT_180;
}

void set_ir_display(display_ircallback cb)
{
    pthread_mutex_lock(&g_display_lock);
    g_display_cb = cb;
    pthread_mutex_unlock(&g_display_lock);
}

void set_ir_process(display_ircallback cb)
{
    pthread_mutex_lock(&g_display_lock);
    g_process_cb = cb;
    pthread_mutex_unlock(&g_display_lock);
}

void set_ir_param(int width, int height, display_ircallback cb)
{
    g_ir_en = ISC_TRUE;
    g_ir_width = width;
    g_ir_height = height;
    set_ir_display(cb);
}

static inline void camrgb_inc_fps(void)
{
    static int fps = 0;
    static struct timeval t0;
    struct timeval t1;

    fps++;
    gettimeofday(&t1, NULL);
    if (t1.tv_sec - t0.tv_sec > 1)
    {
        fps = 0;
        gettimeofday(&t0, NULL);
    } else if ((t1.tv_sec - t0.tv_sec) * 1000000 + (t1.tv_usec - t0.tv_usec) > 1000000)
    {
        printf("ISP ir fps: %d\n", fps);
        fps = 0;
        gettimeofday(&t0, NULL);
    }
}

static void *process(void *arg)
{
    (void)arg;
#if 1
    set_ir_rotation(g_rotation);
    do
    {
#if 0
        camrgb_inc_fps();
#endif
        buf = rkisp_get_frame(ctx, 0);
        if (buf == NULL)
        {
            continue;
        }

        if (g_process_cb)g_process_cb(buf->buf, 1, RK_FORMAT_YCbCr_420_SP, ctx->width, ctx->height, g_rotation);
#if 1
        pthread_mutex_lock(&g_display_lock);
        if (g_display_cb)g_display_cb(buf->buf, 2, RK_FORMAT_YCbCr_420_SP, ctx->width, ctx->height, g_rotation);
        pthread_mutex_unlock(&g_display_lock);
#endif
        rkisp_put_frame(ctx, buf);
    } while (g_run);

    pthread_exit(NULL);
#endif
}

int camir_control_init(void)
{
    if (!g_ir_en)
        return 0;

#ifdef CAMERA_ENGINE_RKISP
    int id = get_video_id("stream_cif_dvp");
    if (id < 0)
    {
        printf("%s: get video id fail!\n", __func__);
        return -1;
    }

    snprintf(name, sizeof(name), "/dev/video%d", id);
    printf("%s: %s\n", __func__, name);
    ctx = rkisp_open_device(name, 0);
#endif
    //#ifdef CAMERA_ENGINE_RKAIQ
    ctx = rkisp_open_device2(CAM_TYPE_RKCIF);
    //#endif
    if (ctx == NULL)
    {
        printf("%s: ctx is NULL\n", __func__);
        return -1;
    }

    rkisp_set_buf(ctx, 3, NULL, 0);

    //    rkisp_set_sensor_fmt(ctx, g_ir_width, g_ir_height, MEDIA_BUS_FMT_YUYV8_2X8);
    if (rkisp_set_fmt(ctx, g_ir_width, g_ir_height, V4L2_PIX_FMT_NV12))
        return -1;

    if (rkisp_start_capture(ctx))
        return -1;

    g_run = ISC_TRUE;
    if (pthread_create(&g_tid, NULL, process, NULL))
    {
        printf("pthread_create fail\n");
        return -1;
    }

    return 0;
}

void camir_control_exit(void)
{
    if (!g_ir_en)
        return;
    g_run = ISC_FALSE;
    if (g_tid)
    {
        pthread_join(g_tid, NULL);
        g_tid = 0;
    }

    rkisp_stop_capture(ctx);

    rkisp_close_device(ctx);
}

int camir_control_run(void)
{
    return g_run;
}
