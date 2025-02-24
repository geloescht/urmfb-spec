/*
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

struct FBRequest {
    unsigned width;
    unsigned height;
    enum PixelFormat {
        PixelFormat_any = 0,
        PixelFormat_RGB_565 = 1,
        PixelFormat_RGB_888 = 2,
        PixelFormat_RGBA_8888 = 3
    } pix_fmt;
    enum Orientation {
        Orientation_any = 0,
        Orientation_0_deg = 1,
        Orientation_90_deg = 2,
        Orientation_180_deg = 3,
        Orientation_270_deg = 4,
        Orientation_0_deg_mirr = 5,
        Orientation_90_deg_mirr = 6,
        Orientation_180_deg_mirr = 7,
        Orientation_270_deg_mirr = 8,
    } orientation;
    unsigned line_stride;
    
    void *data;
    
    /* The following fields are reserved for future extensions of the
       specification */
    unsigned reserved[9];
};

enum UpdateMode {
    UpdateMode_any = 0,
    UpdateMode_clear = 1,
    UpdateMode_direct = 2,
    UpdateMode_hq = 3,
    UpdateMode_mq = 4,
    UpdateMode_fast = 5
};

struct FBRequest *urmfb_acquire(struct FBRequest requests[], unsigned n_requests);
void urmfb_update_sync(void *, unsigned, unsigned, unsigned, unsigned, enum UpdateMode);
int urmfb_update_async(void *, unsigned, unsigned, unsigned, unsigned, enum UpdateMode);
void urmfb_await_update(int handle);
void urmfb_release(void *fb);

