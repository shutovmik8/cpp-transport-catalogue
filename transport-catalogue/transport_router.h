#include "graph.h"
#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <optional>
#include <string_view>
#include <variant>

namespace graph {

struct BusRiding {
	std::string_view name;
	int span_count;
	double time;
};

struct Waiting {
	std::string_view name;
	double time;
};

struct RouteInfo {
	double total_time;
	std::vector<std::variant<BusRiding, Waiting>> route_units;
};

class RoutesManager {
	DirectedWeightedGraph<double> graph;
	Router<double> router;

	DirectedWeightedGraph<double> MakeRoutesGraph(const transport_catalogue::TransportCatalogue& catalogue);

public:
	RoutesManager(const transport_catalogue::TransportCatalogue& catalogue) : graph(MakeRoutesGraph(catalogue)), router(graph) {}

	std::optional<RouteInfo> GetRoute(std::string_view from, std::string_view to) const;
};

}