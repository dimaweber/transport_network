/**
 * @file dot_parser.cxx
 * @brief Implementation of DOT file parser for transportation network simulation
 * @date 2025-06-03
 *
 * This file implements the DOT file parsing functionality for transportation
 * networks. It provides methods to parse Graphviz DOT files containing
 * city definitions with warehouses and truck fleets, as well as road
 * connections with capacity and speed constraints.
 *
 * The parser uses regular expressions and string processing to extract
 * structured data from DOT format files and build an in-memory representation
 * of the transportation network suitable for simulation and analysis.
 *
 * @note This code was generated with assistance from Claude 4 Sonnet AI
 */

#include "dot_parser.hxx"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <cctype>

namespace transport {

// Warehouse implementation
void Warehouse::add_goods(GoodsType type, double quantity) {
    for (auto& good : goods) {
        if (good.type == type) {
            good.quantity_tons += quantity;
            return;
        }
    }
    goods.emplace_back(type, quantity);
}

double Warehouse::get_goods_quantity(GoodsType type) const {
    for (const auto& good : goods) {
        if (good.type == type) {
            return good.quantity_tons;
        }
    }
    return 0.0;
}

void Warehouse::set_goods_quantity(GoodsType type, double quantity) {
    for (auto& good : goods) {
        if (good.type == type) {
            good.quantity_tons = quantity;
            return;
        }
    }
    goods.emplace_back(type, quantity);
}

// TruckFleet implementation
void TruckFleet::add_truck(TruckType type, int count, double capacity, double speed) {
    for (auto& truck : trucks) {
        if (truck.type == type) {
            truck.count += count;
            return;
        }
    }
    trucks.emplace_back(type, count, capacity, speed);
}

int TruckFleet::get_truck_count(TruckType type) const {
    for (const auto& truck : trucks) {
        if (truck.type == type) {
            return truck.count;
        }
    }
    return 0;
}

const Truck* TruckFleet::get_truck(TruckType type) const {
    for (const auto& truck : trucks) {
        if (truck.type == type) {
            return &truck;
        }
    }
    return nullptr;
}

// TransportNetwork implementation
void TransportNetwork::add_city(const std::string& name) {
    if (cities.find(name) == cities.end()) {
        cities[name] = std::make_unique<City>(name);
    }
}

City* TransportNetwork::get_city(const std::string& name) {
    auto it = cities.find(name);
    return (it != cities.end()) ? it->second.get() : nullptr;
}

const City* TransportNetwork::get_city(const std::string& name) const {
    auto it = cities.find(name);
    return (it != cities.end()) ? it->second.get() : nullptr;
}

void TransportNetwork::add_road(const Road& road) {
    roads.push_back(road);
}

std::vector<std::string> TransportNetwork::get_connected_cities(const std::string& city_name) const {
    std::vector<std::string> connected;
    for (const auto& road : roads) {
        if (road.from_city == city_name) {
            connected.push_back(road.to_city);
        } else if (road.to_city == city_name) {
            connected.push_back(road.from_city);
        }
    }
    return connected;
}

std::vector<Road> TransportNetwork::get_roads_from_city(const std::string& city_name) const {
    std::vector<Road> city_roads;
    for (const auto& road : roads) {
        if (road.from_city == city_name || road.to_city == city_name) {
            city_roads.push_back(road);
        }
    }
    return city_roads;

}

bool TransportNetwork::has_direct_connection(const std::string& from, const std::string& to) const {
    for (const auto& road : roads) {
        if ((road.from_city == from && road.to_city == to) ||
            (road.from_city == to && road.to_city == from)) {
            return true;
        }
    }
    return false;
}

// DotParser implementation
bool DotParser::parse_file(const std::string& filename) {
    clear();

    std::string content = load_file_content(filename);
    if (content.empty()) {
        set_error("Failed to read file: " + filename);
        return false;
    }

    return parse_string(content);
}

bool DotParser::parse_string(const std::string& dot_content) {
    clear();
    return parse_dot_content(dot_content);
}

void DotParser::clear() {
    network_ = TransportNetwork{};
    last_error_.clear();
}

bool DotParser::parse_dot_content(const std::string& content) {
    std::istringstream stream(content);
    std::string line;
    bool in_graph = false;

    while (std::getline(stream, line)) {
        line = trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '/' || line.substr(0, 2) == "//") {
            continue;
        }

        // Check for graph start
        if (line.find("digraph") != std::string::npos) {
            in_graph = true;
            continue;
        }

        // Skip graph settings and subgraphs
        if (line.find("rankdir") != std::string::npos ||
            line.find("node [") != std::string::npos ||
            line.find("edge [") != std::string::npos ||
            line.find("subgraph") != std::string::npos ||
            line.find("cluster_") != std::string::npos) {
            continue;
        }

        // Check for graph end
        if (line == "}" && in_graph) {
            break;
        }

        if (!in_graph) continue;

        // Parse node definitions (city descriptions)
        if (line.find("[label=") != std::string::npos && line.find("->") == std::string::npos) {
            if (!parse_node_definition(line)) {
                return false;
            }
        }
        // Parse edge definitions (roads)
        else if (line.find("->") != std::string::npos) {
            if (!parse_edge_definition(line)) {
                return false;
            }
        }
    }

    return validate_network();
}

bool DotParser::parse_node_definition(const std::string& line) {
    std::string city_name = extract_node_name(line);
    if (city_name.empty()) {
        set_error("Failed to extract city name from: " + line);
        return false;
    }

    std::string label = extract_label_content(line);
    if (label.empty()) {
        set_error("Failed to extract label from: " + line);
        return false;
    }

    network_.add_city(city_name);
    return parse_city_label(city_name, label);
}

bool DotParser::parse_edge_definition(const std::string& line) {
    std::string from, to;
    extract_edge_nodes(line, from, to);

    if (from.empty() || to.empty()) {
        set_error("Failed to extract edge nodes from: " + line);
        return false;
    }

    std::string label = extract_label_content(line);
    if (label.empty()) {
        set_error("Failed to extract road label from: " + line);
        return false;
    }

    // Extract color for road type determination
    std::string color = "blue"; // default
    std::regex color_regex(R"(color=(\w+))");
    std::smatch match;
    if (std::regex_search(line, match, color_regex)) {
        color = match[1].str();
    }

    Road road(from, to, "", 0.0, 0.0, RoadType::Standard);
    road.type = determine_road_type("", color);

    if (!parse_road_label(label, road)) {
        return false;
    }

    network_.add_road(road);
    return true;
}

bool DotParser::parse_city_label(const std::string& city_name, const std::string& label) {
    City* city = network_.get_city(city_name);
    if (!city) {
        set_error("City not found: " + city_name);
        return false;
    }

    // Split label into sections
    std::vector<std::string> sections = split(label, '\n');

    bool in_warehouse = false;
    bool in_trucks = false;
    std::string current_section;

    for (const auto& section : sections) {
        std::string trimmed = trim(section);

        if (trimmed == "Warehouse:") {
            in_warehouse = true;
            in_trucks = false;
            current_section.clear();
        } else if (trimmed == "Trucks:") {
            if (in_warehouse && !current_section.empty()) {
                parse_warehouse_section(current_section, city->warehouse);
            }
            in_warehouse = false;
            in_trucks = true;
            current_section.clear();
        } else if (in_warehouse || in_trucks) {
            if (!current_section.empty()) {
                current_section += "\n";
            }
            current_section += trimmed;
        }
    }

    // Parse the last section
    if (in_trucks && !current_section.empty()) {
        parse_truck_section(current_section, city->fleet);
    }

    return true;
}

bool DotParser::parse_warehouse_section(const std::string& section, Warehouse& warehouse) {
    std::vector<std::string> lines = split(section, '\n');

    for (const auto& line : lines) {
        std::string trimmed = trim(line);
        if (trimmed.empty()) continue;

        // Parse format: "Wood: 150t"
        double value;
        if (extract_numeric_value(trimmed, "Wood:", value)) {
            warehouse.set_goods_quantity(GoodsType::Wood, value);
        } else if (extract_numeric_value(trimmed, "Iron:", value)) {
            warehouse.set_goods_quantity(GoodsType::Iron, value);
        } else if (extract_numeric_value(trimmed, "Silk:", value)) {
            warehouse.set_goods_quantity(GoodsType::Silk, value);
        }
    }

    return true;
}

bool DotParser::parse_truck_section(const std::string& section, TruckFleet& fleet) {
    std::vector<std::string> lines = split(section, '\n');

    for (const auto& line : lines) {
        std::string trimmed = trim(line);
        if (trimmed.empty()) continue;

        // Parse format: "Ford: 2 units (max 10t, 80km/h)"
        std::regex truck_regex(R"((\w+):\s*(\d+)\s*units\s*\(max\s*([\d.]+)t,\s*([\d.]+)km/h\))");
        std::smatch match;

        if (std::regex_search(trimmed, match, truck_regex)) {
            std::string type_str = match[1].str();
            int count = std::stoi(match[2].str());
            double capacity = std::stod(match[3].str());
            double speed = std::stod(match[4].str());

            TruckType type = string_to_truck_type(type_str);
            fleet.add_truck(type, count, capacity, speed);
        }
    }

    return true;
}

bool DotParser::parse_road_label(const std::string& label, Road& road) {
    std::vector<std::string> lines = split(label, '\n');

    if (!lines.empty()) {
        road.name = trim(lines[0]);
    }

    for (const auto& line : lines) {
        std::string trimmed = trim(line);

        double value;
        if (extract_numeric_value(trimmed, "Max mass:", value)) {
            road.max_mass_tons = value;
        } else if (extract_numeric_value(trimmed, "Max speed:", value)) {
            road.max_speed_kmh = value;
        }
    }

    return true;
}

RoadType DotParser::determine_road_type(const std::string& road_name, const std::string& color) {
    if (color == "green") {
        return RoadType::Alternative;
    } else if (color == "red") {
        return RoadType::LongDistance;
    }
    return RoadType::Standard;
}

// Utility methods implementation
std::string DotParser::extract_node_name(const std::string& line) {
    std::regex node_regex(R"(^\s*(\w+)\s*\[)");
    std::smatch match;
    if (std::regex_search(line, match, node_regex)) {
        return match[1].str();
    }
    return "";
}

std::string DotParser::extract_label_content(const std::string& line) {
    std::regex label_regex(R"(label="(.*))");
    std::smatch match;
    if (std::regex_search(line, match, label_regex) && match.size() > 1) {
        std::string content = match[1].str();

        // Replace escape sequences
        std::regex newline_regex(R"(\\n)");
        content = std::regex_replace(content, newline_regex, "\n");
        return content;
    }
    return "";
}

std::string DotParser::extract_edge_nodes(const std::string& line, std::string& from, std::string& to) {
    std::regex edge_regex(R"(^\s*(\w+)\s*->\s*(\w+))");
    std::smatch match;
    if (std::regex_search(line, match, edge_regex)) {
        from = match[1].str();
        to = match[2].str();
        return from + " -> " + to;
    }
    from.clear();
    to.clear();
    return "";
}

std::string DotParser::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> DotParser::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

bool DotParser::extract_numeric_value(const std::string& text, const std::string& prefix, double& value) {
    size_t pos = text.find(prefix);
    if (pos == std::string::npos) return false;

    std::string suffix = text.substr(pos + prefix.length());
    std::regex num_regex(R"(\s*([\d.]+))");
    std::smatch match;

    if (std::regex_search(suffix, match, num_regex)) {
        value = std::stod(match[1].str());
        return true;
    }
    return false;
}

GoodsType DotParser::string_to_goods_type(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);

    if (lower_str == "wood") return GoodsType::Wood;
    if (lower_str == "iron") return GoodsType::Iron;
    if (lower_str == "silk") return GoodsType::Silk;

    return GoodsType::Wood; // default
}

TruckType DotParser::string_to_truck_type(const std::string& str) {
    std::string upper_str = str;
    std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(), ::toupper);

    if (upper_str == "FORD") return TruckType::Ford;
    if (upper_str == "MAN") return TruckType::Man;
    if (upper_str == "VW") return TruckType::Vw;

    return TruckType::Ford; // default
}

std::string DotParser::goods_type_to_string(GoodsType type) {
    switch (type) {
        case GoodsType::Wood: return "Wood";
        case GoodsType::Iron: return "Iron";
        case GoodsType::Silk: return "Silk";
        default: return "Unknown";
    }
}

std::string DotParser::truck_type_to_string(TruckType type) {
    switch (type) {
        case TruckType::Ford: return "Ford";
        case TruckType::Man: return "MAN";
        case TruckType::Vw: return "VW";
        default: return "Unknown";
    }
}

bool DotParser::validate_network() {
    if (network_.get_city_count() == 0) {
        set_error("No cities found in the network");
        return false;
    }

    if (network_.get_road_count() == 0) {
        set_error("No roads found in the network");
        return false;
    }

    return true;
}

void DotParser::set_error(const std::string& error_message) {
    last_error_ = error_message;
}

void DotParser::print_network_summary(std::ostream& os) const {
    os << "=== Transport Network Summary ===\n";
    os << "Cities: " << network_.get_city_count() << "\n";
    os << "Roads: " << network_.get_road_count() << "\n\n";

    for (const auto& [name, city] : network_.cities) {
        os << "City: " << name << "\n";
        os << "  Warehouse:\n";
        for (const auto& good : city->warehouse.goods) {
            os << "    " << DotParser::goods_type_to_string(good.type)
               << ": " << good.quantity_tons << "t\n";
        }
        os << "  Fleet:\n";
        for (const auto& truck : city->fleet.trucks) {
            os << "    " << DotParser::truck_type_to_string(truck.type)
               << ": " << truck.count << " units\n";
        }
        os << "\n";
    }

    os << "Roads:\n";
    for (const auto& road : network_.roads) {
        os << "  " << road.from_city << " -> " << road.to_city
           << " (" << road.name << ")\n";
        os << "    Max mass: " << road.max_mass_tons
           << "t, Max speed: " << road.max_speed_kmh << "km/h\n";
    }
}

// Utility functions
std::string load_file_content(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool save_network_to_dot(const TransportNetwork& network, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    file << "digraph transport_network {\n";
    file << "    rankdir=LR;\n";
    file << "    node [shape=box, style=filled, fillcolor=lightblue];\n\n";

    // Write cities
    for (const auto& [name, city] : network.cities) {
        file << "    " << name << " [label=\"" << name << "\\n";
        file << "Warehouse:\\n";
        for (const auto& good : city->warehouse.goods) {
                file << DotParser::goods_type_to_string(good.type)
                     << ": " << good.quantity_tons << "t\\n";
            }
            file << "\\nTrucks:\\n";
            for (const auto& truck : city->fleet.trucks) {
                file << DotParser::truck_type_to_string(truck.type)
                 << ": " << truck.count << " units\\n";
        }
        file << "\"];\n";
    }

    file << "\n";

    // Write roads
    for (const auto& road : network.roads) {
        std::string color = "blue";
        if (road.type == RoadType::Alternative) color = "green";
        else if (road.type == RoadType::LongDistance) color = "red";

        file << "    " << road.from_city << " -> " << road.to_city;
        file << " [label=\"" << road.name << "\\n";
        file << "Max mass: " << road.max_mass_tons << "t\\n";
        file << "Max speed: " << road.max_speed_kmh << "km/h\", color=" << color << "];\n";
    }

    file << "}\n";
    return true;
}

} // namespace transport
