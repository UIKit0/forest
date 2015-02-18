// Automatic rescan for USB MIDI devices (c) 2015 Micah Elizabeth Scott
// MIT license

#pragma once

#include <IOKit/IOTypes.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <mach/mach_host.h>
#include <mach/mach_port.h>
#include <thread>


class MacMidiHotplug {
public:
    void start();
    void stop();

private:
    std::thread mThread;
    CFRunLoopRef mRunLoop;
    
    void threadFunc();
    static void clearIterator(io_iterator_t iter);
    static void deviceAddedFunc(void *context, io_iterator_t devices);
};
