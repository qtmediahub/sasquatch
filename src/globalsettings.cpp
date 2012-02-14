/****************************************************************************

This file is part of the QtMediaHub project on http://www.qtmediahub.com

Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).*
All rights reserved.

Contact:  Girish Ramakrishnan girish@forwardbias.in
Contact:  Nokia Corporation donald.carr@nokia.com
Contact:  Nokia Corporation johannes.zellner@nokia.com

You may use this file under the terms of the BSD license as follows:

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#include "globalsettings.h"

GlobalSettings *GlobalSettings::m_instance = 0;

GlobalSettings * GlobalSettings::instance()
{
    if (m_instance)
        new GlobalSettings();

    return m_instance;
}

GlobalSettings::GlobalSettings(QObject *parent) :
    Settings(parent)
{
    addOptionEntry(GlobalSettings::SkinsPath,           "",             "skinsPath",        "<path> adds path to skins search paths");
    addOptionEntry(GlobalSettings::SkinResolution,      "",             "skinResolution",   "<string> resolution name from skin manifest");
    addOptionEntry(GlobalSettings::Keymap,              "stdkeyboard",  "keymap",           "<name> specifies the keymap");
    addOptionEntry(GlobalSettings::KeymapsPath,         "",             "keymapsPath",      "<path> adds path to keymaps search paths");
    addOptionEntry(GlobalSettings::ApplicationsPath,    "",             "appsPath",         "<path> adds path to skins search paths");
    addOptionEntry(GlobalSettings::FullScreen,          "true",         "fullscreen",       "<bool> toggle fullscreen");
#ifdef QT5
    addOptionEntry(GlobalSettings::Skin,                "shroomfluence",   "skin",             "<name> specifies the skin");
    addOptionEntry(GlobalSettings::OverlayMode,         "true",        "overlayMode",      "<bool> toggle overlay mode used for devices with other mediaplayers than QtMultimediaKit");
#else
    addOptionEntry(GlobalSettings::Skin,                "confluence",   "skin",             "<name> specifies the skin");
    addOptionEntry(GlobalSettings::OverlayMode,         "false",        "overlayMode",      "<bool> toggle overlay mode used for devices with other mediaplayers than QtMultimediaKit");
#endif
    addOptionEntry(GlobalSettings::Headless,            "false",        "headless",         "<bool> toggle running with user interface, usable for streaming server usage");
    addOptionEntry(GlobalSettings::Proxy,               "false",        "proxy",            "<bool> use a proxy for network access");
    addOptionEntry(GlobalSettings::ProxyHost,           "localhost",    "proxyHost",        "<hostname> set proxy host, only used with -proxy=true");
    addOptionEntry(GlobalSettings::ProxyPort,           "8080",         "proxyPort",        "<port> set port number for proxy usage, only used with -proxy=true");
    addOptionEntry(GlobalSettings::MultiInstance,       "false",        "multiInstance",    "<bool> allow running multiple instances");
    addOptionEntry(GlobalSettings::Mouse,               "true",         "mouse",            "<bool> toggle mouse cursor");
    addOptionEntry(GlobalSettings::MouseTimeout,        "2",            "mouseTimeout",     "<int> hide idle mouse cursor timeout in seconds");
    addOptionEntry(GlobalSettings::DrivenFPS,           "false",        "drivenFPS",        "<bool> toggle fps counter");
    addOptionEntry(GlobalSettings::StreamingAddress,    "",             "streamingAddress", "<string> specifies the streaming address");
    addOptionEntry(GlobalSettings::StreamingPort,       "1337",         "streamingPort",    "<int> specifies the streaming port");
    addOptionEntry(GlobalSettings::Overscan,            "false",        "overscan",         "<bool> use overscan mode, use also overscanGeometry");
    addOptionEntry(GlobalSettings::OverscanGeometry,    "",             "overscanGeometry", "<int>x<int> set the overscan dimension");   // TODO test this!!!
    addOptionEntry(GlobalSettings::IdleTimeout,         "120",          "idleTimeout",      "<int> idle timeout in seconds");
    addOptionEntry(GlobalSettings::SystemTray,          "true",         "systemTray",       "<bool> toggle system tray icon");
    addOptionEntry(GlobalSettings::WindowGeometry,      "1080x720",     "windowGeometry",   "<int>x<int> set the window dimension");   // TODO test this!!!
    addOptionEntry(GlobalSettings::OpenGLUsage,         "true",         "opengl-usage",     "<bool> enable OpenGL if built with OpenGL");   // TODO test this!!!
    addOptionEntry(GlobalSettings::OpenGLFormatHack,    "false",         "opengl-format-hack","<bool> use special OpenGL format hacks");   // TODO test this!!!
    addOptionEntry(GlobalSettings::OpenGLViewport,      "true",         "opengl-viewport",  "<bool> use special OpenGL viewport");   // TODO test this!!!
    addOptionEntry(GlobalSettings::SmoothScaling,       "true",         "smooth-scaling",   "<bool> toggle smooth scaling");
    addOptionEntry(GlobalSettings::Avahi,               "true",         "avahi",            "<bool> toggle avahi usage");
    addOptionEntry(GlobalSettings::AvahiAdvertize,      "true",         "avahiAdvertize",   "<bool> toggle avahi advertizing");
    addOptionEntry(GlobalSettings::AvahiHideIPv6,       "true",         "avahiHideIPv6",    "<bool> hide IPv6 addresses");
    addOptionEntry(GlobalSettings::AvahiHideLocal,      "true",         "avahiHideLocal",   "<bool> hide local address");
    addOptionEntry(GlobalSettings::ResizeDelay,         "25",           "resizeDelay",      "<int> wait n milliseconds to do actual resize");
    addOptionEntry(GlobalSettings::TranslationsPath,    "",             "translationsPath", "<path> adds path to translations search paths");
    addOptionEntry(GlobalSettings::ResourcesPath,       "",             "resourcesPath",    "<path> adds path to resources search paths");
    addOptionEntry(GlobalSettings::AppsPath,            "",             "appsPath",         "<path> adds path to apps search paths");
    addOptionEntry(GlobalSettings::ImportsPath,         "",             "importsPath",      "<path> adds path to qml imports paths");
    addOptionEntry(GlobalSettings::ThumbnailPath,       "",             "thumbnailPath",    "<path> set path for thumbnail storage");
    addOptionEntry(GlobalSettings::ThumbnailSize,       "256",          "thumbnailSize",    "<int> edge size for thumbnails");
    addOptionEntry(GlobalSettings::MediaRefreshInterval,"1000",         "mediaRefreshInterval","<int> media model refresh interval");
    addOptionEntry(GlobalSettings::RemoteOverride,      "false",        "remoteOverride",   "<bool> force to run as remote control");
    addOptionEntry(GlobalSettings::ScanDelay,           "0",            "scanDelay",        "<int> delay scanner for n ms to keep user interface responsive");
    addOptionEntry(GlobalSettings::ExtraSNESExtensions, "",             "extraSNESExtensions","<stringlist> comma separated list of additional SNES file extensions");
    addOptionEntry(GlobalSettings::ExtraVideoExtensions,"",             "extraVideoExtensions","<stringlist> comma separated list of additional video file extensions");
    addOptionEntry(GlobalSettings::Installed,           "false",         "installed","<bool> assume installed to correct prefix");
    addOptionEntry(GlobalSettings::MemoryConscious,     "false",         "memoryConscious","<bool> constrain footprint");
    addOptionEntry(GlobalSettings::VSyncAnim,           "false",         "vsync","<bool> sync animations to refresh rate!");
    addOptionEntry(GlobalSettings::RedirectDebugOutput, "true",          "log","<bool> log debug messenging to file!");
    addOptionEntry(GlobalSettings::SwapLogPollInterval, "1000",          "swaplogInterval","<int> Interval at which to sample the swap log!");
    addOptionEntry(GlobalSettings::FPS,                 "false",         "fps","<bool> show fps counter!");

    m_instance = this;
}

bool GlobalSettings::isEnabled(GlobalSettings::Option option) const
{
    return Settings::isEnabled(m_table[option]);
}

QVariant GlobalSettings::value(GlobalSettings::Option option) const
{
    return Settings::value(m_table[option]);
}

const QString GlobalSettings::name(GlobalSettings::Option option) const
{
    return m_table[option];
}

const QString GlobalSettings::doc(GlobalSettings::Option option) const
{
    return Settings::doc(name(option));
}

void GlobalSettings::setValue(GlobalSettings::Option option, const QVariant &value)
{
    insert(m_table[option], value);
    emit valueChanged(m_table[option], value);
}

void GlobalSettings::addOptionEntry(GlobalSettings::Option option, const QVariant &value, const QString &name, const QString &doc)
{
    m_table[option] = name;
    Settings::addOptionEntry(name, value, doc);
}

