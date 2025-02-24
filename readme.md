Specification for universal framebuffer bindings on reMarkable tablets
===================================================================

Motivation
-------------------------------------------------------------------
At the moment, various competing implementations of acquiring a
framebuffer for the different reMarkable devices (1, 2 and Paper Pro)
with different firmware compatibility exist within the homebrew
scene: Raw /dev/fb0 access, rm2fb, qtfb, access through libgsqepaper.so,
etc.
Applications ported to or developed for the reMarkable have varying
levels of support for those framebuffer formats. This document aims to
standardise a set of function signatures for the C language, that
provide negotiation and acquisition of a framebuffer, updates of portions
of the screen and releasing the framebuffer. These routines shall be kept
outside the application binary and linked dynamically to whatever
implementation of this specification the user of the tablet chooses to use.

Negotiation and acquisition of a framebuffer
--------------------------------------------------------------------

For negotiating a framebuffer, a client application shall fill out
an array of the following structure:
```
struct FBRequest {
    /* The width and height of the visible portion of the logical framebuffer.
       It might differ from the physical dimensions of the screen (see 
       field orientation) and the memory layout of the framebuffer (see
       field line_stride). */
    unsigned width;
    unsigned height;
    
    /* The format of each pixel of the framebuffer. This also defines the
       size in bits of one pixel and the data pointer will need to be cast
       to a C type of the same size before usage. */
    enum PixelFormat {
        PixelFormat_any = 0,
        PixelFormat_BGR_565 = 1,
        PixelFormat_RGB_888 = 2,
        PixelFormat_RGBA_8888 = 3
    } pix_fmt;
    
    /* The type of transformation between the logical orientation of the
       framebuffer, where data points to the top-left pixel of the framebuffer
       and (data + height*line_stride) points to the bottom-left pixel of the
       framebuffer, and the physical screen while holding the device in its
       most common orientation as intended by the manufacturer (w/o dynamic
       orientation changes). */
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
    
    /* The number of pixels that need to be skipped to advance by one line.
       If the application cannot handle a line_stride differing from the width
       of the framebuffer, it shall set this field to the logical width it
       requires and set width to 0 */
    unsigned line_stride;
    
    /* A pointer to the pixel data of the framebuffer, starting at the
       logical top-left. This field is only for the result of the negotiation
       process and does not need to be filled by the client application */
    void *data;
    
    /* The following fields are reserved for future extensions of the
       specification */
    unsigned reserved[9];
};
```
The array shall contain a list of framebuffer formats it can work with
in order of preference, where the element with index 0 is that with
the highest priority. The application can set any field to 0 to signify
that it will accept any value for this field.

It shall then pass this array and its size to the following function:
`struct FBRequest *urmfb_acquire(struct FBRequest requests[], unsigned n_requests)`

The implementation of urmfb_acquire() shall choose the request with the
highest priority that it can satisfy.

It shall fill the field data with a valid pointer to the first byte of the
acquired framebuffer. It shall fill the fields that have been set to 0 by
the application with the actual value corresponding to the acquired
framebuffer.

It shall return a pointer to the chosen and filled request structure or,
in case it cannot satisfy any of the requests, shall return 0.

The implementation shall return 0 in case it cannot provide more than
one framebuffer at a time and a framebuffer has already been acquired
and not released by this or another application.

After examining the contents of the request structure returned by
`urmfb_acquire()` and saving at least the field data to its internal data
structures, the application may deallocate the array of request structures.
The framebuffer address returned in the field data serves as a handle for
other functions of this specification.

Working with the framebuffer
------------------------------------------------------------------------

The application shall cast the field data of the FBRequest structure returned
by `urmfb_acquire()` to a type compatible with the pixel format specified
in the field pix_fmt and use the cast pointer for all interactions with the
framebuffer.

The application shall only read or write to data pointed to by a pointer of
in the following range:
`(fb_ptr + x + (line_stride * y)`
where
`0 <= x < width` and `0 <= y < height`

Care shall be taken not to access any data where `width <= x < line_stride`!

The implementation may or may not update the screen to reflect the contents
of the framebuffer at any time, but subroutines shall be provided to request
such an update.

Requesting screen updates
------------------------------------------------------------------------

To update portions of the screen to reflect the contents of the framebuffer,
the application may call one of the following function:

`void urmfb_update_sync(void *fb, unsigned left, unsigned top, unsigned right, unsigned bottom, enum UpdateMode update_mode)`

`int urmfb_update_async(void *fb, unsigned left, unsigned top, unsigned right, unsigned bottom, enum UpdateMode update_mode)`

where fb is set to the first byte of an acquired and unreleased framebuffer,
left, top, right and bottom correspond to the distances of the updated
areas from the virtual top left corner of the framebuffer, and update_mode
is one of the following values:
```
enum UpdateMode {
    UpdateMode_any = 0, /* Use whatever update mode is the current default for each pixel inside the specified area */
    UpdateMode_clear = 1,
    UpdateMode_direct = 2,
    UpdateMode_hq = 3,
    UpdateMode_mq = 4,
    UpdateMode_fast = 5
    /* More update modes may be defined in future iterations of this specification
       in the range 6 - 255 */
    /* The implemenation may define implementation specific update modes in the range
       257 - 65535, provided that its bottom 8 bits equal one of the universally
       defined update modes from this specification that other implementations may
       choosd to use in place of the implementation specific update mode. Implementation
       providers shall take care not to reuse the same values already in use by another
       implementation for their implementation specific update modes.*/
};
```

The implementation shall update the screen using the requested update mode
or its closest analogue available. It shall wait for the update to complete
if `urmfb_update_sync()` has been called, whereas `urmfb_update_async()`
shall return immediately after requesting the update.

An implementation that cannot wait for a screen update to complete from within
`urmfb_update_sync()` shall sleep for a constant number of milliseconds depending
on the mode of the update it has performed before returning.

The implementation may optionally return a handle from `urmfb_update_async()`
or 0 in case it cannot check for the completion of the update after it has
been submitted.

The application can call `urmfb_update_async()` multiple times while updates
are being performed by the screen. The implementation shall ensure that
update requests are queued properly. It may return the same handle for
multiple updates.

In case the application has received a valid handle from `urmfb_update_async()`,
it may wait for its completion by calling

`void urmfb_await_update(int handle)`

An implementation that does not support waiting for the completion of a
previously submitted update shall return immediately.

Otherwise the implementation shall not return until the update referred
to by the handle has been completed by the screen. It shall return
immediately if the update has been already performed or 0 was passed
to the function.

Releasing the framebuffer
------------------------------------------------------------------------

To release the framebuffer, the application shall call the following
function with the pointer to the first byte of the framebuffer:

`void urmfb_release(void *fb)`

The implementation shall de-allocate all internal data and release
locks and other resources associated with the framebuffer. In case
one or more of these operations fail, it shall not return an error
but may print a message on stderr.


Accessing the universal framebuffer bindings
-------------------------------------------------------------------------
Client applications wishing to access the universal framebuffer binding
routines shall link `liburmfb.so`. The user is responsible for providing
an implementation compatible with their device and firmware in a standard
location within LD_LIBRARY_PATH such as '/usr/lib' or to pre-load a 
compatible implementation through LD_PRELOAD.

Equivalent bindings can be provided for other programming languages by
calling the functions provided inside `liburmfb.so` in a standard location on
the device through a FFI (foreign function interface) or an adapter module.

The implementation may acquire and manage the framebuffer directly within
its library code or act as a shim and delegate those functions to other
software on the device.

The implementations of these subroutines may be unsafe to call from more than
one thread. The application must make sure to call any of the functions
defined in this specification only from the same thread as it called
`urmfb_acquire()` from.
