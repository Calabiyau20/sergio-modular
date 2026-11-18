// SERGIO Modular — Calabi Yau Cosmic Neon visual language (colors, fonts,
// primitive painters): deep space panels, glowing cyan frames, neon magenta
// rosette knobs, luminous color-coded banana jacks, amber target LEDs.
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../engine/EngineCore.h"

namespace pf50ui {

constexpr float PX = 52.0f;              // pixels per panel inch
constexpr float MOD_H = 7.0f;            // module height, inches

namespace col {
    const juce::Colour cream       (0xff111322); // cosmic deep-navy module face
    const juce::Colour creamShade  (0xff191d36); // subtle bottom bevel
    const juce::Colour ink         (0xffd7f9ff); // crisp luminous cyan-white ink
    const juce::Colour frame       (0xff2ce6ff); // neon cyan module border
    const juce::Colour boxLine     (0xff1aa8c4); // neon cyan section box lines
    const juce::Colour wood        (0xff382246); // dark cosmic plum side rail
    const juce::Colour woodDark    (0xff1e1128); // darker rail edge
    const juce::Colour headerBg    (0xff080912); // deep space background
    const juce::Colour ioBg        (0xff0d0f1c); // dark I/O strip face
    const juce::Colour ioInk       (0xff7deeff); // cyan I/O text
    const juce::Colour jackWhite   (0xffd8fbff); // electric ice-cyan jack
    const juce::Colour jackRed     (0xffff2ea6); // neon magenta logic jack
    const juce::Colour jackBlack   (0xff1b1f36); // dark cosmic audio jack
    const juce::Colour jackPurple  (0xffb967ff); // neon violet sync jack
    const juce::Colour jackGrey    (0xff42e3c8); // neon mint-teal bi-di jack
    const juce::Colour jackHole    (0xff080a14); // deep socket center
    const juce::Colour led         (0xffffbe3b); // warm solar gold LED
    const juce::Colour ledOff      (0xff4a3a1e);
    const juce::Colour knob        (0xff12101f); // obsidian knob core
    const juce::Colour knobHi      (0xffff2ea6); // neon magenta knob rim/ticks
    const juce::Colour cable       (0xff2ce6ff);
    const juce::Colour neonPink    (0xffff2ea6);
    const juce::Colour neonCyan    (0xff2ce6ff);
    const juce::Colour neonGold    (0xffffbe3b);
}

inline juce::Font panelFont (float h, bool bold = false)
{
    auto f = juce::Font (juce::FontOptions ("Futura", h, bold ? juce::Font::bold : juce::Font::plain));
    return f;
}

inline juce::Colour jackColour (pf50::JackColor c)
{
    using JC = pf50::JackColor;
    switch (c)
    {
        case JC::White:  return col::jackWhite;
        case JC::Red:    return col::jackRed;
        case JC::Black:  return col::jackBlack;
        case JC::Purple: return col::jackPurple;
        case JC::Grey:   return col::jackGrey;
        case JC::TRS:    return col::jackBlack;
        default:         return col::jackWhite;
    }
}

// 8-lobed cosmic rosette knob with neon magenta tick ring & bright pointer
inline void drawKnob (juce::Graphics& g, juce::Rectangle<float> r, float value01, bool /*big*/)
{
    auto c = r.getCentre();
    float R = r.getWidth() * 0.5f;

    // Subtle neon magenta tick dots around 270 deg sweep
    for (int t = 0; t <= 10; ++t)
    {
        float ta = juce::degreesToRadians (-135.0f + 27.0f * (float) t);
        auto dotPt = c.getPointOnCircumference (R * 1.04f, ta);
        bool lit = (float) t / 10.0f <= value01 + 0.02f;
        g.setColour (lit ? col::neonPink.withAlpha (0.9f) : col::neonPink.withAlpha (0.25f));
        g.fillEllipse (juce::Rectangle<float> (2.2f, 2.2f).withCentre (dotPt));
    }

    juce::Path p;
    const int lobes = 8;
    for (int i = 0; i <= 96; ++i)
    {
        float a = (float) i / 96.0f * juce::MathConstants<float>::twoPi;
        float rad = R * 0.88f * (0.86f + 0.14f * std::cos (a * lobes));
        auto pt = c.getPointOnCircumference (rad, a);
        if (i == 0) p.startNewSubPath (pt); else p.lineTo (pt);
    }
    p.closeSubPath();
    g.setColour (col::knob);
    g.fillPath (p);
    g.setColour (col::neonPink.withAlpha (0.82f));
    g.strokePath (p, juce::PathStrokeType (1.25f));
    // inner cyan ring
    g.setColour (col::neonCyan.withAlpha (0.35f));
    g.drawEllipse (juce::Rectangle<float> (R * 1.05f, R * 1.05f).withCentre (c), 0.9f);
    // pointer: 270 deg sweep
    float ang = juce::degreesToRadians (-135.0f + 270.0f * value01);
    auto tip  = c.getPointOnCircumference (R * 0.78f, ang);
    auto tail = c.getPointOnCircumference (R * 0.15f, ang);
    g.setColour (col::neonCyan);
    g.drawLine ({ tail, tip }, juce::jmax (1.8f, R * 0.14f));
}

inline void drawJack (juce::Graphics& g, juce::Rectangle<float> r, pf50::JackColor jc, bool highlight)
{
    auto c = r.getCentre();
    float R = r.getWidth() * 0.5f;
    auto body = jackColour (jc);
    auto rim = (jc == pf50::JackColor::Black) ? col::neonCyan : body;

    // subtle outer glow
    g.setColour (rim.withAlpha (highlight ? 0.45f : 0.18f));
    g.fillEllipse (r.expanded (2.2f));

    g.setColour (body);
    g.fillEllipse (r);
    g.setColour (rim.brighter (0.25f));
    g.drawEllipse (r, juce::jmax (1.2f, R * 0.12f));
    g.setColour (col::jackHole);
    g.fillEllipse (juce::Rectangle<float> (R * 0.85f, R * 0.85f).withCentre (c));
    g.setColour (juce::Colours::black.withAlpha (0.85f));
    g.fillEllipse (juce::Rectangle<float> (R * 0.45f, R * 0.45f).withCentre (c));
    if (highlight)
    {
        g.setColour (juce::Colours::white.withAlpha (0.95f));
        g.drawEllipse (r.expanded (2.5f), 2.0f);
    }
}

inline void drawLed (juce::Graphics& g, juce::Rectangle<float> r, float bright)
{
    auto c = r.getCentre();
    float R = r.getWidth() * 0.5f;
    if (bright > 0.02f)
    {
        g.setColour (col::led.withAlpha (0.45f * bright));
        g.fillEllipse (juce::Rectangle<float> (R * 2.8f, R * 2.8f).withCentre (c));
    }
    auto ring = col::led.withAlpha (0.4f + 0.6f * bright);
    for (float f : { 1.0f, 0.68f, 0.38f })
    {
        g.setColour (ring);
        g.drawEllipse (juce::Rectangle<float> (2 * R * f, 2 * R * f).withCentre (c), juce::jmax (1.1f, R * 0.17f));
    }
    g.setColour (bright > 0.35f ? col::led.brighter (0.4f) : col::led.withAlpha (0.75f));
    g.fillEllipse (juce::Rectangle<float> (R * 0.44f, R * 0.44f).withCentre (c));
}

inline void drawPushButton (juce::Graphics& g, juce::Rectangle<float> r, bool down)
{
    auto c = r.getCentre();
    float R = r.getWidth() * 0.5f;
    g.setColour (col::neonCyan.withAlpha (0.45f));
    g.fillEllipse (r);
    g.setColour (col::neonCyan);
    g.drawEllipse (r, 1.2f);
    auto btn = juce::Rectangle<float> (R * 1.34f, R * 1.34f).withCentre (c);
    g.setColour (down ? col::neonPink.darker (0.35f) : col::neonPink);
    g.fillEllipse (btn);
    g.setColour (juce::Colours::white.withAlpha (down ? 0.15f : 0.35f));
    g.fillEllipse (btn.reduced (R * 0.28f).translated (-R * 0.1f, -R * 0.12f));
}

inline void drawToggle (juce::Graphics& g, juce::Rectangle<float> r, int pos, int nPos)
{
    auto c0 = r.getCentre();
    auto base = juce::Rectangle<float> (r.getWidth() * 0.52f, r.getHeight() * 0.3f).withCentre (c0);
    g.setColour (juce::Colour (0xff242a47));
    g.fillEllipse (base);
    g.setColour (col::neonCyan.withAlpha (0.85f));
    g.drawEllipse (base, 1.1f);
    float t = nPos <= 1 ? 0.5f : (float) pos / (float) (nPos - 1);
    float y = juce::jmap (t, r.getY() + r.getHeight() * 0.06f, r.getBottom() - r.getHeight() * 0.06f);
    auto tip = juce::Point<float> (r.getCentreX(), y);
    g.setColour (col::neonCyan);
    g.drawLine (c0.x, c0.y, tip.x, tip.y, r.getWidth() * 0.2f);
    g.setColour (col::neonPink);
    g.fillEllipse (juce::Rectangle<float> (r.getWidth() * 0.5f, r.getWidth() * 0.5f).withCentre (tip));
    g.setColour (juce::Colours::white.withAlpha (0.65f));
    g.drawEllipse (juce::Rectangle<float> (r.getWidth() * 0.5f, r.getWidth() * 0.5f).withCentre (tip), 1.0f);
}

// small waveform glyphs used beside jacks in the drawings
enum class Glyph { Saw, Sine, PulseUp, PulseDown, RampUp, RampDown, ArcAR };
inline void drawGlyph (juce::Graphics& g, juce::Rectangle<float> r, Glyph gl)
{
    juce::Path p;
    auto b = r.reduced (1.0f);
    switch (gl)
    {
        case Glyph::Saw:
            p.startNewSubPath (b.getX(), b.getBottom());
            p.lineTo (b.getX() + b.getWidth() * 0.5f, b.getY());
            p.lineTo (b.getX() + b.getWidth() * 0.5f, b.getBottom());
            p.lineTo (b.getRight(), b.getY());
            break;
        case Glyph::Sine:
        {
            for (int i = 0; i <= 24; ++i)
            {
                float x = b.getX() + b.getWidth() * i / 24.0f;
                float y = b.getCentreY() - std::sin (i / 24.0f * juce::MathConstants<float>::twoPi) * b.getHeight() * 0.42f;
                if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
            }
            break;
        }
        case Glyph::PulseUp:
            p.startNewSubPath (b.getX(), b.getBottom());
            p.lineTo (b.getX() + b.getWidth() * 0.25f, b.getBottom());
            p.lineTo (b.getX() + b.getWidth() * 0.25f, b.getY());
            p.lineTo (b.getX() + b.getWidth() * 0.75f, b.getY());
            p.lineTo (b.getX() + b.getWidth() * 0.75f, b.getBottom());
            p.lineTo (b.getRight(), b.getBottom());
            break;
        case Glyph::PulseDown:
            p.startNewSubPath (b.getX(), b.getY());
            p.lineTo (b.getX() + b.getWidth() * 0.25f, b.getY());
            p.lineTo (b.getX() + b.getWidth() * 0.25f, b.getBottom());
            p.lineTo (b.getX() + b.getWidth() * 0.75f, b.getBottom());
            p.lineTo (b.getX() + b.getWidth() * 0.75f, b.getY());
            p.lineTo (b.getRight(), b.getY());
            break;
        case Glyph::RampUp:
            p.startNewSubPath (b.getX(), b.getBottom());
            p.lineTo (b.getRight() - b.getWidth() * 0.3f, b.getY());
            p.lineTo (b.getRight(), b.getY());
            break;
        case Glyph::RampDown:
            p.startNewSubPath (b.getX(), b.getY());
            p.lineTo (b.getRight() - b.getWidth() * 0.3f, b.getBottom());
            p.lineTo (b.getRight(), b.getBottom());
            break;
        case Glyph::ArcAR:
            p.startNewSubPath (b.getX(), b.getBottom());
            p.quadraticTo (b.getCentreX(), b.getY() - b.getHeight() * 0.2f, b.getRight(), b.getBottom());
            break;
    }
    g.setColour (col::ink);
    g.strokePath (p, juce::PathStrokeType (1.2f));
}

} // namespace pf50ui
