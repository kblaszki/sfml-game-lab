#include <syndicate/app/Theme.hpp>

namespace sgl::syndicate
{

Theme defaultTheme()
{
    Theme t{};
    t.sector = {
        sgl::Color{80, 180, 255},  // AI — cyan-blue
        sgl::Color{90, 210, 120},  // CleanEnergy — green
        sgl::Color{200, 120, 230}, // Biotech — violet
        sgl::Color{255, 110, 90},  // CyberSec — coral
        sgl::Color{255, 190, 70},  // Logistics — amber
        sgl::Color{100, 220, 200}, // Fintech — teal
    };
    t.edge = sgl::Color{70, 80, 100};
    t.payday = sgl::Color{240, 210, 80};
    t.exchange = sgl::Color{180, 160, 255};
    t.hub = sgl::Color{200, 200, 210};
    t.audit = sgl::Color{255, 100, 140};
    t.grant = sgl::Color{120, 230, 180};
    t.highlight = sgl::Color{255, 255, 120};
    t.pawn[0] = sgl::Color{255, 90, 90};
    t.pawn[1] = sgl::Color{90, 160, 255};
    t.pawn[2] = sgl::Color{255, 200, 60};
    t.pawn[3] = sgl::Color{180, 255, 120};
    return t;
}

} // namespace sgl::syndicate
