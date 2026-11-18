// Paperface 50 — per-module face layouts, transcribed from the manual's panel
// drawings (pp. 8–24). Coordinates are in panel inches.
#pragma once
#include "Widgets.h"
#include "../engine/System.h"

namespace pf50ui {

using namespace pf50;

inline void buildVCO (ModuleComp& c, VCO& m)
{
    const bool plus = m.plus;
    // sine pair (left column) + wave shape (right)
    c.addBox (0.14f, 0.72f, plus ? 2.72f : 1.72f, 2.62f);
    c.addLine (1.0f, 0.72f, 1.0f, 2.62f); // inner compartment divider (ref p8/9)
    c.addJack (VCO::jSineB, 0.5f, 1.1f);
    c.addLine (0.5f, 1.32f, 0.5f, 1.78f);
    c.addJack (VCO::jSineW, 0.5f, 2.0f);
    c.addGlyph (Glyph::Sine, 0.5f, 2.42f, 0.42f, 0.22f);
    c.addKnob (VCO::cShape, 1.5f, 1.12f, true, true);
    c.addLabel ("WAVE\nSHAPE", 1.5f, 1.62f, 1.0f, 0.14f);
    c.addJack (VCO::jShapeCV, 1.5f, 2.25f);
    // saw row — the OSCILLATOR+ drawing shows only saw white + pulse red here;
    // its bipolar saw stays patchable on the basic OSCILLATOR panel
    c.addGlyph (Glyph::Saw, 0.28f, 3.0f, 0.34f, 0.2f);
    c.addJack (VCO::jSawW, 0.62f, 3.0f);
    if (! plus)
        c.addJack (VCO::jSawB, 1.5f, 3.0f);
    else
    {
        c.addGlyph (Glyph::PulseUp, 1.55f, 2.7f, 0.34f, 0.2f);
        c.addJack (VCO::jPulse, 1.55f, 3.05f);
    }
    // pitch section
    c.addBox (0.14f, 3.48f, 1.72f, 2.28f);
    c.addJack (VCO::jCV1, 0.5f, 3.85f);
    c.addJack (VCO::jCV2, 1.5f, 3.85f);
    c.addJack (VCO::jV1O, 1.0f, 4.42f, "1V/O", true);
    c.addKnob (VCO::cProc1, 0.5f, 5.12f, false, true);
    c.addKnob (VCO::cProc2, 1.5f, 5.12f, false, true);
    c.addLine (0.5f, 4.05f, 0.5f, 4.85f);
    c.addLine (1.5f, 4.05f, 1.5f, 4.85f);
    // sync / freq
    c.addJack (VCO::jSync, 0.5f, 6.32f, "SYNC");
    c.addKnob (VCO::cFreq, 1.5f, 6.28f, true);
    c.addLabel ("FREQ", 1.5f, 6.66f, 0.9f, 0.15f, true);
    if (plus)
    {
        c.addJack (VCO::jVCPWM, 2.5f, 1.0f, "VC PWM", true);
        c.addKnob (VCO::cPWMAtt, 2.5f, 1.62f, false);
        c.addKnob (VCO::cPW, 2.5f, 2.42f, false);
        c.addLabel ("PULSEWIDTH", 2.42f, 2.72f, 1.0f, 0.13f);
        c.addBox (1.94f, 0.72f, 0.92f, 2.22f);
        c.addJack (VCO::jWhite, 2.5f, 3.55f, "WHITE");
        c.addJack (VCO::jPink,  2.5f, 4.55f, "PINK");
        c.addJack (VCO::jSHSrc, 2.5f, 6.32f, "S/H SRC");
    }
}

inline void buildSSG (ModuleComp& c, SSG& m)
{
    juce::ignoreUnused (m);
    // smooth
    c.addBox (0.14f, 0.72f, 2.72f, 2.6f);
    c.addJack (SSG::jSmIn, 0.5f, 1.1f, "~IN", true);
    c.addLed (0, 1.28f, 1.1f);
    c.addLine (1.42f, 1.1f, 1.62f, 1.1f);
    c.addJack (SSG::jSmOut, 1.85f, 1.1f);
    c.addLabel ("SMOOTH", 1.55f, 1.35f, 1.2f, 0.15f);
    c.addKnob (SSG::cSmRate, 0.55f, 2.05f, true);
    c.addKnob (SSG::cSmVC, 0.55f, 3.0f, false, true);
    c.addJack (SSG::jSmCycle, 1.5f, 2.05f, "CYCLE");
    c.addJack (SSG::jSmHold, 1.5f, 3.0f, "HOLD");
    c.addJack (SSG::jSmVC, 2.5f, 2.05f);            // black VC input in a diamond
    c.addDiamond (2.5f, 2.05f);
    // coupler column: black "hot" above red, label under the red
    c.addJack (SSG::jCoupHot, 2.5f, 3.5f);
    c.addJack (SSG::jCoupRed, 2.5f, 4.15f);
    c.addLine (2.5f, 3.7f, 2.5f, 3.95f);
    c.addLabel ("COUPLER", 2.45f, 4.42f, 1.0f, 0.14f);
    // stepped
    c.addBox (0.14f, 3.55f, 2.05f, 3.1f);
    c.addJack (SSG::jStIn, 0.5f, 3.95f, "~IN", true);
    c.addJack (SSG::jStSample, 1.5f, 3.95f, "SAMPLE");
    c.addKnob (SSG::cStRate, 0.55f, 4.9f, true);
    c.addJack (SSG::jStCycle, 1.5f, 4.9f, "CYCLE");
    c.addKnob (SSG::cStVC, 0.55f, 5.95f, false, true);
    c.addJack (SSG::jStVC, 2.5f, 5.45f);
    c.addDiamond (2.5f, 5.45f);
    c.addLed (1, 1.28f, 6.05f);
    c.addLine (1.42f, 6.05f, 1.62f, 6.05f);
    c.addJack (SSG::jStOut, 1.85f, 6.05f);
    c.addLabel ("STEPPED", 1.55f, 6.32f, 1.2f, 0.15f);
}

inline void buildCVPro (ModuleComp& c, CVPro& m)
{
    juce::ignoreUnused (m);
    for (int s = 0; s < 2; ++s)
    {
        float y0 = 0.85f + 2.95f * s;
        int jb = s * 4, cb = s * 4;
        c.addBox (0.14f, y0 - 0.1f, 2.72f, 2.8f);
        for (int i = 0; i < 3; ++i)
        {
            float y = y0 + 0.32f + 0.95f * i;
            c.addJack (jb + i, 0.5f, y);
            c.addDiamond (0.5f, y); // diamond marks a processed input
            if (i < 2) // interlocking ribbon between neighbouring diamonds
            {
                c.addLine (0.5f, y + 0.34f, 0.5f, y + 0.61f);
            }
            c.addKnob (cb + i, 1.5f, y, false, true);
            c.arches.push_back ({ 1.5f - 0.33f, y - 0.35f, 0.66f, 0.33f }); // ref arches all knobs here
        }
        c.addJack (jb + 3, 2.45f, y0 + 0.32f, "~OUT", false);
        c.addLine (2.45f, y0 + 0.54f, 2.45f, y0 + 0.9f); // out jack → level LED
        c.addLed (s, 2.45f, y0 + 1.05f);
        c.addKnob (cb + 3, 2.45f, y0 + 2.2f, false);
        c.addLabel ("~OFFSET", 2.42f, y0 + 2.5f, 0.9f, 0.14f);
    }
}

inline void buildTWS (ModuleComp& c, TWS& m)
{
    juce::ignoreUnused (m);
    for (int s = 0; s < 3; ++s)
    {
        float y0 = 0.85f + 1.98f * s;
        int jb = s * 5;
        c.addBox (0.14f, y0 - 0.08f, 2.72f, 1.98f); // contiguous sections (ref p13)
        c.addJack (jb + 0, 0.5f, y0 + 0.28f, "~IN", true);
        c.addJack (jb + 1, 1.5f, y0 + 0.28f, "~VC 1", true);
        c.addJack (jb + 3, 2.5f, y0 + 0.28f);
        c.addLine (2.5f, y0 + 0.5f, 2.5f, y0 + 0.95f);
        c.addKnob ((int) TWS::cAtt0 + s, 0.55f, y0 + 1.25f, true);
        c.addJack (jb + 2, 1.5f, y0 + 1.25f, "~VC 2", true);
        // bowtie device framing the bipolar VC input (per drawing)
        c.addLine (1.16f, y0 + 0.97f, 1.16f, y0 + 1.53f);
        c.addLine (1.16f, y0 + 0.97f, 1.44f, y0 + 1.25f);
        c.addLine (1.16f, y0 + 1.53f, 1.44f, y0 + 1.25f);
        c.addLine (1.84f, y0 + 0.97f, 1.84f, y0 + 1.53f);
        c.addLine (1.84f, y0 + 0.97f, 1.56f, y0 + 1.25f);
        c.addLine (1.84f, y0 + 1.53f, 1.56f, y0 + 1.25f);
        c.addJack (jb + 4, 2.5f, y0 + 1.25f);
    }
}

inline void buildVCF (ModuleComp& c, VCF1973& m)
{
    juce::ignoreUnused (m);
    // balanced out
    c.addBox (0.14f, 0.72f, 1.72f, 0.85f);
    c.addJack (VCF1973::jBalIn, 0.42f, 1.08f, "IN", true);
    // fat arrow IN -> BAL OUT
    c.addLine (0.72f, 1.03f, 1.0f, 1.03f);
    c.addLine (0.72f, 1.13f, 1.0f, 1.13f);
    c.addLine (1.0f, 0.95f, 1.14f, 1.08f);
    c.addLine (1.0f, 1.21f, 1.14f, 1.08f);
    c.addTRS (1.48f, 1.08f);
    c.addLabel ("BAL. OUT", 1.48f, 1.34f, 0.9f, 0.13f);
    // filter outs
    c.addBox (0.14f, 1.68f, 1.72f, 1.62f);
    c.addJack (VCF1973::jHigh, 0.5f, 2.0f, "HIGH");
    c.addJack (VCF1973::jBand, 1.5f, 2.0f, "BAND");
    c.addJack (VCF1973::jLow, 0.5f, 2.9f, "LOW");
    c.addToggle (VCF1973::cLowRange, 1.5f, 2.88f, 2);
    c.addLabel ("LOW", 1.5f, 3.12f, 0.7f, 0.13f);
    // input & VC
    c.addJack (VCF1973::jIn, 0.5f, 3.95f, "IN");
    c.addDiamond (0.5f, 3.95f, 0.27f); // sized to clear the IN label
    c.addJack (VCF1973::jVCFreq, 1.5f, 3.95f, "VC FREQ");
    c.addKnob (VCF1973::cGain, 0.5f, 4.95f, false);
    c.addKnob (VCF1973::cVCProc, 1.5f, 4.95f, false, true);
    c.addLine (0.5f, 4.15f, 0.5f, 4.68f);
    c.addLine (1.5f, 4.15f, 1.5f, 4.68f);
    c.addKnob (VCF1973::cFreq, 0.5f, 6.15f, true);
    c.addLabel ("FREQ", 0.5f, 6.55f, 0.8f, 0.15f, true);
    c.addKnob (VCF1973::cQ, 1.5f, 6.15f, true);
    c.addLabel ("Q", 1.5f, 6.55f, 0.5f, 0.15f, true);
}

inline void buildPosSlew (ModuleComp& c, PosSlew& m)
{
    juce::ignoreUnused (m);
    for (int s = 0; s < 2; ++s)
    {
        float y0 = 0.85f + 3.06f * s;
        int jb = s * 8, cb = s * 2;
        c.addBox (0.14f, y0 - 0.1f, 2.72f, 2.95f);
        c.addJack (jb + 0, 0.5f, y0 + 0.28f, "~IN", true);
        c.addJack (jb + 4, 1.5f, y0 + 0.28f);
        c.addDiamond (1.5f, y0 + 0.28f, 0.36f);
        c.addJack (jb + 5, 2.5f, y0 + 0.28f);
        c.addGlyph (Glyph::RampUp, 2.32f, y0 + 0.62f, 0.3f, 0.18f);
        c.addLed (s, 2.68f, y0 + 0.62f);
        c.addJack (jb + 1, 0.92f, y0 + 0.8f, "", true);
        c.addKnob (cb + 1, 0.5f, y0 + 1.25f, false, true);
        c.addJack (jb + 2, 1.5f, y0 + 1.25f, "START");
        c.addGlyph (Glyph::PulseUp, 2.5f, y0 + 1.0f, 0.32f, 0.18f);
        c.addJack (jb + 6, 2.5f, y0 + 1.32f);
        c.addKnob (cb + 0, 0.5f, y0 + 2.3f, true);
        c.addJack (jb + 3, 1.5f, y0 + 2.3f, "SUSTAIN");
        c.addGlyph (Glyph::PulseDown, 2.5f, y0 + 2.05f, 0.32f, 0.18f);
        c.addJack (jb + 7, 2.5f, y0 + 2.38f);
    }
}

inline void buildNegSlew (ModuleComp& c, NegSlew& m)
{
    juce::ignoreUnused (m);
    for (int s = 0; s < 2; ++s)
    {
        float y0 = 0.85f + 3.06f * s;
        int jb = s * 6, cb = s * 2;
        c.addBox (0.14f, y0 - 0.1f, 1.72f, 2.95f);
        c.addJack (jb + 0, 0.5f, y0 + 0.28f, "~IN", true);
        c.addJack (jb + 3, 1.4f, y0 + 0.28f);
        c.addDiamond (1.4f, y0 + 0.28f);
        c.addJack (jb + 2, 0.95f, y0 + 0.82f, "", true); // wildcard
        c.addKnob (cb + 1, 0.5f, y0 + 1.3f, false, true);
        c.addGlyph (Glyph::PulseUp, 1.4f, y0 + 1.05f, 0.32f, 0.18f);
        c.addJack (jb + 5, 1.4f, y0 + 1.36f);
        c.addKnob (cb + 0, 0.5f, y0 + 2.32f, true);
        c.addGlyph (Glyph::RampDown, 1.22f, y0 + 2.1f, 0.3f, 0.18f);
        c.addJack (jb + 4, 1.4f, y0 + 2.42f);
        c.addLed (s, 1.74f, y0 + 2.42f);
    }
}

inline void buildEnv (ModuleComp& c, EnvGen& m)
{
    juce::ignoreUnused (m);
    c.addBox (0.14f, 0.72f, 1.72f, 5.95f);
    c.addJack (EnvGen::jEnd, 0.5f, 1.1f, "END");
    c.addJack (EnvGen::jOut, 1.4f, 1.1f);
    c.addGlyph (Glyph::ArcAR, 1.4f, 1.48f, 0.34f, 0.18f);
    c.addLed (0, 1.74f, 1.48f);
    c.addJack (EnvGen::jDur2, 0.95f, 1.58f, "", true); // center VC jack (drawing p17)
    c.addJack (EnvGen::jStart, 0.5f, 2.05f, "START");
    c.addGlyph (Glyph::PulseUp, 1.4f, 1.82f, 0.3f, 0.16f);
    c.addJack (EnvGen::jWindow, 1.4f, 2.12f);
    c.addLed (1, 1.74f, 2.12f);
    c.addJack (EnvGen::jCycle, 0.5f, 3.0f, "CYCLE");
    c.addJack (EnvGen::jSize, 1.4f, 3.0f, "SIZE");
    c.addJack (EnvGen::jHold, 0.5f, 3.95f, "HOLD");
    c.addKnob (EnvGen::cSize, 1.4f, 3.95f, false);
    c.addJack (EnvGen::jDur, 0.5f, 4.95f, "DURATION", true);
    c.addKnob (EnvGen::cRise, 1.4f, 4.95f, false, false, "RISE");
    c.addKnob (EnvGen::cDurProc, 0.5f, 5.95f, false, true);
    c.addKnob (EnvGen::cFall, 1.4f, 5.95f, false, false, "FALL");
}

inline void buildGates (ModuleComp& c, DualGates& m)
{
    juce::ignoreUnused (m);
    for (int s = 0; s < 2; ++s)
    {
        float y0 = 0.85f + 3.0f * s;
        int jb = s * 5, cb = s * 2;
        c.addBox (0.14f, y0 - 0.1f, 2.72f, 2.9f);
        c.addJack (jb + 0, 0.5f, y0 + 0.28f);
        c.addKnob (cb + 0, 1.9f, y0 + 0.28f, true);
        c.addLabel ("GAIN", 1.9f, y0 + 0.62f, 0.8f, 0.14f, true);
        c.addJack (jb + 1, 0.5f, y0 + 1.05f, "LIN");
        c.addJack (jb + 4, 1.9f, y0 + 1.45f);
        c.addLine (1.9f, y0 + 1.05f, 2.35f, y0 + 1.45f);
        c.addLine (2.35f, y0 + 1.45f, 1.9f, y0 + 1.85f);
        c.addLine (1.9f, y0 + 1.85f, 1.45f, y0 + 1.45f);
        c.addLine (1.45f, y0 + 1.45f, 1.9f, y0 + 1.05f);
        c.addToggle (cb + 1, 0.95f, y0 + 1.62f, 3);
        c.addLabel ("DC",  1.32f, y0 + 1.38f, 0.4f, 0.125f);
        c.addLabel ("AC",  1.32f, y0 + 1.56f, 0.4f, 0.125f);
        c.addLabel ("MOD", 1.32f, y0 + 1.74f, 0.5f, 0.125f);
        c.addJack (jb + 2, 0.5f, y0 + 2.35f, "LOG");
        c.addJack (jb + 3, 1.9f, y0 + 2.35f, "MOD");
    }
}

inline void buildGateSeq (ModuleComp& c, GateSeq& m)
{
    juce::ignoreUnused (m);
    c.addBox (0.14f, 0.72f, 1.72f, 4.05f);
    // counter-clockwise: 1..4 down the left, 5..8 up the right
    for (int i = 0; i < 4; ++i)
    {
        float y = 1.1f + i * 0.95f;
        c.addJack (i, 0.55f, y);
        c.addLed (i, 0.55f, y + 0.42f);
        int rightStage = 7 - i; // top-right = 8
        c.addJack (rightStage, 1.45f, y);
        c.addLed (rightStage, 1.45f, y + 0.42f);
    }
    // tapering fan from the stage box down to CLOCK / RESET
    c.addLine (0.14f, 4.77f, 0.55f, 5.05f);
    c.addLine (1.86f, 4.77f, 1.45f, 5.05f);
    c.addLine (1.0f, 4.77f, 0.72f, 5.05f);
    c.addLine (1.0f, 4.77f, 1.28f, 5.05f);
    c.addJack (GateSeq::jClock, 0.55f, 5.25f, "CLOCK");
    c.addJack (GateSeq::jReset, 1.45f, 5.25f, "RESET");
    c.addJack (GateSeq::jHold, 0.55f, 6.3f, "HOLD");
    auto* b = c.addButton (GateSeq::cResetBtn, 1.45f, 6.3f, 0.27f);
    juce::ignoreUnused (b);
    c.addLabel ("RESET", 1.45f, 6.62f, 0.8f, 0.14f);
}

inline void buildSeq4 (ModuleComp& c, Seq4& m)
{
    juce::ignoreUnused (m);
    c.addBox (0.14f, 0.72f, 3.9f, 3.45f);
    for (int i = 0; i < 4; ++i)
    {
        float x = 0.62f + 0.98f * i;
        c.addJack (Seq4::jG1 + i, x, 1.05f);                        // per-stage gate out
        c.addKnob (0 + i, x, 1.85f, false);                          // A row
        c.addKnob (4 + i, x, 2.72f, false);                          // B row
        c.addKnob (8 + i, x, 3.59f, false);                          // C row
        c.addJack (Seq4::jSel1 + i, x, 4.55f);                       // stage select in
        c.addLed (i, x, 5.1f);
        c.addButton (12 + i, x, 5.75f, 0.3f);
    }
    // per-stage column dividers inside the knob grid
    for (int i = 1; i < 4; ++i)
        c.addLine (0.62f + 0.98f * i - 0.49f, 0.72f, 0.62f + 0.98f * i - 0.49f, 4.17f);
    c.addJack (Seq4::jGate, 4.45f, 1.05f, "GATE");
    c.addJack (Seq4::jA, 4.45f, 1.85f, "A");
    c.addJack (Seq4::jB, 4.45f, 2.72f, "B");
    c.addJack (Seq4::jC, 4.45f, 3.59f, "C");
    c.addJack (Seq4::jClock, 3.95f, 4.75f, "CLOCK");
    c.addJack (Seq4::jReset, 4.6f, 4.75f, "RESET");
    c.addJack (Seq4::jHold, 3.95f, 5.85f, "HOLD");
    c.addJack (Seq4::jUpDown, 4.6f, 5.85f);
    c.addLabel ("UP / DOWN", 4.55f, 6.11f, 0.85f, 0.13f);
}

inline void buildClock (ModuleComp& c, MetroClock& m)
{
    juce::ignoreUnused (m);
    c.addBox (0.14f, 0.72f, 0.74f, 5.9f);
    c.addJack (MetroClock::jOut, 0.5f, 1.1f);
    c.addLabel ("METRO-\nNOME", 0.5f, 1.34f, 0.8f, 0.12f);
    c.addLed (0, 0.5f, 2.1f);
    c.addKnob (MetroClock::cFine, 0.5f, 3.0f, false, false, "FINE");
    c.addKnob (MetroClock::cTempo, 0.5f, 4.3f, true, false, "TEMPO");
    c.arches.pop_back(); // reference draws no arch over TEMPO
}

inline void buildRouter (ModuleComp& c, BiRouter& m)
{
    juce::ignoreUnused (m);
    for (int s = 0; s < 3; ++s)
    {
        float y0 = 0.85f + 1.95f * s;
        int jb = s * 4;
        c.addBox (0.14f, y0 - 0.08f, 1.72f, 1.82f);
        // reference puts the SWITCH/A0 labels above their jacks
        c.addJack (jb + 0, 0.5f, y0 + 0.28f, "A1");
        c.addJack (jb + 1, 1.4f, y0 + 0.28f, "B");
        if (s < 2)
        {
            c.addLabel ("SWITCH", 0.5f, y0 + 0.84f, 0.8f, 0.135f);
            c.addLabel ("A0", 1.4f, y0 + 0.84f, 0.5f, 0.135f);
            c.addJack (jb + 3, 0.5f, y0 + 1.28f);
            c.addJack (jb + 2, 1.4f, y0 + 1.28f);
        }
        else // flip-flop section: FLOP sits centre, labels move under
        {
            c.addJack (BiRouter::jFlop, 0.95f, y0 + 0.72f);
            c.addLabel ("FLOP", 0.95f, y0 + 0.94f, 0.55f, 0.115f);
            c.addJack (jb + 3, 0.5f, y0 + 1.28f, "SWITCH");
            c.addJack (jb + 2, 1.4f, y0 + 1.28f, "A0");
        }
    }
}

inline void buildPeakTrough (ModuleComp& c, PeakTrough& m)
{
    juce::ignoreUnused (m);
    c.addBox (0.14f, 0.72f, 0.74f, 3.9f);
    c.addJack (PeakTrough::jPeak, 0.5f, 1.1f, "PEAK");
    c.addJack (PeakTrough::jTrough, 0.5f, 2.05f, "TROUGH");
    for (int i = 0; i < 2; ++i)
    {
        float y = 3.1f + 0.95f * i;
        c.addJack (PeakTrough::jIn1 + i, 0.5f, y);
        c.addDiamond (0.5f, y);
    }
    c.addLine (0.5f, 3.44f, 0.5f, 3.71f); // interlocking ribbon
}

inline void buildNTO (ModuleComp& c, NTO& m)
{
    juce::ignoreUnused (m);
    // 2-inch panel: top wave outputs, middle timbre & FM, bottom 1V/O, portamento & freq
    c.addBox (0.14f, 0.72f, 1.72f, 1.85f);
    c.addJack (NTO::jVariableOut, 0.5f, 1.05f, "VAR", true);
    c.addJack (NTO::jSineOut,     1.5f, 1.05f, "SINE", true);
    c.addJack (NTO::jSawOut,      0.5f, 1.95f, "SAW", true);
    c.addJack (NTO::jSubOut,      1.5f, 1.95f, "SUB", true);
    c.addLed (0, 1.0f, 1.5f);

    c.addBox (0.14f, 2.68f, 1.72f, 1.9f);
    c.addKnob (NTO::cTimbre,   0.55f, 3.15f, false, false, "TIMBRE");
    c.addJack (NTO::jTimbreCV, 1.5f,  3.15f, "VC", true);
    c.addKnob (NTO::cFMLinAtt, 0.55f, 4.05f, false, false, "LIN FM");
    c.addJack (NTO::jFMLin,    1.5f,  4.05f, "LIN", true);

    c.addBox (0.14f, 4.68f, 1.72f, 2.05f);
    c.addJack (NTO::jV1O,      0.5f,  5.0f,  "1V/O", true);
    c.addJack (NTO::jPortCV,   1.0f,  5.0f,  "PORTA", true);
    c.addJack (NTO::jFMExp,    1.5f,  5.0f,  "EXP", true);
    c.addKnob (NTO::cPort,     0.45f, 5.75f, false);
    c.addKnob (NTO::cFMExpAtt, 1.55f, 5.75f, false, true);
    c.addJack (NTO::jSync,     0.45f, 6.38f, "SYNC", true);
    c.addKnob (NTO::cFreq,     1.35f, 6.28f, true, false, "FREQ");
}

inline void buildDUSG (ModuleComp& c, DUSG& m)
{
    juce::ignoreUnused (m);
    // 4-inch panel: two 2-inch slope channels side by side
    for (int s = 0; s < 2; ++s)
    {
        float x0 = 2.0f * s;
        int jb = s * 7, cb = s * 4;
        c.addBox (x0 + 0.12f, 0.72f, 1.76f, 5.95f);
        c.addJack (jb + 0, x0 + 0.5f, 1.1f,  "IN", true);
        c.addJack (jb + 1, x0 + 1.5f, 1.1f,  "TRIG", true);
        c.addJack (jb + 4, x0 + 0.5f, 1.95f, "BIP", true);
        c.addJack (jb + 5, x0 + 1.5f, 1.95f, "OUT", true);
        c.addLed  (s,      x0 + 1.0f, 1.95f);

        c.addJack (jb + 6, x0 + 0.5f, 2.85f, "END", true);
        c.addToggle (cb + 3, x0 + 1.5f, 2.85f, 2);
        c.addLabel ("CYCLE", x0 + 1.5f, 3.16f, 0.7f, 0.12f);

        c.addJack (jb + 2, x0 + 0.5f, 3.75f, "RISE VC", true);
        c.addJack (jb + 3, x0 + 1.5f, 3.75f, "FALL VC", true);
        c.addKnob (cb + 0, x0 + 0.5f, 4.75f, true, false, "RISE");
        c.addKnob (cb + 1, x0 + 1.5f, 4.75f, true, false, "FALL");
        c.addKnob (cb + 2, x0 + 1.0f, 5.95f, false, true, "LOG / EXP");
    }
}

inline void buildVCFQ (ModuleComp& c, VCFQ& m)
{
    juce::ignoreUnused (m);
    // 2-inch panel: outputs top, inputs mid, controls bottom
    c.addBox (0.14f, 0.72f, 1.72f, 1.85f);
    c.addJack (VCFQ::jLP,    0.5f, 1.08f, "LOW", true);
    c.addJack (VCFQ::jBP,    1.5f, 1.08f, "BAND", true);
    c.addJack (VCFQ::jHP,    0.5f, 1.95f, "HIGH", true);
    c.addJack (VCFQ::jNotch, 1.5f, 1.95f, "NOTCH", true);
    c.addLed  (0,            1.0f, 1.52f);

    c.addBox (0.14f, 2.68f, 1.72f, 1.9f);
    c.addJack (VCFQ::jIn,       0.5f, 3.05f, "IN", true);
    c.addJack (VCFQ::jTrigPing, 1.5f, 3.05f, "PING", true);
    c.addJack (VCFQ::jV1O,      0.5f, 3.95f, "1V/O", true);
    c.addKnob (VCFQ::cGain,     1.5f, 3.95f, false, false, "GAIN");

    c.addBox (0.14f, 4.68f, 1.72f, 2.05f);
    c.addJack (VCFQ::jVCFreq, 0.5f, 5.02f, "VC F", true);
    c.addJack (VCFQ::jVCQ,    1.5f, 5.02f, "VC Q", true);
    c.addKnob (VCFQ::cFreqVC, 0.5f, 5.65f, false, true);
    c.addKnob (VCFQ::cQVC,    1.5f, 5.65f, false, true);
    c.addKnob (VCFQ::cFreq,   0.5f, 6.32f, true);
    c.addKnob (VCFQ::cQ,      1.5f, 6.32f, true);
}

inline void buildVCM (ModuleComp& c, WaveMultipliers& m)
{
    juce::ignoreUnused (m);
    // 3-inch panel: 3 stacked horizontal sections (Grinder, Folder, Rectifier)
    // Section 1: Grinder
    c.addBox (0.14f, 0.72f, 2.72f, 1.82f);
    c.addJack (WaveMultipliers::jIn1,   0.5f, 1.15f, "IN 1", true);
    c.addJack (WaveMultipliers::jVC1,   1.5f, 1.15f, "VC 1", true);
    c.addJack (WaveMultipliers::jOut1,  2.5f, 1.15f, "OUT 1", true);
    c.addKnob (WaveMultipliers::cGain1, 1.0f, 1.98f, false, false, "GRINDER");

    // Section 2: Middle Wavefolder
    c.addBox (0.14f, 2.64f, 2.72f, 2.15f);
    c.addJack (WaveMultipliers::jIn2,       0.5f, 3.05f, "IN 2", true);
    c.addJack (WaveMultipliers::jVC2,       1.5f, 3.05f, "VC FOLD", true);
    c.addJack (WaveMultipliers::jOut2,      2.5f, 3.05f, "OUT 2", true);
    c.addLed  (0,                           2.5f, 3.72f);
    c.addKnob (WaveMultipliers::cFold2,     0.6f, 4.1f,  true, false, "FOLD");
    c.addKnob (WaveMultipliers::cSweepAtt2, 1.55f,4.1f,  false, true);
    c.addJack (WaveMultipliers::jSweep2,    2.5f, 4.2f,  "SWEEP", true);

    // Section 3: Full-wave rectifier + VCA
    c.addBox (0.14f, 4.88f, 2.72f, 1.82f);
    c.addJack (WaveMultipliers::jIn3,   0.5f, 5.3f,  "IN 3", true);
    c.addJack (WaveMultipliers::jVC3,   1.5f, 5.3f,  "VC 3", true);
    c.addJack (WaveMultipliers::jOut3,  2.5f, 5.3f,  "OUT 3", true);
    c.addKnob (WaveMultipliers::cGain3, 1.0f, 6.12f, false, false, "RECT VCA");
}

inline void buildResEQ (ModuleComp& c, ResEQ& m)
{
    juce::ignoreUnused (m);
    // 4-inch panel: 10 resonant frequency bands in 2 columns of 5 knobs + Odd/Even/All outputs
    c.addBox (0.14f, 0.72f, 3.72f, 1.25f);
    c.addJack (ResEQ::jIn,      0.5f,  1.15f, "IN", true);
    c.addJack (ResEQ::jFbIn,    1.25f, 1.15f, "FB IN", true);
    c.addLed  (0,               1.85f, 1.15f);
    c.addJack (ResEQ::jOddOut,  2.35f, 1.15f, "ODD", true);
    c.addJack (ResEQ::jAllOut,  2.95f, 1.15f, "ALL", true);
    c.addJack (ResEQ::jEvenOut, 3.55f, 1.15f, "EVEN", true);

    c.addBox (0.14f, 2.08f, 3.72f, 4.62f);
    static const char* bNames[10] = {
        "29 Hz", "61 Hz", "115 Hz", "218 Hz", "411 Hz",
        "777 Hz", "1.5 kHz", "2.8 kHz", "5.2 kHz", "11 kHz"
    };
    for (int b = 0; b < 10; ++b)
    {
        int col = b / 5;
        int row = b % 5;
        float x = (col == 0) ? 1.05f : 2.95f;
        float y = 2.52f + row * 0.86f;
        c.addKnob (b, x, y, false, true, bNames[b]);
    }
}

inline void buildNCOM (ModuleComp& c, NCOM& m)
{
    juce::ignoreUnused (m);
    // 2-inch panel: Divide-by-N & Comparator
    c.addBox (0.14f, 0.72f, 1.72f, 5.95f);
    c.addJack (NCOM::jClkIn,    0.5f, 1.15f, "CLOCK", true);
    c.addJack (NCOM::jDivOut,   1.5f, 1.15f, "/N OUT", true);
    c.addLed  (0,               1.0f, 1.15f);
    c.addJack (NCOM::jVC_N,     0.5f, 2.25f, "VC N", true);
    c.addJack (NCOM::jStairOut, 1.5f, 2.25f, "STAIR", true);
    c.addKnob (NCOM::cDivN,     1.0f, 3.35f, true, false, "DIVIDE N");

    c.addLine (0.14f, 4.15f, 1.86f, 4.15f);
    c.addJack (NCOM::jCompIn,   0.5f, 4.65f, "COMP IN", true);
    c.addJack (NCOM::jCompOut,  1.5f, 4.65f, "COMP", true);
    c.addKnob (NCOM::cThresh,   1.0f, 5.75f, false, false, "THRESHOLD");
}

inline void buildTKB (ModuleComp& c, TKB& m)
{
    juce::ignoreUnused (m);
    // 17-inch wide Touch Activated Keyboard Sequencer (matches hardware TKB layout)
    const float xStart = 0.72f;
    const float xStep  = 1.035f;

    // Top control & output jacks row
    c.addBox (0.18f, 0.56f, 16.64f, 1.28f);
    c.addJack (TKB::jVertClk,  xStart + 0  * xStep, 0.88f, "VERT CLK", true);
    c.addJack (TKB::jABCD,     xStart + 1  * xStep, 0.88f, "ABCD", true);
    c.addJack (TKB::jA,        xStart + 2  * xStep, 0.88f, "A", true);
    c.addJack (TKB::jB,        xStart + 3  * xStep, 0.88f, "B", true);
    c.addJack (TKB::jC,        xStart + 4  * xStep, 0.88f, "C", true);
    c.addJack (TKB::jD,        xStart + 5  * xStep, 0.88f, "D", true);
    c.addJack (TKB::jKeyV,     xStart + 6  * xStep, 0.88f, "KEY V", true);
    c.addJack (TKB::jKeyPulse, xStart + 7  * xStep, 0.88f, "KEY PLS", true);
    c.addJack (TKB::jPressure, xStart + 8  * xStep, 0.88f, "PRESS", true);
    c.addJack (TKB::jVReset,   xStart + 9  * xStep, 0.88f, "V RESET", true);
    c.addJack (TKB::jReset,    xStart + 10 * xStep, 0.88f, "RESET", true);
    c.addJack (TKB::jRandom,   xStart + 11 * xStep, 0.88f, "RANDOM", true);
    c.addToggle (TKB::cKeyboardToggle, xStart + 12 * xStep, 0.86f, 2);
    c.addLabel ("KEYBD", xStart + 12 * xStep, 1.12f, 0.8f, 0.11f);
    c.addJack (TKB::jDir,      xStart + 13 * xStep, 0.88f, "DIR", true);
    c.addJack (TKB::jHold,     xStart + 14 * xStep, 0.88f, "HOLD", true);
    c.addJack (TKB::jClock,    xStart + 15 * xStep, 0.88f, "CLOCK", true);

    // 16 Red stage-select jacks
    for (int i = 0; i < 16; ++i)
        c.addJack (TKB::jSel0 + i, xStart + i * xStep, 1.54f, "", true);

    // 4 rows (A, B, C, D) x 16 stages = 64 knobs + 4 vertical row LEDs on left
    const float rowY[4] = { 2.22f, 2.96f, 3.70f, 4.44f };
    for (int r = 0; r < 4; ++r)
    {
        c.addLed (16 + r, 0.28f, rowY[r], 0.10f);
        for (int s = 0; s < 16; ++s)
            c.addKnob (r * 16 + s, xStart + s * xStep, rowY[r], false);
    }

    // Touch keyboard strip at bottom: 16 LEDs + 16 golden oval capacitive plates
    c.addBox (0.18f, 4.98f, 16.64f, 1.84f);
    for (int i = 0; i < 16; ++i)
    {
        float x = xStart + i * xStep;
        c.addLed (i, x, 5.20f, 0.10f);
        c.addTouchPlate (TKB::cPlatesStart + i, i + 1, x, 6.02f, 0.84f, 1.32f);
    }
}

inline void buildModule (ModuleComp& c)
{
    auto& m = c.mod;
    if (auto* v = dynamic_cast<VCO*> (&m))             { buildVCO (c, *v); return; }
    if (auto* v = dynamic_cast<SSG*> (&m))             { buildSSG (c, *v); return; }
    if (auto* v = dynamic_cast<CVPro*> (&m))           { buildCVPro (c, *v); return; }
    if (auto* v = dynamic_cast<TWS*> (&m))             { buildTWS (c, *v); return; }
    if (auto* v = dynamic_cast<VCF1973*> (&m))         { buildVCF (c, *v); return; }
    if (auto* v = dynamic_cast<PosSlew*> (&m))         { buildPosSlew (c, *v); return; }
    if (auto* v = dynamic_cast<NegSlew*> (&m))         { buildNegSlew (c, *v); return; }
    if (auto* v = dynamic_cast<EnvGen*> (&m))          { buildEnv (c, *v); return; }
    if (auto* v = dynamic_cast<DualGates*> (&m))       { buildGates (c, *v); return; }
    if (auto* v = dynamic_cast<GateSeq*> (&m))         { buildGateSeq (c, *v); return; }
    if (auto* v = dynamic_cast<Seq4*> (&m))            { buildSeq4 (c, *v); return; }
    if (auto* v = dynamic_cast<MetroClock*> (&m))      { buildClock (c, *v); return; }
    if (auto* v = dynamic_cast<BiRouter*> (&m))        { buildRouter (c, *v); return; }
    if (auto* v = dynamic_cast<PeakTrough*> (&m))      { buildPeakTrough (c, *v); return; }
    if (auto* v = dynamic_cast<NTO*> (&m))             { buildNTO (c, *v); return; }
    if (auto* v = dynamic_cast<DUSG*> (&m))            { buildDUSG (c, *v); return; }
    if (auto* v = dynamic_cast<VCFQ*> (&m))            { buildVCFQ (c, *v); return; }
    if (auto* v = dynamic_cast<WaveMultipliers*> (&m)) { buildVCM (c, *v); return; }
    if (auto* v = dynamic_cast<ResEQ*> (&m))           { buildResEQ (c, *v); return; }
    if (auto* v = dynamic_cast<NCOM*> (&m))            { buildNCOM (c, *v); return; }
    if (auto* v = dynamic_cast<TKB*> (&m))             { buildTKB (c, *v); return; }
}

} // namespace pf50ui
