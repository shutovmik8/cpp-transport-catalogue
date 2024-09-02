#pragma once

#include <iosfwd>
#include <string_view>

#include "geo.h"
#include "transport_catalogue.h"

namespace trancport_catalogue {

void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request, std::ostream& output);

}