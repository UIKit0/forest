// Automatic rescan for USB MIDI devices (c) 2015 Micah Elizabeth Scott
// MIT license

#include "MacMidiHotplug.h"
#include <CoreMIDI/CoreMIDI.h>
#include <ostream>

using namespace std;


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
    port = IONotificationPortCreate(kIOMasterPortDefault);
    source = IONotificationPortGetRunLoopSource(port);
    CFRunLoopAddSource(mRunLoop, source, kCFRunLoopDefaultMode);
    
    IOServiceAddMatchingNotification(port, kIOFirstMatchNotification,
                                     IOServiceMatching(kIOUSBDeviceClassName),
                                     (IOServiceMatchingCallback)deviceAddedFunc,
                                     this, &deviceAdded);
 
    deviceAddedFunc(this, deviceAdded);
    
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
    // Re-enumerate available MIDI devices
    cerr << "[MIDI] Re-enumerating attached devices..." << endl;
    MIDIRestart();

    clearIterator(devices);
}
