// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once
#include <nx/sdk/analytics/helpers/plugin.h>
#include <nx/sdk/analytics/i_engine.h>
#include <string>
#include <filesystem>
#include "engine.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aol_color_detection {

class Plugin: public nx::sdk::analytics::Plugin
{
protected:
    virtual nx::sdk::Result<nx::sdk::analytics::IEngine*> doObtainEngine() override;
    virtual std::string manifestString() const override;
};

} // namespace opencv_object_detection
} // namespace vms_server_plugins
} // namespace sample_company
} // namespace nx