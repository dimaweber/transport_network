/**
 * @file main.cxx
 * @brief Main program entry point for the transport network simulation
 * @date 2025-06-03
 */

#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>  // For fmt::join
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <CLI/CLI.hpp>
#include <cstdlib>
#include <filesystem>

#include "dot_parser.hxx"

// Using these functions from transport namespace
namespace transport
{
void print_cities_list(const TransportNetwork& network);
void print_cities_list(const TransportNetwork& network, const CityDisplayOptions& options);
}  // namespace transport

using namespace transport;

// Initialize logger
void setup_logging (const std::string& log_level)
{
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);

    if ( log_level == "debug" ) {
        spdlog::set_level(spdlog::level::debug);
    } else if ( log_level == "info" ) {
        spdlog::set_level(spdlog::level::info);
    } else if ( log_level == "warn" ) {
        spdlog::set_level(spdlog::level::warn);
    } else if ( log_level == "error" ) {
        spdlog::set_level(spdlog::level::err);
    } else {
        spdlog::set_level(spdlog::level::info);  // Default level
    }

    spdlog::info("Logging initialized at level: {}", log_level);
}

// Print network statistics with formatted output
void print_network_stats (const TransportNetwork& network)
{
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::green), "\n=== Transport Network Statistics ===\n");

    // Count only actual cities (exclude legend and subgraph entries)
    int actual_city_count = 0;
    for ( const auto& [name, _]: network.cities ) {
        if ( name != "legend" && !name.starts_with("cluster_") ) {
            actual_city_count++;
        }
    }

    fmt::print("Cities: {}\n", actual_city_count);
    fmt::print("Roads: {}\n\n", network.get_road_count( ));

    // Calculate total goods by type
    double total_wood = 0.0;
    double total_iron = 0.0;
    double total_silk = 0.0;

    for ( const auto& [name, city]: network.cities ) {
        total_wood += city->warehouse.get_goods_quantity(GoodsType::Wood);
        total_iron += city->warehouse.get_goods_quantity(GoodsType::Iron);
        total_silk += city->warehouse.get_goods_quantity(GoodsType::Silk);
    }

    fmt::print(fmt::emphasis::bold, "Total Goods:\n");
    fmt::print("  Wood: {:.1f}t\n", total_wood);
    fmt::print("  Iron: {:.1f}t\n", total_iron);
    fmt::print("  Silk: {:.1f}t\n\n", total_silk);

    // Count trucks by type
    int total_ford = 0;
    int total_man  = 0;
    int total_vw   = 0;

    for ( const auto& [name, city]: network.cities ) {
        total_ford += city->fleet.get_truck_count(TruckType::Ford);
        total_man += city->fleet.get_truck_count(TruckType::Man);
        total_vw += city->fleet.get_truck_count(TruckType::Vw);
    }

    fmt::print(fmt::emphasis::bold, "Total Trucks:\n");
    fmt::print("  Ford: {} units\n", total_ford);
    fmt::print("  MAN: {} units\n", total_man);
    fmt::print("  VW: {} units\n", total_vw);
}

int main (int argc, char** argv)
{
    // Command line parser setup
    CLI::App app {"Game of Thrones Transport Network Simulator"};

    // Command line options
    std::string input_file  = "networks/got.dot";
    std::string output_file = "";
    std::string log_level   = "info";
    bool        verbose     = false;
    bool        stats       = false;

    // City display options
    bool               show_cities_cmd = false;
    CityDisplayOptions city_options;

    // Define CLI options
    app.add_option("-i,--input", input_file, "Input DOT file path")->check(CLI::ExistingFile);
    app.add_option("-o,--output", output_file, "Output DOT file path");
    app.add_option("-l,--log-level", log_level, "Logging level (debug, info, warn, error)")->check(CLI::IsMember({"debug", "info", "warn", "error"}));
    app.add_flag("-v,--verbose", verbose, "Enable verbose output");
    app.add_flag("-s,--stats", stats, "Print network statistics");

    // Add 'show cities' command
    auto show_cmd   = app.add_subcommand("show", "Show various network information");
    auto cities_cmd = show_cmd->add_subcommand("cities", "Show all available cities");
    cities_cmd->add_flag("--warehouse", city_options.show_warehouse, "Display available goods in warehouses");
    cities_cmd->add_flag("--fleet", city_options.show_fleet, "Display available truck fleets");
    cities_cmd->add_flag("--neighbor", city_options.show_neighbors, "Display connected cities");
    cities_cmd->add_flag("--avg-dist", city_options.show_avg_dist, "Display average distance to neighboring cities");
    cities_cmd->callback([&] ( ) { show_cities_cmd = true; });

    // Add 'ship' command for cargo transportation
    struct ShipCommandOptions {
        std::string cargo_type;
        double cargo_mass = 0.0;
        std::string destination;
        std::string arrival_time;
        std::vector<std::string> from_cities;
        bool ship_command = false;
    };

    ShipCommandOptions ship_options;
    auto ship_cmd = app.add_subcommand("ship", "Ship cargo between cities");

    // Required parameters for ship command
    ship_cmd->add_option("cargo", ship_options.cargo_type, "Type of cargo to ship (wood, iron, silk)")
            ->required()
            ->check(CLI::IsMember({"wood", "iron", "silk"}, CLI::ignore_case));

    ship_cmd->add_option("mass", ship_options.cargo_mass, "Mass of cargo in tons")
            ->required()
            ->transform([](std::string mass) {
                // Handle both "10t" and "10" formats
                if (mass.back() == 't') {
                    mass.pop_back();
                }
                return mass;
            });

    ship_cmd->add_option("destination", ship_options.destination, "Destination city")
            ->required();

    ship_cmd->add_option("time", ship_options.arrival_time, "Required arrival time (format: HH:MM)")
            ->required()
            ->check(CLI::TypeValidator<std::string>{"HH:MM"});

    // Optional parameters
    ship_cmd->add_option("--from", ship_options.from_cities, "Comma-separated list of source cities")
            ->delimiter(',');

    ship_cmd->callback([&]() { ship_options.ship_command = true; });

    // Parse command line
    try {
        app.parse(argc, argv);
    } catch ( const CLI::ParseError& e ) {
        return app.exit(e);
    }

    // Setup logging
    setup_logging(log_level);

    // Parse the DOT file
    spdlog::info("Parsing DOT file: {}", input_file);
    DotParser parser;

    if ( !parser.parse_file(input_file) ) {
        spdlog::error("Failed to parse DOT file: {}", parser.get_last_error( ));
        return EXIT_FAILURE;
    }

    spdlog::info("Successfully parsed network with {} cities and {} roads", parser.get_network( ).get_city_count( ), parser.get_network( ).get_road_count( ));

    // Print detailed information if verbose
    if ( verbose ) {
        fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::blue), "\n=== Network Details ===\n");
        parser.print_network_summary( );
    }

    // Print network statistics if requested
    if ( stats ) {
        print_network_stats(parser.get_network( ));
    }

    // Show cities if requested
    if ( show_cities_cmd ) {
        print_cities_list(parser.get_network( ), city_options);
    }

    // Handle ship command if requested
    if ( ship_options.ship_command ) {
        const auto& network = parser.get_network();

        // Validate destination city exists
        if (!network.get_city(ship_options.destination)) {
            spdlog::error("Destination city '{}' not found in the network", ship_options.destination);
            return EXIT_FAILURE;
        }

        // Format cargo type to proper case
        std::string cargo_display;
        GoodsType cargo_type;
        if (ship_options.cargo_type == "wood" || ship_options.cargo_type == "WOOD") {
            cargo_display = "Wood";
            cargo_type = GoodsType::Wood;
        } else if (ship_options.cargo_type == "iron" || ship_options.cargo_type == "IRON") {
            cargo_display = "Iron";
            cargo_type = GoodsType::Iron;
        } else if (ship_options.cargo_type == "silk" || ship_options.cargo_type == "SILK") {
            cargo_display = "Silk";
            cargo_type = GoodsType::Silk;
        }

        // Display shipping request
        fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::blue), "\n=== Shipping Request ===\n");
        fmt::print("Cargo: {} {:.1f}t\n", cargo_display, ship_options.cargo_mass);
        fmt::print("Destination: {}\n", ship_options.destination);
        fmt::print("Arrival time: {}\n", ship_options.arrival_time);

        std::vector<std::string> source_cities;
        if (ship_options.from_cities.empty()) {
            // If no source cities specified, use all cities except destination
            for (const auto& [name, _] : network.cities) {
                if (name != ship_options.destination &&
                    name != "legend" &&
                    !name.starts_with("cluster_")) {
                    source_cities.push_back(name);
                }
            }
            fmt::print("Source cities: All available cities\n");
        } else {
            // Validate that all specified source cities exist
            for (const auto& city : ship_options.from_cities) {
                if (!network.get_city(city)) {
                    spdlog::error("Source city '{}' not found in the network", city);
                    return EXIT_FAILURE;
                }
                source_cities.push_back(city);
            }
            // Create a comma-separated list of cities manually
            std::string cities_list;
            for (size_t i = 0; i < source_cities.size(); ++i) {
                if (i > 0) cities_list += ", ";
                cities_list += source_cities[i];
            }
            fmt::print("Source cities: {}", cities_list);
        }

        fmt::print("\n\n");
        fmt::print(fmt::emphasis::bold, "Finding optimal shipping routes...\n");

        // TODO: Implement actual shipping logic to find optimal routes
        // For now, just provide a placeholder implementation
        fmt::print("Results would show the best possible routes to ship {:.1f}t of {} to {} by {}\n",
            ship_options.cargo_mass, cargo_display, ship_options.destination, ship_options.arrival_time);
    }

    // Save to output file if specified
    if ( !output_file.empty( ) ) {
        spdlog::info("Saving network to: {}", output_file);
        if ( save_network_to_dot(parser.get_network( ), output_file) ) {
            spdlog::info("Network saved successfully");
        } else {
            spdlog::error("Failed to save network to: {}", output_file);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
