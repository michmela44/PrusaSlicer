#ifndef SVGTOOLS_HPP
#define SVGTOOLS_HPP

#include <iostream>
#include <fstream>
#include <string>

#include <libnest2d.h>
#include <libnest2d/geometries_io.hpp>

namespace libnest2d { namespace svg {

class SVGWriter {
public:

    enum OrigoLocation {
        TOPLEFT,
        BOTTOMLEFT
    };

    struct Config {
        OrigoLocation origo_location;
        Coord mm_in_coord_units;
        double width, height;
        Config():
            origo_location(BOTTOMLEFT), mm_in_coord_units(1000000),
            width(500), height(500) {}

    };

private:
    Config conf_;
    std::vector<std::string> svg_layers_;
    bool finished_ = false;
public:

    SVGWriter(const Config& conf = Config()):
        conf_(conf) {}

    void setSize(const Box& box) {
        conf_.height = static_cast<double>(box.height()) /
                conf_.mm_in_coord_units;
        conf_.width = static_cast<double>(box.width()) /
                conf_.mm_in_coord_units;
    }

    void writeItem(const Item& item) {
        if(svg_layers_.empty()) addLayer();
        Item tsh(item.transformedShape());
        if(conf_.origo_location == BOTTOMLEFT)
        for(unsigned i = 0; i < tsh.vertexCount(); i++) {
            auto v = tsh.vertex(i);
            setY(v, -getY(v) + conf_.height*conf_.mm_in_coord_units);
            tsh.setVertex(i, v);
        }
        currentLayer() += ShapeLike::serialize<Formats::SVG>(tsh.rawShape(),
                                            1.0/conf_.mm_in_coord_units) + "\n";
    }

    void writePackGroup(const PackGroup& result) {
        for(auto r : result) {
            addLayer();
            for(Item& sh : r) {
                writeItem(sh);
            }
            finishLayer();
        }
    }

    void addLayer() {
        svg_layers_.emplace_back(header());
        finished_ = false;
    }

    void finishLayer() {
        currentLayer() += "\n</svg>\n";
        finished_ = true;
    }

    void save(const std::string& filepath) {
        unsigned lyrc = svg_layers_.size() > 1? 1 : 0;
        unsigned last = svg_layers_.size() > 1? svg_layers_.size() : 0;

        for(auto& lyr : svg_layers_) {
            std::fstream out(filepath + (lyrc > 0? std::to_string(lyrc) : "") +
                             ".svg", std::fstream::out);
            if(out.is_open()) out << lyr;
            if(lyrc == last && !finished_) out << "\n</svg>\n";
            out.flush(); out.close(); lyrc++;
        };
    }

private:

    std::string& currentLayer() { return svg_layers_.back(); }

    const std::string header() const {
        std::string svg_header =
R"raw(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.0//EN" "http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd">
<svg height=")raw";
        svg_header += std::to_string(conf_.height) + "\" width=\"" + std::to_string(conf_.width) + "\" ";
        svg_header += R"raw(xmlns="http://www.w3.org/2000/svg" xmlns:svg="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">)raw";
        return svg_header;
    }

};

}
}

#endif // SVGTOOLS_HPP
