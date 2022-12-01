// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
#ifndef FRAMEWORK_DEBUGGUI_H
#define FRAMEWORK_DEBUGGUI_H

#include <functional>
#include <iosfwd>

namespace o2::framework {

/// Default implementation of the error callback
void default_error_callback(int, const char*);

void* initGUI(const char* name, decltype(default_error_callback) = nullptr);
bool pollGUI(void *context, std::function<void(void)> guiCallback);
void getFrameRaw(void *data, void **raw_data, int *size);
bool pollGUIPreRender(void* context, float delta);
void* pollGUIRender(std::function<void(void)> guiCallback);
void pollGUIPostRender(void* context, void* draw_data);
void disposeGUI();

} // namespace o2::framework

#endif // FRAMEWORK_DEBUGGUI_H
