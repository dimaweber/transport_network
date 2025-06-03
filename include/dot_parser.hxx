/**
 * @file dot_parser.hxx
 * @brief DOT file parser for transportation network simulation
 * @date 2025-06-03
 *
 * This file contains the header definitions for parsing DOT format files
 * that describe transportation networks with cities, warehouses, truck fleets,
 * and road connections. The parser extracts structured data from Graphviz
 * DOT files for use in logistics and transportation simulations.
 *
 * @note This code was generated with assistance from Claude 4 Sonnet AI
 */

#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace transport
{

// Forward declarations
struct Warehouse;
struct TruckFleet;
struct Road;
struct City;
struct TransportNetwork;

// Enums for better type safety
enum class GoodsType { Wood, Iron, Silk };

enum class TruckType { Ford, Man, Vw };

enum class RoadType { Standard, Alternative, LongDistance };

// Data structures
struct Goods {
    GoodsType type;
    double    quantity_tons;

    Goods (GoodsType t, double qty) : type(t), quantity_tons(qty) { }
};

struct Truck {
    TruckType type;
    int       count;
    double    max_capacity_tons;
    double    max_speed_kmh;

    Truck (TruckType t, int c, double cap, double speed) : type(t), count(c), max_capacity_tons(cap), max_speed_kmh(speed) { }
};

struct Warehouse {
    std::vector<Goods> goods;

    void   add_goods(GoodsType type, double quantity);
    double get_goods_quantity(GoodsType type) const;
    void   set_goods_quantity(GoodsType type, double quantity);
};

struct TruckFleet {
    std::vector<Truck> trucks;

    void         add_truck(TruckType type, int count, double capacity, double speed);
    int          get_truck_count(TruckType type) const;
    const Truck* get_truck(TruckType type) const;
};

struct Road {
    std::string from_city;
    std::string to_city;
    std::string name;
    double      max_mass_tons;
    double      max_speed_kmh;
    RoadType    type;

    Road (const std::string& from, const std::string& to, const std::string& road_name, double mass, double speed, RoadType road_type) :
        from_city(from),
        to_city(to),
        name(road_name),
        max_mass_tons(mass),
        max_speed_kmh(speed),
        type(road_type)
    {
    }
};

struct City {
    std::string name;
    Warehouse   warehouse;
    TruckFleet  fleet;

    explicit City (const std::string& city_name) : name(city_name) { }
};

struct TransportNetwork {
    std::unordered_map<std::string, std::unique_ptr<City>> cities;
    std::vector<Road>                                      roads;

    void        add_city(const std::string& name);
    City*       get_city(const std::string& name);
    const City* get_city(const std::string& name) const;
    void        add_road(const Road& road);

    // Network analysis methods
    std::vector<std::string> get_connected_cities(const std::string& city_name) const;
    std::vector<Road>        get_roads_from_city(const std::string& city_name) const;
    bool                     has_direct_connection(const std::string& from, const std::string& to) const;

    // Statistics
    size_t get_city_count ( ) const { return cities.size( ); }

    size_t get_road_count ( ) const { return roads.size( ); }
};

// Parser class
class DotParser
{
public:
    DotParser( )  = default;
    ~DotParser( ) = default;

    // Main parsing methods
    bool parse_file(const std::string& filename);
    bool parse_string(const std::string& dot_content);

    // Access parsed data
    const TransportNetwork& get_network ( ) const { return network_; }

    TransportNetwork& get_network ( ) { return network_; }

    // Error handling
    const std::string& get_last_error ( ) const { return last_error_; }

    bool has_error ( ) const { return !last_error_.empty( ); }

    // Utility methods
    void clear( );
    void print_network_summary(std::ostream& os = std::cout) const;

    // Type conversion utilities - made public for use in free functions
    static std::string goods_type_to_string(GoodsType type);
    static std::string truck_type_to_string(TruckType type);

private:
    TransportNetwork network_;
    std::string      last_error_;

    // Internal parsing methods
    bool parse_dot_content(const std::string& content);
    bool parse_node_definition(const std::string& line);
    bool parse_edge_definition(const std::string& line);

    // Node parsing helpers
    bool parse_city_label(const std::string& city_name, const std::string& label);
    bool parse_warehouse_section(const std::string& section, Warehouse& warehouse);
    bool parse_truck_section(const std::string& section, TruckFleet& fleet);

    // Edge parsing helpers
    bool     parse_road_label(const std::string& label, Road& road);
    RoadType determine_road_type(const std::string& road_name, const std::string& color);

    // Utility parsing methods
    std::string extract_node_name(const std::string& line);
    std::string extract_label_content(const std::string& line);
    std::string extract_edge_nodes(const std::string& line, std::string& from, std::string& to);

    // String processing utilities
    std::string              trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string              remove_quotes(const std::string& str);
    bool                     extract_numeric_value(const std::string& text, const std::string& prefix, double& value);

    // Type conversion utilities
    GoodsType string_to_goods_type(const std::string& str);
    TruckType string_to_truck_type(const std::string& str);

    // Validation methods
    bool validate_network( );
    bool validate_city(const City& city);
    bool validate_road(const Road& road);

    // Error reporting
    void set_error(const std::string& error_message);
    void append_error(const std::string& additional_info);
};

// City display options struct for controlling city information display
struct CityDisplayOptions {
    bool show_warehouse = false;
    bool show_fleet     = false;
    bool show_neighbors = false;
    bool show_avg_dist  = false;
};

// Utility functions
std::string load_file_content(const std::string& filename);
bool        save_network_to_dot(const TransportNetwork& network, const std::string& filename);

}  // namespace transport
