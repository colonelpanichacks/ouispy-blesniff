#pragma once

#include <Arduino.h>

namespace web_dashboard {

bool init();
void tick();
uint32_t connected_clients();

} // namespace web_dashboard
