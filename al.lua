-- Copyright 2023-2025, Mansour Moufid <mansourmoufid@gmail.com>

-- This file is part of Aluminium Library.
--
-- Aluminium Library is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by the
-- Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- Aluminium Library is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
-- See the GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License along
-- with Aluminium Library. If not, see <https://www.gnu.org/licenses/>.

local ffi = require('ffi')

local str = {}

function str.join(sep, xs)
    if #xs == 0 then
        return ''
    end
    local ys = xs[1]
    for i = 2, #xs do
        ys = ys .. sep .. xs[i]
    end
    return ys
end

function str.split(s, sep)
    local xs = {}
    local j = 1
    for i = 1, #s do
        local a = i
        local b = i + #sep - 1
        if string.sub(s, a, b) == sep then
            xs[#xs + 1] = string.sub(s, j, i - 1)
            j = i + #sep
        end
    end
    if j <= #s then
        xs[#xs + 1] = string.sub(s, j, -1)
    end
    return xs
end

local al = {}

local libal = ffi.load('al')

ffi.cdef('void free(void *ptr);')

ffi.cdef([[
enum al_status {
    AL_OK = 0,
    AL_ERROR = 1,
    AL_NOTIMPLEMENTED = 2,
    AL_NOMEMORY = 3,
};

enum al_color_format {
    AL_COLOR_FORMAT_UNKNOWN = 0,
    AL_COLOR_FORMAT_YUV420SP = 1,
    AL_COLOR_FORMAT_YUV420P = 2,
    AL_COLOR_FORMAT_RGBA = 3,
};

enum al_camera_facing {
    AL_CAMERA_FACING_FRONT = 0,
    AL_CAMERA_FACING_BACK = 1,
};

void al_init(void);

extern const char *const copyright;
extern const char *const platform;
const char *al_locale(void);
const char *al_datadir(void);
const char *al_libdir(void);

bool al_permissions_have(const char *);
void al_permissions_request(const char *);

int al_display_orientation(void);

char *al_net_get_local_ip_address(void);

struct al_camera;
enum al_status al_camera_new(struct al_camera **, size_t, size_t, size_t);
void al_camera_free(struct al_camera *);
void al_camera_start(struct al_camera *);
void al_camera_stop(struct al_camera *);
enum al_status al_camera_get_id(struct al_camera *, const char **);
enum al_status al_camera_get_color_format(struct al_camera *, enum al_color_format *);
enum al_status al_camera_get_width(struct al_camera *, size_t *);
enum al_status al_camera_get_height(struct al_camera *, size_t *);
enum al_status al_camera_get_data(struct al_camera *, enum al_color_format, void **);
enum al_status al_camera_get_rgba(struct al_camera *, void **);
enum al_status al_camera_get_facing(struct al_camera *, enum al_camera_facing *);
enum al_status al_camera_get_orientation(struct al_camera *, int *);
enum al_status al_camera_set_stride(struct al_camera *, size_t);

struct al_image {
    size_t width;
    size_t height;
    size_t stride;
    void *restrict data;
    enum al_color_format format;
};
enum al_status al_image_alloc(struct al_image *);
void al_image_free(struct al_image *);
enum al_status al_image_convert(struct al_image *, struct al_image *);
enum al_status al_image_rotate(struct al_image *, struct al_image *, int);
enum al_status al_image_copy(const struct al_image *, struct al_image *);
]])

al.OK = 0
al.ERROR = 1
al.NOTIMPLEMENTED = 2
al.NOMEMORY = 3

al.COLOR_FORMAT_UNKNOWN = 0
al.COLOR_FORMAT_YUV420SP = 1
al.COLOR_FORMAT_YUV420P = 2
al.COLOR_FORMAT_RGBA = 3

al.CAMERA_FACING_FRONT = 0
al.CAMERA_FACING_BACK = 1

al.platform = ffi.string(libal.platform)

function al.init()
end

if al.platform == 'android' then
    ffi.cdef([[
        void *al_android_getactivity(const char *, const char *);
        int al_android_multicast_lock_acquire(void);
        int al_android_multicast_lock_release(void);
    ]])
end

if al.platform == 'android' then
    function al.init(args)
        libal.al_android_getactivity(
            args.activity.class,
            args.activity.field
        )
    end
    function al.multicast_lock_acquire()
        return libal.al_android_multicast_lock_acquire()
    end
    function al.multicast_lock_release()
        return libal.al_android_multicast_lock_release()
    end
end

function al.locale()
    local x = libal.al_locale()
    if x == nil then
        return nil
    end
    return ffi.string(x)
end

function al.getlang()
    local locale = al.locale()
    if locale == nil then
        return nil
    end
    -- [language]-[script]_[region].[encoding]
    local x, _ = unpack(str.split(locale, '.'))
    local y, _ = unpack(str.split(x, '_'))
    local language, _ = unpack(str.split(y, '-'))
    return language
end

function al.datadir()
    local dir = libal.al_datadir()
    if dir == nil then
        return nil
    end
    return ffi.string(dir)
end

function al.libdir()
    local dir = libal.al_libdir()
    if dir == nil then
        return nil
    end
    return ffi.string(dir)
end

al.permissions = {}

if al.platform == 'android' then
    al.permissions.camera = 'android.permission.CAMERA'
    al.permissions.internet = 'android.permission.INTERNET'
end

if al.platform == 'darwin' then
    al.permissions.camera = 'com.apple.security.device.camera'
    al.permissions.internet = 'com.apple.security.network.client'
end

function al.permissions.have(permission)
    return libal.al_permissions_have(permission)
end

function al.permissions.request(arg)
    if arg == nil then
        return
    end
    assert(type(arg) == 'string' or type(arg) == 'table')
    if type(arg) == 'string' then
        libal.al_permissions_request(arg)
    elseif type(arg) == 'table' then
        for _, permission in ipairs(arg) do
            al.permissions.request(permission)
        end
    end
end

al.display = {}

function al.display.orientation()
    return tonumber(libal.al_display_orientation())
end

al.net = {}

if al.platform == 'android' then
    function al.net.multicast_lock_acquire()
        return libal.al_android_multicast_lock_acquire()
    end
    function al.net.multicast_lock_release()
        return libal.al_android_multicast_lock_release()
    end
end

function al.net.get_local_ip_address()
    local ip = libal.al_net_get_local_ip_address()
    if ip == nil then
        return nil
    end
    local s = ffi.string(ip)
    s = string.gsub(s, '/.*', '')
    return s
end

al.camera = {}

function al.camera.new(index, width, height)
    local p = ffi.new('struct al_camera *[1]')
    local status = libal.al_camera_new(
        p,
        index or 0,
        width or 640,
        height or 480
    )
    if status == al.OK then
        local camera = p[0]
        return camera
    else
        return nil
    end
end

function al.camera.free(camera)
    if camera == nil then
        return
    end
    libal.al_camera_free(camera)
end

function al.camera.start(camera)
    return libal.al_camera_start(camera)
end

function al.camera.stop(camera)
    if camera == nil then
        return
    end
    return libal.al_camera_stop(camera)
end

function al.camera.id(camera)
    local id = ffi.new('const char * [1]')
    local status = libal.al_camera_get_id(camera, id)
    if status == al.OK then
        if id[0] == nil then
            return nil
        else
            return ffi.string(id[0])
        end
    end
    return nil
end

function al.camera.color_format(camera, format)
    if format == nil then
        local fmt = ffi.new('int [1]')
        local status = libal.al_camera_get_color_format(camera, fmt)
        if status == al.OK then
            return tonumber(fmt[0])
        end
        return nil
    else
    end
end

function al.camera.size(camera)
    return al.camera.width(camera), al.camera.height(camera)
end

function al.camera.width(camera)
    local w = ffi.new('size_t [1]')
    local status = libal.al_camera_get_width(camera, w)
    if status == al.OK then
        return tonumber(w[0])
    end
    return nil
end

function al.camera.height(camera)
    local h = ffi.new('size_t [1]')
    local status = libal.al_camera_get_height(camera, h)
    if status == al.OK then
        return tonumber(h[0])
    end
    return nil
end

function al.camera.dimensions(camera)
    return {al.camera.width(camera), al.camera.height(camera)}
end

function al.camera.data(camera, format)
    if format == nil then
        format = al.COLOR_FORMAT_UNKNOWN
    end
    local data = ffi.new('void * [1]')
    local status = libal.al_camera_get_data(camera, format, data)
    if status == al.OK then
        return data[0]
    end
    return nil
end

function al.camera.rgba(camera)
    local data = ffi.new('void * [1]')
    local status = libal.al_camera_get_rgba(camera, data)
    if status == al.OK then
        return data[0]
    end
    return nil
end

function al.camera.facing(camera)
    local facing = ffi.new('int [1]')
    local status = libal.al_camera_get_facing(camera, facing)
    if status == al.OK then
        if facing[0] == al.CAMERA_FACING_FRONT then
            return 'front'
        elseif facing[0] == al.CAMERA_FACING_BACK then
            return 'back'
        end
    end
    return nil
end

function al.camera.orientation(camera)
    local orientation = ffi.new('int [1]')
    local status = libal.al_camera_get_orientation(camera, orientation)
    if status == al.OK then
        return tonumber(orientation[0])
    end
    return nil
end

function al.camera.stride(camera, stride)
    libal.al_camera_set_stride(camera, stride)
end

al.image = {}

al.video = {}

return al
