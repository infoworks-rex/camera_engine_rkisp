/*
 * drm_display.h - drm display
 *
 *  Copyright (c) 2015 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: John Ye <john.ye@intel.com>
 */

#ifndef XCAM_DRM_DISPLAY_H
#define XCAM_DRM_DISPLAY_H

#include <xcam_std.h>
#include <xcam_mutex.h>
#include <v4l2_buffer_proxy.h>

extern "C" {
#define virtual vir
#include <drm.h>
#include <drm_mode.h>
#include <rk_intel_bufmgr.h>
#include <linux/videodev2.h>
#undef virtual
}

#include <errno.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <list>
#include <vector>
#include <map>

namespace XCam {

class DrmBoData;
class DrmBoBufferPool;
class DrmBoBuffer;

enum DrmDisplayMode {
    DRM_DISPLAY_MODE_NONE = 0,
    DRM_DISPLAY_MODE_PRIMARY,
    DRM_DISPLAY_MODE_OVERLAY,
};

class DrmDisplay {
    friend class DrmBoBufferPool;
    friend class CLBoBufferPool;

    struct FB {
        uint32_t fb_handle;
        uint32_t index;

        FB () : fb_handle (0), index (0) {}
    };

public:
    // if need local preview, please call set_preview() before instance()
    static SmartPtr<DrmDisplay> instance ();
    static uint32_t to_drm_fourcc (uint32_t fourcc_of_v4l2);

    virtual ~DrmDisplay();
    const char *get_module_name () const {
        return _module;
    }

    bool is_render_inited () const {
        return _is_render_inited;
    }
    XCamReturn render_init (
        uint32_t con_id,
        uint32_t crtc_id,
        uint32_t width,
        uint32_t height,
        uint32_t format,
        const struct v4l2_rect* compose);

    bool has_frame_buffer (SmartPtr<VideoBuffer> &buf) {
        return _buf_fb_handles.find (buf.ptr ()) != _buf_fb_handles.end ();
    };
    XCamReturn render_setup_frame_buffer (SmartPtr<VideoBuffer> &buf);
    XCamReturn render_buffer (SmartPtr<VideoBuffer> &buf);

    int get_drm_handle() const {
        return _fd;
    };

    SmartPtr<V4l2Buffer> create_drm_buf (
        const struct v4l2_format &format,
        const uint32_t index,
        const enum v4l2_buf_type buf_type,
        struct v4l2_plane *planes,
        int num_plane);
    SmartPtr<DrmBoBuffer> convert_to_drm_bo_buf (SmartPtr<DrmDisplay> &self, SmartPtr<VideoBuffer> &buf_in);

    static bool set_preview (bool flag);
    bool can_preview () {
        return _preview_flag;
    }
    bool set_display_mode (DrmDisplayMode mode) {
        _display_mode = mode;
        return true;
    };

private:
    DrmDisplay (const char *module = NULL);

    SmartPtr<DrmBoData> create_drm_bo (SmartPtr<DrmDisplay> &self, const VideoBufferInfo& info);
    drm_intel_bo *create_drm_bo_from_fd (int32_t fd, uint32_t size);

    bool is_authenticated (int fd, const char *msg);
    int open_driver (const char *dev_path);
    int open_drivers (const char *base_path, int base_id);

    XCamReturn get_crtc(drmModeRes *res);
    XCamReturn get_connector(drmModeRes *res);
    XCamReturn get_plane();
    XCamReturn set_plane(const FB &fb);
    XCamReturn set_crtc(const FB &fb);
    XCamReturn page_flip(const FB &fb);

private:
    typedef std::map<const VideoBuffer *, FB> FBMap;

    static bool    _preview_flag;

    char *_module;
    int _fd;
    drm_intel_bufmgr *_buf_manager;
    DrmDisplayMode _display_mode;
    int _crtc_index;
    unsigned int _crtc_id;
    unsigned int _con_id;
    unsigned int _encoder_id;
    unsigned int _plane_id;
    drmModeModeInfo _mode;
    drmModeConnector *_connector;
    bool _is_render_inited;

    unsigned int _format;
    unsigned int _width;
    unsigned int _height;

    struct v4l2_rect _compose;

    FBMap _buf_fb_handles;
    SmartPtr<VideoBuffer>  _display_buf;

private:
    XCAM_DEAD_COPY (DrmDisplay);

private:
    static SmartPtr<DrmDisplay> _instance;
    static Mutex                _mutex;
};

};
#endif // XCAM_DRM_DISPLAY_H

