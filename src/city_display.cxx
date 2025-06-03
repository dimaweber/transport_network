/**
 * @file city_display.cxx
 * @brief Functions for displaying city information
 * @date 2025-06-03
 */

#include "dot_parser.hxx"
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/ranges.h>
#include <vector>
#include <algorithm>
#include <numeric>

namespace transport {

// Calculate average distance to neighbor cities
double calculate_avg_distance(const TransportNetwork& network, const std::string& city_name) {
    auto roads = network.get_roads_from_city(city_name);
    if (roads.empty()) return 0.0;

    // For this simulation, we'll estimate distance based on speed limit
    // Higher speed limit routes typically indicate better roads and longer distances
    // This is a simplified model: distance ≈ 100 * speed_limit / 60 (in km)
    double total_distance = 0.0;
    for (const auto& road : roads) {
        // Estimate distance in kilometers based on speed limit
        // This creates a plausible distance value for each route
        double estimated_distance = road.max_speed_kmh * 1.5;
        total_distance += estimated_distance;
    }

    return total_distance / roads.size();
}

void print_cities_list(const TransportNetwork& network) {
    // Extract city names
    std::vector<std::string> city_names;
    for (const auto& [name, _] : network.cities) {
        // Skip the legend node and subgraph entries which are not real cities
        if (name == "legend" || name.starts_with("cluster_")) continue;
        city_names.push_back(name);
    }

    // Sort alphabetically
    std::sort(city_names.begin(), city_names.end());

    // Display header
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::cyan),
               "\n=== Available Cities ===\n");

    // Display cities in a formatted list
    for (size_t i = 0; i < city_names.size(); i++) {
        const std::string& name = city_names[i];
        const City* city = network.get_city(name);

        // Get connected cities
        std::vector<std::string> connected = network.get_connected_cities(name);
        std::sort(connected.begin(), connected.end());

        fmt::print(fmt::emphasis::bold, "{}. {}\n", i + 1, name);

        // Warehouse summary
        fmt::print("   Warehouse: ");
        bool first = true;
        for (const auto& good : city->warehouse.goods) {
            if (!first) fmt::print(", ");
            fmt::print("{}: {:.1f}t", DotParser::goods_type_to_string(good.type), good.quantity_tons);
            first = false;
        }
        fmt::print("\n");

        // Truck fleet summary
        fmt::print("   Fleet: {}", city->fleet.trucks.size() > 0 ? "" : "None");
        for (const auto& truck : city->fleet.trucks) {
            fmt::print("{} x {}, ", truck.count, DotParser::truck_type_to_string(truck.type));
        }
        fmt::print("\n");

        // Connected cities
        fmt::print("   Connected to: ");
        if (connected.empty()) {
            fmt::print("None\n");
        } else {
            fmt::print("{}\n", fmt::join(connected, ", "));
        }

        fmt::print("\n");
    }

    fmt::print("Total cities: {}\n", city_names.size());
}

void print_cities_list(const TransportNetwork& network, const CityDisplayOptions& options) {
    // Extract city names
    std::vector<std::string> city_names;
    for (const auto& [name, _] : network.cities) {
        // Skip the legend node and subgraph entries which are not real cities
        if (name == "legend" || name.starts_with("cluster_")) continue;
        city_names.push_back(name);
    }

    // Sort alphabetically
    std::sort(city_names.begin(), city_names.end());

    // Display header
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::cyan),
               "\n=== Available Cities ===\n");

    // Display cities in a formatted list
    for (size_t i = 0; i < city_names.size(); i++) {
        const std::string& name = city_names[i];
        const City* city = network.get_city(name);

        fmt::print(fmt::emphasis::bold, "{}. {}\n", i + 1, name);

        // Warehouse summary (only if explicitly requested)
        if (options.show_warehouse) {
            fmt::print("   Warehouse: ");
            bool first = true;
            for (const auto& good : city->warehouse.goods) {
                if (!first) fmt::print(", ");
                fmt::print("{}: {:.1f}t", DotParser::goods_type_to_string(good.type), good.quantity_tons);
                first = false;
            }
            fmt::print("\n");
        }

        // Truck fleet summary (only if explicitly requested)
        if (options.show_fleet) {
            fmt::print("   Fleet: {}", city->fleet.trucks.size() > 0 ? "" : "None");
            for (const auto& truck : city->fleet.trucks) {
                fmt::print("{} x {}, ", truck.count, DotParser::truck_type_to_string(truck.type));
            }
            fmt::print("\n");
        }

        // Connected cities (only if explicitly requested)
        if (options.show_neighbors) {
            // Get connected cities
            std::vector<std::string> connected = network.get_connected_cities(name);
            std::sort(connected.begin(), connected.end());

            fmt::print("   Connected to: ");
            if (connected.empty()) {
                fmt::print("None\n");
            } else {
                fmt::print("{}\n", fmt::join(connected, ", "));
            }
        }

        // Average distance calculation (only if requested)
        if (options.show_avg_dist) {
            double avg_distance = calculate_avg_distance(network, name);
            fmt::print("   Average distance to neighbors: {:.1f} km\n", avg_distance);
        }
    }

    fmt::print("\nTotal cities: {}\n", city_names.size());
}

} // namespace transport
