#include "graph.h"
#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <optional>
#include <string_view>

namespace graph {

struct RouteUnit {
	std::string_view name;
	int span_count;
	double time;
	bool type; //false - wait, true - bus
};

struct RouteInfo {
	double total_time;
	std::vector<RouteUnit> route_units;
};

class RoutesManager {
	DirectedWeightedGraph<double> graph;
	Router<double> router;

	DirectedWeightedGraph<double> MakeRoutesGraph(const transport_catalogue::TransportCatalogue& catalogue);

public:
	RoutesManager(const transport_catalogue::TransportCatalogue& catalogue) : graph(MakeRoutesGraph(catalogue)), router(graph) {}

	std::optional<RouteInfo> GetRoute(std::string_view from, std::string_view to);
};

}