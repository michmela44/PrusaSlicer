#include <catch2/catch.hpp>
//#include "test_data.hpp"
#include "Fill/Fill.hpp"
#include "Print.hpp"
#include "Geometry.hpp"
#include "Flow.hpp"
#include "MTUtils.hpp"
#include "ClipperUtils.hpp"

using namespace Slic3r;
using namespace Slic3r::Geometry;
using Pointf = Vec2d;

bool test_if_solid_surface_filled(const ExPolygon& expolygon, double flow_spacing, double angle = 0, double density = 1.0);

template<class ExPolyInp> ExPolygon expolygon(ExPolyInp &&inp) {
    ExPolygon ret;
    ret.contour.points = std::forward<ExPolyInp>(inp);
    return ret;
}

ExPolygon expolygon(std::initializer_list<Point> pts) {
    return expolygon(Points(pts));
}

TEST_CASE("Fill: adjusted solid distance") {
    Print print;
    int surface_width {250};

    int distance {Slic3r::Flow::solid_spacing(surface_width, 47)};

    REQUIRE(distance == Approx(50));
    REQUIRE(surface_width % distance == 0);
}

TEST_CASE("Fill: Pattern Path Length") {
    auto filler {Slic3r::Fill::new_from_type("rectilinear")};
    filler->angle = -(PI)/2.0;
    filler->spacing = 5;
    FillParams fillparams;
    fillparams.dont_adjust = true;
    filler->overlap = 0;
    fillparams.density = filler->spacing / 50.0;

    auto test {[filler, &fillparams] (const ExPolygon& poly) -> Polylines {
        auto surface {Slic3r::Surface(stTop, poly)};
        return filler->fill_surface(&surface, fillparams);
    }};
    
    auto scalepoint = [](const Vec2d &pf) { return scaled(pf); };

    SECTION("Square") {
        Points test_set;
        test_set.reserve(4);
        Pointfs points {Vec2d(0,0), Vec2d(100,0), Vec2d(100,100), Vec2d(0,100)};

        for (size_t i = 0; i < 4; ++i) {
            std::transform(points.cbegin() + i, points.cend(),
                           std::back_inserter(test_set), scalepoint);
            std::transform(points.cbegin(), points.cbegin() + i,
                           std::back_inserter(test_set), scalepoint);
            
            Polylines paths{test(expolygon(test_set))};
            REQUIRE(paths.size() == 1); // one continuous path

            // TODO: determine what the "Expected length" should be for rectilinear
            // fill of a 100x100 polygon. This check only checks that it's above
            // scale(3*100 + 2*50) + scaled_epsilon. ok abs($paths->[0]->length
            // - scale(3*100 + 2*50)) - scaled_epsilon, 'path has expected length';
            REQUIRE(
                std::abs(paths[0].length() - double{scale_(3 * 100 + 2 * 50)}) -
                    SCALED_EPSILON > 0
                ); // path has expected length

            test_set.clear();
        }
    }
    SECTION("Diamond with endpoints on grid") {
        Pointfs points{{0, 0}, {100, 0}, {150, 50}, {100, 100}, {0, 100}, {-50, 50}};

        Points test_set; test_set.reserve(6);
        std::transform(points.cbegin(), points.cend(),
                       std::back_inserter(test_set), scalepoint);
        
        Polylines paths {test(expolygon(test_set))};
        REQUIRE(paths.size() == 1); // one continuous path
    }

    SECTION("Square with hole") {
        Pointfs square { {0,0}, {100, 0}, {100, 100}, {0,100} };
        Pointfs hole { {25, 25}, {75, 25}, {75, 75}, {25, 75} };
        std::reverse(hole.begin(), hole.end());

        Points test_hole;
        Points test_square;

        std::transform(square.cbegin(), square.cend(), std::back_inserter(test_square), scalepoint); 
        std::transform(hole.cbegin(), hole.cend(), std::back_inserter(test_hole), scalepoint); 

        for (double angle : {-(PI/2.0), -(PI/4.0), -(PI), PI/2.0, PI}) {
            for (double spacing : {25.0, 5.0, 7.5, 8.5}) {
                fillparams.density = filler->spacing / spacing;
                filler->angle = angle;
                ExPolygon e = expolygon(test_square);
                e.holes.emplace_back(test_hole);
                Polylines paths {test(e)};
                REQUIRE((paths.size() >= 2 && paths.size() <= 3));
                // paths don't cross hole
                REQUIRE(diff_pl(paths, offset(e, +SCALED_EPSILON*10)).size() == 0);
            }
        }
    }
//    SECTION("Regression: Missing infill segments in some rare circumstances") {
//        filler->angle = (PI/4.0);
//        fillparams.dont_adjust = false;
//        filler->spacing = 0.654498;
//        filler->overlap = unscaled(359974);
//        fillparams.density = 1;
//        filler->layer_id = 66;
//        filler->z = 20.15;

//        Points points{Point(25771516, 14142125), Point(14142138, 25771515),
//                      Point(2512749, 14142131), Point(14142125, 2512749)};

//        Polylines paths {test(expolygon(points))};
//        REQUIRE(paths.size() == 1); // one continuous path

//        // TODO: determine what the "Expected length" should be for rectilinear fill of a 100x100 polygon. 
//        // This check only checks that it's above scale(3*100 + 2*50) + scaled_epsilon.
//        // ok abs($paths->[0]->length - scale(3*100 + 2*50)) - scaled_epsilon, 'path has expected length';
//        REQUIRE(std::abs(paths[0].length() - double{scale_(3*100 + 2*50)}) - SCALED_EPSILON > 0); // path has expected length
//    }

//    SECTION("Rotated Square") {
//        ExPolygon square = expolygon(
//            {Point::new_scale(0, 0), Point::new_scale(50, 0),
//             Point::new_scale(50, 50), Point::new_scale(0, 50)});

//        auto filler {Slic3r::Fill::new_from_type("rectilinear")};
//        filler->bounding_box = square.contour.bounding_box();
//        filler->angle = 0;
        
//        auto surface {Surface(stTop, square)};
//        auto flow {Slic3r::Flow(0.69, 0.4, 0.50)};

//        filler->spacing = flow.spacing();
//        fillparams.density = 1.0;

//        for (auto angle : { 0.0, 45.0}) {
//            surface.expolygon.rotate(angle, Point(0,0));
//            auto paths {filler->fill_surface(&surface, fillparams)};
//            REQUIRE(paths.size() == 1);
//        }
//    }
//    SECTION("Solid surface fill") {
//        ExPolygon poly = expolygon({
//            Point::new_scale(6883102, 9598327.01296997),
//            Point::new_scale(6883102, 20327272.01297),
//            Point::new_scale(3116896, 20327272.01297),
//            Point::new_scale(3116896, 9598327.01296997)
//        });
         
//        REQUIRE(test_if_solid_surface_filled(poly, 0.55) == true);
//        for (size_t i = 0; i <= 20; ++i)
//        {
//            poly.scale(1.05);
//            REQUIRE(test_if_solid_surface_filled(poly, 0.55) == true);
//        }
//    }
//    SECTION("Solid surface fill") {
//        ExPolygon poly = expolygon(
//            {{59515297, 5422499},  {59531249, 5578697},  {59695801, 6123186},
//             {59965713, 6630228},  {60328214, 7070685},  {60773285, 7434379},
//             {61274561, 7702115},  {61819378, 7866770},  {62390306, 7924789},
//             {62958700, 7866744},  {63503012, 7702244},  {64007365, 7434357},
//             {64449960, 7070398},  {64809327, 6634999},  {65082143, 6123325},
//             {65245005, 5584454},  {65266967, 5422499},  {66267307, 5422499},
//             {66269190, 8310081},  {66275379, 17810072}, {66277259, 20697500},
//             {65267237, 20697500}, {65245004, 20533538}, {65082082, 19994444},
//             {64811462, 19488579}, {64450624, 19048208}, {64012101, 18686514},
//             {63503122, 18415781}, {62959151, 18251378}, {62453416, 18198442},
//             {62390147, 18197355}, {62200087, 18200576}, {61813519, 18252990},
//             {61274433, 18415918}, {60768598, 18686517}, {60327567, 19047892},
//             {59963609, 19493297}, {59695865, 19994587}, {59531222, 20539379},
//             {59515153, 20697500}, {58502480, 20697500}, {58502480, 5422499}});

//        REQUIRE(test_if_solid_surface_filled(poly, 0.55) == true);
//        REQUIRE(test_if_solid_surface_filled(poly, 0.55, PI/2.0) == true);
//    }
//    SECTION("Solid surface fill") {
//        ExPolygon expoly = expolygon(
//            {Point::new_scale(0, 0), Point::new_scale(98, 0),
//             Point::new_scale(98, 10), Point::new_scale(0, 10)});

//        REQUIRE(test_if_solid_surface_filled(expoly, 0.5, 45.0, 0.99) == true);
//    }
    
    delete filler;

}

bool test_if_solid_surface_filled(const ExPolygon &expolygon,
                                  double           flow_spacing,
                                  double           angle,
                                  double           density)
{
    auto* filler {Slic3r::Fill::new_from_type("rectilinear")};
    FillParams fillparams;
    filler->bounding_box = expolygon.contour.bounding_box();
    filler->angle = angle;
    fillparams.dont_adjust = false;
    
    Surface surface(stBottom, expolygon);
    Flow flow(flow_spacing, 0.4, flow_spacing);
    
    filler->spacing = flow.spacing();
    
    Polylines paths {filler->fill_surface(&surface, fillparams)};
    
    // check whether any part was left uncovered
    Polygons grown_paths;
    grown_paths.reserve(paths.size());
    
    // figure out what is actually going on here re: data types
    std::for_each(paths.begin(), paths.end(),
                  [filler, &grown_paths](const Polyline &p) {
                      polygons_append(grown_paths,
                                      offset(p, scale_(filler->spacing / 2.0)));
                  });

    ExPolygons uncovered = diff_ex(expolygon, grown_paths, true);
    
    // ignore very small dots
    const auto scaled_flow_spacing { std::pow(scale_(flow_spacing), 2) };

    auto iter{std::remove_if(uncovered.begin(), uncovered.end(),
                             [scaled_flow_spacing](const ExPolygon &poly) {
                                 return poly.area() > scaled_flow_spacing;
                             })};
    
    uncovered.erase(iter, uncovered.end());
    
    return uncovered.size() == 0; // solid surface is fully filled
}
