/*
 ==============================================================================

 This file is part of the JUCE library - "Jules' Utility Class Extensions"
 Copyright 2004-11 by Raw Material Software Ltd.

 ------------------------------------------------------------------------------

 JUCE can be redistributed and/or modified under the terms of the GNU General
 Public License (Version 2), as published by the Free Software Foundation.
 A copy of the license is included in the JUCE distribution, or can be found
 online at www.gnu.org/licenses.

 JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

 ------------------------------------------------------------------------------

 To release a closed-source product which uses JUCE, commercial licenses are
 available: visit www.rawmaterialsoftware.com/juce for more information.

 ==============================================================================
 */


// Your project must contain an AppConfig.h file with your project-specific settings in it,
// and your header search path must make it accessible to the module's files.
#include "AppConfig.h"
#include "../utility/juce_CheckSettingMacros.h"

#if JucePlugin_Build_VST3

#define JUCE_MAC_WINDOW_VISIBITY_BODGE 1

#include "../utility/juce_IncludeSystemHeaders.h"
#include "../utility/juce_IncludeModuleHeaders.h"
#include "../utility/juce_FakeMouseMoveGenerator.h"
#include "../utility/juce_CarbonVisibility.h"

#undef Component
#undef Point

//==============================================================================
namespace juce
{
    static void initialiseMac()
    {
       #if ! JUCE_64BIT
        NSApplicationLoad();
       #endif
    }

   #if ! JUCE_64BIT
    static void updateComponentPos (Component* const comp)
    {
        DBG ("updateComponentPos()");

        HIViewRef dummyView = (HIViewRef) (void*) (pointer_sized_int)
        comp->getProperties() ["dummyViewRef"].toString().getHexValue64();

        HIRect r;
        HIViewGetFrame (dummyView, &r);
        HIViewRef root;
        HIViewFindByID (HIViewGetRoot (HIViewGetWindow (dummyView)), kHIViewWindowContentID, &root);
        HIViewConvertRect (&r, HIViewGetSuperview (dummyView), root);

        Rect windowPos;
        GetWindowBounds (HIViewGetWindow (dummyView), kWindowContentRgn, &windowPos);

        comp->setTopLeftPosition ((int) (windowPos.left + r.origin.x),
                                  (int) (windowPos.top + r.origin.y));
    }

    static pascal OSStatus viewBoundsChangedEvent (EventHandlerCallRef, EventRef, void* user)
    {
        updateComponentPos ((Component*) user);
        return noErr;
    }
   #endif

    static void* attachComponentToWindowRef (Component* comp, void* windowRef, bool isHIView)
    {
        DBG ("attachComponentToWindowRef()");

        JUCE_AUTORELEASEPOOL
        {
       #if JUCE_64BIT
        NSView* parentView = (NSView*) windowRef;

       #if JucePlugin_EditorRequiresKeyboardFocus
        comp->addToDesktop (0, parentView);
       #else
        comp->addToDesktop (ComponentPeer::windowIgnoresKeyPresses, parentView);
       #endif

        // (this workaround is because Wavelab provides a zero-size parent view..)
        if ([parentView frame].size.height == 0)
            [((NSView*) comp->getWindowHandle()) setFrameOrigin: NSZeroPoint];

        comp->setVisible (true);
        comp->toFront (false);

        [[parentView window] setAcceptsMouseMovedEvents: YES];
        return parentView;

       #else
        //treat NSView like 64bit
        if (! isHIView)
        {
            NSView* parentView = (NSView*) windowRef;

           #if JucePlugin_EditorRequiresKeyboardFocus
            comp->addToDesktop (0, parentView);
           #else
            comp->addToDesktop (ComponentPeer::windowIgnoresKeyPresses, parentView);
           #endif

            // (this workaround is because Wavelab provides a zero-size parent view..)
            if ([parentView frame].size.height == 0)
                [((NSView*) comp->getWindowHandle()) setFrameOrigin: NSZeroPoint];

            comp->setVisible (true);
            comp->toFront (false);

            [[parentView window] setAcceptsMouseMovedEvents: YES];
            return parentView;
        }

        NSWindow* hostWindow = [[NSWindow alloc] initWithWindowRef: windowRef];
        [hostWindow retain];
        [hostWindow setCanHide: YES];
        [hostWindow setReleasedWhenClosed: YES];

        HIViewRef parentView = nullptr;

        WindowAttributes attributes;
        GetWindowAttributes ((WindowRef) windowRef, &attributes);

        if ((attributes & kWindowCompositingAttribute) != 0)
        {
            HIViewRef root = HIViewGetRoot ((WindowRef) windowRef);
            HIViewFindByID (root, kHIViewWindowContentID, &parentView);

            if (parentView == nullptr)
                parentView = root;
        }
        else
        {
            GetRootControl ((WindowRef) windowRef, (ControlRef*) &parentView);

            if (parentView == nullptr)
                CreateRootControl ((WindowRef) windowRef, (ControlRef*) &parentView);
        }

        // It seems that the only way to successfully position our overlaid window is by putting a dummy
        // HIView into the host's carbon window, and then catching events to see when it gets repositioned
        HIViewRef dummyView = 0;
        HIImageViewCreate (0, &dummyView);
        HIRect r = { {0, 0}, { (float) comp->getWidth(), (float) comp->getHeight()} };
        HIViewSetFrame (dummyView, &r);
        HIViewAddSubview (parentView, dummyView);
        comp->getProperties().set ("dummyViewRef", String::toHexString ((pointer_sized_int) (void*) dummyView));

        EventHandlerRef ref;
        const EventTypeSpec kControlBoundsChangedEvent = { kEventClassControl, kEventControlBoundsChanged };
        InstallEventHandler (GetControlEventTarget (dummyView), NewEventHandlerUPP (viewBoundsChangedEvent), 1, &kControlBoundsChangedEvent, (void*) comp, &ref);
        comp->getProperties().set ("boundsEventRef", String::toHexString ((pointer_sized_int) (void*) ref));

        updateComponentPos (comp);

       #if ! JucePlugin_EditorRequiresKeyboardFocus
        comp->addToDesktop (ComponentPeer::windowIsTemporary | ComponentPeer::windowIgnoresKeyPresses);
       #else
        comp->addToDesktop (ComponentPeer::windowIsTemporary);
       #endif

        comp->setVisible (true);
        comp->toFront (false);

        NSView* pluginView = (NSView*) comp->getWindowHandle();
        NSWindow* pluginWindow = [pluginView window];
        [pluginWindow setExcludedFromWindowsMenu: YES];
        [pluginWindow setCanHide: YES];

        [hostWindow addChildWindow: pluginWindow
                           ordered: NSWindowAbove];
        [hostWindow orderFront: nil];
        [pluginWindow orderFront: nil];

        attachWindowHidingHooks (comp, (WindowRef) windowRef, hostWindow);

        return hostWindow;
       #endif

       }
    }

    static void detachComponentFromWindowRef (Component* comp, void* nsWindow, bool isHIView)
    {
       #if JUCE_64BIT
        comp->removeFromDesktop();
       #else
        //treat NSView like 64bit
        if (! isHIView)
        {
            comp->removeFromDesktop();
        }
        else
        {
            JUCE_AUTORELEASEPOOL
            {
                EventHandlerRef ref = (EventHandlerRef) (void*) (pointer_sized_int)
                comp->getProperties() ["boundsEventRef"].toString().getHexValue64();
                RemoveEventHandler (ref);

                removeWindowHidingHooks (comp);

                HIViewRef dummyView = (HIViewRef) (void*) (pointer_sized_int)
                comp->getProperties() ["dummyViewRef"].toString().getHexValue64();

                if (HIViewIsValid (dummyView))
                    CFRelease (dummyView);

                NSWindow* hostWindow = (NSWindow*) nsWindow;
                NSView* pluginView = (NSView*) comp->getWindowHandle();
                NSWindow* pluginWindow = [pluginView window];

                [hostWindow removeChildWindow: pluginWindow];
                comp->removeFromDesktop();

                [hostWindow release];
            }

            // The event loop needs to be run between closing the window and deleting the plugin,
            // presumably to let the cocoa objects get tidied up. Leaving out this line causes crashes
            // in Live and Reaper when you delete the plugin with its window open.
            // (Doing it this way rather than using a single longer timout means that we can guarantee
            // how many messages will be dispatched, which seems to be vital in Reaper)
            for (int i = 20; --i >= 0;)
                MessageManager::getInstance()->runDispatchLoopUntil (1);
        }
       #endif
    }

    static void setNativeHostWindowSize (void* nsWindow, Component* component, int newWidth, int newHeight, bool isHIView)
    {
        JUCE_AUTORELEASEPOOL
        {
       #if JUCE_64BIT
        component->setSize (newWidth, newHeight);
       #else
        if (! isHIView)
        { //Treat NSView like 64bit:
            component->setSize (newWidth, newHeight);
        }
        else if (HIViewRef dummyView = (HIViewRef) (void*) (pointer_sized_int)
            component->getProperties() ["dummyViewRef"].toString().getHexValue64())
        {
            HIRect frameRect;
            HIViewGetFrame (dummyView, &frameRect);
            frameRect.size.width = newWidth;
            frameRect.size.height = newHeight;
            HIViewSetFrame (dummyView, &frameRect);
        }
       #endif
       }
    }
} // (juce namespace)


#endif
