// Automatic rescan for USB MIDI devices (c) 2015 Micah Elizabeth Scott
// MIT license

#include "MacMidiHotplug.h"
#include <CoreMIDI/CoreMIDI.h>
#include <ostream>

using namespace std;

static const CFTimeInterval kMidiRescanDelay = 1.5f;


void MacMidiHotplug::start()
{
    mThread = thread(bind(&MacMidiHotplug::threadFunc, this));
}

void MacMidiHotplug::stop()
{
    if (mRunLoop) {
        CFRunLoopStop(mRunLoop);
        mThread.join();
        CFRelease(mRunLoop);
        mRunLoop = 0;
    }
}

void MacMidiHotplug::threadFunc()
{
    CFRunLoopSourceRef source;
    IONotificationPortRef port;
    io_iterator_t deviceAdded;

    mRunLoop = CFRunLoopGetCurrent();
    CFRetain(mRunLoop);

    // Get notified when devices are plugged in
    port = IONotificationPortCreate(kIOMasterPortDefault);
    source = IONotificationPortGetRunLoopSource(port);
    CFRunLoopAddSource(mRunLoop, source, kCFRunLoopDefaultMode);
    IOServiceAddMatchingNotification(port, kIOFirstMatchNotification,
                                     IOServiceMatching(kIOUSBDeviceClassName),
                                     (IOServiceMatchingCallback)deviceAddedFunc,
                                     this, &deviceAdded);
    clearIterator(deviceAdded);

    // Use a timer to coalesce these events and avoid rescanning MIDI too often,
    // which seems to cause problems.
    
    mRescanTimer = CFRunLoopTimerCreateWithHandler(
           kCFAllocatorDefault,
           CFAbsoluteTimeGetCurrent() + kMidiRescanDelay,
           1e9, 0, 0,
           ^(CFRunLoopTimerRef timer) {
               // Re-enumerate available MIDI devices
               cerr << "[MIDI] Re-enumerating attached devices..." << endl;
               MIDIRestart();
           });

    CFRunLoopAddTimer(mRunLoop, mRescanTimer, kCFRunLoopDefaultMode);
    
    CFRunLoopRun();
}

void MacMidiHotplug::clearIterator(io_iterator_t iter)
{
    io_service_t device;
    while ((device = IOIteratorNext(iter)) != 0) {
        IOObjectRelease(device);
    }
}

void MacMidiHotplug::deviceAddedFunc(void *context, io_iterator_t devices)
{
    auto self = static_cast<MacMidiHotplug*>(context);
    CFRunLoopTimerSetNextFireDate(self->mRescanTimer, CFAbsoluteTimeGetCurrent() + kMidiRescanDelay);
    clearIterator(devices);
}
