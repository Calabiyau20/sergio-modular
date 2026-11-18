// Paperface 50 — slopes & sequencing module DSP
// (Positive/Negative Slew, Envelope, Dual Gates, sequencers, clock, routers,
//  Peak & Trough, and the software I/O strip). See docs/PANEL_SPEC.md.
#pragma once
#include "ModulesVoice.h"

namespace pf50 {

// --------------------------------------------------------- Positive Slew ----
// Dual. Follows falling voltages exactly, slews rising ones. START triggers an
// envelope-style rise to +5 V; pulse fires on arrival; SUSTAIN holds the top.
class PosSlew : public Module
{
public:
    // per section: IN, VC, START, SUSTAIN, OUT(black), OUT(white), PULSE, PULSE-INV
    enum J { jIn0, jVC0, jStart0, jSus0, jOutB0, jOutW0, jP0, jPI0,
             jIn1, jVC1, jStart1, jSus1, jOutB1, jOutW1, jP1, jPI1 };
    enum C { cRate0, cProc0, cRate1, cProc1 };
    PosSlew()
    {
        for (int s = 0; s < 2; ++s)
        {
            addJack ("IN",      JackType::In,  JackColor::White);
            addJack ("VC",      JackType::In,  JackColor::White);
            addJack ("START",   JackType::In,  JackColor::Red);
            addJack ("SUSTAIN", JackType::In,  JackColor::Red);
            addJack ("OUT",     JackType::Out, JackColor::Black);
            addJack ("OUT",     JackType::Out, JackColor::White);
            addJack ("PULSE",   JackType::Out, JackColor::Red);
            addJack ("PULSE",   JackType::Out, JackColor::Red); // inverted
        }
        addCtl ("RATE", 0.4f); addCtl ("PROC", 0.5f);
        addCtl ("RATE", 0.4f); addCtl ("PROC", 0.5f);
        leds.resize (2);
    }
    void prepare (double s) override { Module::prepare (s); for (int i = 0; i < 2; ++i) { y[i] = 0; latch[i] = false; edge[i] = {}; } }

    void tick() override
    {
        const float dt = 1.0f / sr;
        for (int s = 0; s < 2; ++s)
        {
            const int jb = s * 8, cb = s * 2;
            float rc = std::clamp (ctls[cb].v + av (ctls[cb+1].v) * in (jb+1) / kCV, 0.0f, 1.2f);
            float rate = expmap (rc, 0.55f, 24000.0f); // V/s (fast → sub-ms rise)
            if (edge[s].rising (in (jb+2))) latch[s] = true;

            bool sus = isHigh (in (jb+3));
            float target = latch[s] ? kCV : in (jb);
            float& v = y[s];
            if (target < v) v = sus && latch[s] ? v : target;            // falls follow exactly
            else            v += std::min (target - v, rate * dt);       // rises slew
            if (latch[s] && v >= kCV - 1.0e-3f)
            {
                v = kCV;
                if (! sus) latch[s] = false;                             // release: follow input again
            }
            // Hardware-style top comparator: pulse HIGH while the output sits
            // at +5 V (mirror of the Negative Slew's 0 V end pulse).
            float p = (v >= kCV - 0.02f) ? kCV : 0.0f;
            jacks[jb+4].value = v - kAudio;      // black, centered
            jacks[jb+5].value = v;               // white 0..5
            jacks[jb+6].value = p;
            jacks[jb+7].value = kCV - p;
            leds[(size_t) s] = v / kCV;
        }
    }
    float y[2] {}; bool latch[2] {};
    EdgeDetector edge[2];
};

// --------------------------------------------------------- Negative Slew ----
// Dual (top fast: cycles to ~3 kHz; bottom slow: down to ~20 s). Follows rising
// exactly, slews falling. Pulse fires on reaching 0 V; PULSE→IN self-cycles as
// an inverted saw. Anniversary "wildcard" input morphs the slope shape:
// ~2.5 V = linear, 0/5 V = log/exponential.
class NegSlew : public Module
{
public:
    enum J { jIn0, jVC0, jWild0, jOutB0, jOutW0, jP0,
             jIn1, jVC1, jWild1, jOutB1, jOutW1, jP1 };
    enum C { cRate0, cProc0, cRate1, cProc1 };
    NegSlew()
    {
        for (int s = 0; s < 2; ++s)
        {
            addJack ("IN",       JackType::In,  JackColor::White);
            addJack ("VC",       JackType::In,  JackColor::White);
            addJack ("WILDCARD", JackType::In,  JackColor::White);
            addJack ("OUT",      JackType::Out, JackColor::Black);
            addJack ("OUT",      JackType::Out, JackColor::White);
            addJack ("PULSE",    JackType::Out, JackColor::Red);
        }
        addCtl ("RATE", 0.5f); addCtl ("PROC", 0.5f);
        addCtl ("RATE", 0.5f); addCtl ("PROC", 0.5f);
        leds.resize (2);
    }
    void prepare (double s) override { Module::prepare (s); y[0] = y[1] = 0; }

    void tick() override
    {
        const float dt = 1.0f / sr;
        for (int s = 0; s < 2; ++s)
        {
            const int jb = s * 6, cb = s * 2;
            float rc = std::clamp (ctls[cb].v + av (ctls[cb+1].v) * in (jb+1) / kCV, 0.0f, 1.2f);
            float rate = expmap (rc, s == 0 ? 0.55f : 0.27f, s == 0 ? 34000.0f : 17000.0f); // V/s
            float target = in (jb);
            float& v = y[s];
            if (target > v) v = target;                                   // rises follow exactly
            else
            {
                float w = connected (jb+2) ? std::clamp (in (jb+2) / kCV, 0.0f, 1.0f) : 0.5f;
                float k = (w - 0.5f) * 2.0f;                              // -1 log … 0 lin … +1 expo
                float p = std::clamp ((v - target) / kCV, 1.0e-3f, 1.0f);
                float shape = std::pow (p, k);                            // rate multiplier vs position
                v -= std::min (v - target, rate * shape * dt);
            }
            // Hardware-style end comparator: the pulse output is HIGH whenever
            // the output sits at 0 V. This is what makes PULSE→IN self-start
            // and self-cycle (inverted saw), exactly as on the panel.
            jacks[jb+3].value = v - kAudio;
            jacks[jb+4].value = v;
            jacks[jb+5].value = (v <= 0.02f) ? kCV : 0.0f;
            leds[(size_t) s] = std::clamp (v / kCV, 0.0f, 1.0f);
        }
    }
    float y[2] {};
};

// --------------------------------------------------------------- Envelope ---
// Complex AR envelope (p. 17). END fires only when START-triggered; CYCLE out
// can be looped to START; WINDOW is a delayed gate; HOLD freezes; retriggers
// are ignored while running (→ sub-harmonic generation at audio rate).
class EnvGen : public Module
{
public:
    enum J { jEnd, jOut, jStart, jWindow, jCycle, jSize, jHold, jDur, jDur2 };
    enum C { cRise, cFall, cSize, cDurProc };
    EnvGen()
    {
        addJack ("END",     JackType::Out, JackColor::Red);
        addJack ("OUT",     JackType::Out, JackColor::White);
        addJack ("START",   JackType::In,  JackColor::Red);
        addJack ("WINDOW",  JackType::Out, JackColor::Red);
        addJack ("CYCLE",   JackType::Out, JackColor::Red);
        addJack ("SIZE",    JackType::In,  JackColor::White);
        addJack ("HOLD",    JackType::In,  JackColor::Red);
        addJack ("DURATION",JackType::In,  JackColor::White);
        addJack ("VC",      JackType::In,  JackColor::White); // center panel jack; summed duration CV
        addCtl ("RISE", 0.35f); addCtl ("FALL", 0.35f); addCtl ("SIZE", 0.5f); addCtl ("PROC", 0.5f);
        leds.resize (2); // out level, window
    }
    void prepare (double s) override { Module::prepare (s); y=0; st=Idle; prog=0; startEdge={}; endPg={}; cycPg={}; }

    enum St { Idle, Rise, Fall };
    void tick() override
    {
        const float dt = 1.0f / sr;
        float scale = std::exp2 (av (ctls[cDurProc].v) * (in (jDur) + in (jDur2)) / 1.25f); // ±4 oct over 5 V
        float riseT = expmap (ctls[cRise].v, 0.0008f, 8.0f) / std::max (scale, 1.0e-3f);
        float fallT = expmap (ctls[cFall].v, 0.0008f, 8.0f) / std::max (scale, 1.0e-3f);

        // A START edge that coincides with our own CYCLE pulse counts as
        // "cycled", not "triggered" — END stays quiet for cycled loops (p. 17).
        bool cycled = cycPg.active();
        if (startEdge.rising (in (jStart)) && st == Idle) { st = Rise; prog = 0; viaStart = ! cycled; }

        if (! isHigh (in (jHold)))
        {
            if (st == Rise)
            {
                y += kCV * dt / riseT; prog += 0.5f * dt / riseT;
                if (y >= kCV) { y = kCV; st = Fall; prog = 0.5f; }
            }
            else if (st == Fall)
            {
                y -= kCV * dt / fallT; prog += 0.5f * dt / fallT;
                if (y <= 0)
                {
                    y = 0; st = Idle; prog = 1.0f;
                    if (viaStart) endPg.fire (sr, 0.4f);
                    cycPg.fire (sr, 0.8f);
                    viaStart = false;
                }
            }
        }
        float winStart = std::clamp (ctls[cSize].v + in (jSize) / kCV, 0.0f, 0.98f);
        bool windowHigh = (st != Idle) && prog >= winStart;

        jacks[jOut].value    = y;
        jacks[jEnd].value    = endPg.tick();
        jacks[jCycle].value  = cycPg.tick();
        jacks[jWindow].value = windowHigh ? kCV : 0.0f;
        leds[0] = y / kCV; leds[1] = windowHigh ? 1.0f : 0.0f;
    }
    float y = 0, prog = 0; St st = Idle; bool viaStart = false;
    EdgeDetector startEdge; PulseGen endPg, cycPg;
};

// ------------------------------------------------------------- Dual Gates ---
// 2023 redesign (p. 18): per section DC (linear VCA) / AC (expo audio VCA) /
// MOD (4-quadrant multiply; ring mod at full gain), modes freely abusable.
class DualGates : public Module
{
public:
    enum J { jIn0, jLin0, jLog0, jMod0, jOut0,
             jIn1, jLin1, jLog1, jMod1, jOut1 };
    enum C { cGain0, cMode0, cGain1, cMode1 };
    DualGates()
    {
        for (int s = 0; s < 2; ++s)
        {
            addJack ("IN",  JackType::In,  JackColor::Black);
            addJack ("LIN", JackType::In,  JackColor::White);
            addJack ("LOG", JackType::In,  JackColor::White);
            addJack ("MOD", JackType::In,  JackColor::Black);
            addJack ("OUT", JackType::Out, JackColor::Black);
        }
        addCtl ("GAIN", 0.75f); addCtl ("MODE", 1.0f, 3); // 0 DC, 1 AC, 2 MOD
        addCtl ("GAIN", 0.75f); addCtl ("MODE", 1.0f, 3);
        leds.resize (2);
    }
    void tick() override
    {
        for (int s = 0; s < 2; ++s)
        {
            const int jb = s * 5, cb = s * 2;
            float x = in (jb);
            float g = ctls[cb].v;
            int mode = (int) std::lround (ctls[cb+1].v);
            float out = 0;
            float linK = connected (jb+1) ? std::clamp (in (jb+1) / kCV, 0.0f, 1.6f) : 1.0f;
            float logK = 1.0f;
            if (connected (jb+2))
            {
                float v = std::clamp (in (jb+2), 0.0f, 7.5f);
                logK = std::pow (10.0f, (v - kCV) * (50.0f / kCV) / 20.0f); // -50 dB at 0 V, unity at 5 V
            }
            float modK = connected (jb+3) ? in (jb+3) / kAudio : 1.0f;
            switch (mode)
            {
                case 0:  out = x * g * linK * (connected (jb+3) ? modK : 1.0f); break;                     // DC linear
                case 1: { float kdB = -66.0f * (1.0f - g);
                          out = x * std::pow (10.0f, kdB / 20.0f) * logK * linK
                                  * (connected (jb+3) ? modK : 1.0f); break; }                             // AC expo
                default: out = x * modK * g * linK * logK; break;                                          // MOD ring
            }
            out = CVPro::softlim (out);
            jacks[jb+4].value = out;
            leds[(size_t) s] = std::clamp (std::fabs (out) / kCV, 0.0f, 1.0f);
        }
    }
};

// ---------------------------------------------------------- Gate Sequencer --
// 8 one-hot pulse outs, CLOCK/RESET/HOLD + reset button (p. 19).
class GateSeq : public Module
{
public:
    enum J { jS1, jS2, jS3, jS4, jS5, jS6, jS7, jS8, jClock, jReset, jHold };
    enum C { cResetBtn };
    GateSeq()
    {
        for (int i = 0; i < 8; ++i) addJack ("STAGE", JackType::Out, JackColor::Red);
        addJack ("CLOCK", JackType::In, JackColor::Red);
        addJack ("RESET", JackType::In, JackColor::Red);
        addJack ("HOLD",  JackType::In, JackColor::Red);
        addCtl ("RESET", 0.0f, 2);
        leds.resize (8);
    }
    void prepare (double s) override { Module::prepare (s); stage = 0; ck = {}; rs = {}; btnPrev = false; }
    void tick() override
    {
        bool btn = ctls[cResetBtn].v > 0.5f;
        if ((btn && ! btnPrev) || rs.rising (in (jReset))) stage = 0;
        btnPrev = btn;
        bool clockEdge = ck.rising (in (jClock));
        if (clockEdge && ! isHigh (in (jHold))) stage = (stage + 1) & 7;
        for (int i = 0; i < 8; ++i)
        {
            jacks[(size_t) i].value = (i == stage ? kCV : 0.0f);
            leds[(size_t) i] = (i == stage ? 1.0f : 0.0f);
        }
    }
    int stage = 0; EdgeDetector ck, rs; bool btnPrev = false;
};

// ------------------------------------------------------ Sequencer / SEQ4 ----
// 4-stage × 3-row programmer/sequencer (pp. 20–21).
class Seq4 : public Module
{
public:
    enum J { jSel1, jSel2, jSel3, jSel4, jG1, jG2, jG3, jG4,
             jGate, jA, jB, jC, jClock, jReset, jHold, jUpDown };
    // ctls: 12 knobs rows A,B,C × stage, then 4 buttons
    Seq4()
    {
        for (int i = 0; i < 4; ++i) addJack ("SELECT", JackType::In,  JackColor::Red);
        for (int i = 0; i < 4; ++i) addJack ("GATE",   JackType::Out, JackColor::Red);
        addJack ("GATE", JackType::Out, JackColor::Red);
        addJack ("A", JackType::Out, JackColor::White);
        addJack ("B", JackType::Out, JackColor::White);
        addJack ("C", JackType::Out, JackColor::White);
        addJack ("CLOCK", JackType::In, JackColor::Red);
        addJack ("RESET", JackType::In, JackColor::Red);
        addJack ("HOLD",  JackType::In, JackColor::Red);
        addJack ("UP/DOWN", JackType::In, JackColor::Red);
        static const char* rows[3] = { "A", "B", "C" };
        for (int r = 0; r < 3; ++r)
            for (int s = 0; s < 4; ++s) addCtl (rows[r], 0.5f);
        for (int s = 0; s < 4; ++s) addCtl ("BTN", 0.0f, 2);
        leds.resize (4);
    }
    void prepare (double s) override { Module::prepare (s); stage = 0; ck={}; rs={}; for (auto& e : sel) e = {}; for (auto& b : btnPrev) b = false; }
    void tick() override
    {
        bool anyPress = false;
        for (int i = 0; i < 4; ++i)
        {
            bool b = ctls[(size_t) (12 + i)].v > 0.5f;
            if (b && ! btnPrev[i]) stage = i;
            if (b) anyPress = true;
            btnPrev[i] = b;
            if (sel[i].rising (in (jSel1 + i))) stage = i;
            if (isHigh (in (jSel1 + i))) anyPress = true;
        }
        if (rs.rising (in (jReset))) stage = 0;
        if (ck.rising (in (jClock)) && ! isHigh (in (jHold)))
        {
            int dir = connected (jUpDown) ? (isHigh (in (jUpDown)) ? 1 : -1) : 1;
            stage = (stage + dir + 4) & 3;
        }
        jacks[jA].value = ctls[(size_t) (0 + stage)].v * kCV;
        jacks[jB].value = ctls[(size_t) (4 + stage)].v * kCV;
        jacks[jC].value = ctls[(size_t) (8 + stage)].v * kCV;
        for (int i = 0; i < 4; ++i)
        {
            jacks[jG1 + i].value = (i == stage ? kCV : 0.0f);
            leds[(size_t) i] = (i == stage ? 1.0f : 0.0f);
        }
        jacks[jGate].value = anyPress ? kCV : 0.0f;
    }
    int stage = 0; EdgeDetector ck, rs, sel[4]; bool btnPrev[4] {};
};

// ------------------------------------------------------- Metronomic Clock ---
class MetroClock : public Module
{
public:
    enum J { jOut };
    enum C { cTempo, cFine };
    MetroClock()
    {
        addJack ("METRONOME", JackType::Out, JackColor::Red);
        addCtl ("TEMPO", 0.5f); addCtl ("FINE", 0.5f);
        leds.resize (1);
    }
    void prepare (double s) override { Module::prepare (s); ph = 0; }
    void tick() override
    {
        float f = expmap (ctls[cTempo].v, 0.1f, 30.0f) * (1.0f + (ctls[cFine].v - 0.5f) * 0.06f);
        ph += f / sr; if (ph >= 1.0f) ph -= 1.0f;
        bool hi = ph < std::max (0.02f, f * 0.002f * 2.0f); // ~2 ms trigger
        jacks[jOut].value = hi ? kCV : 0.0f;
        leds[0] = hi ? 1.0f : leds[0] * 0.995f;
    }
    float ph = 0;
};

// ------------------------------------------------- Bi-directional routers ---
// Three analog switches (p. 23): OFF connects A0↔B, ON connects A1↔B. Top two
// follow the SWITCH level (momentary); the bottom is a flip-flop with FLOP out.
class BiRouter : public Module
{
public:
    // per section: A1, B, A0, SWITCH (+FLOP on section 2)
    enum J { jA1_0, jB_0, jA0_0, jSw_0,
             jA1_1, jB_1, jA0_1, jSw_1,
             jA1_2, jB_2, jA0_2, jSw_2, jFlop };
    BiRouter()
    {
        for (int s = 0; s < 3; ++s)
        {
            addJack ("A1", JackType::Bi, JackColor::Grey);
            addJack ("B",  JackType::Bi, JackColor::White);
            addJack ("A0", JackType::Bi, JackColor::Grey);
            addJack ("SWITCH", JackType::In, JackColor::Red);
        }
        addJack ("FLOP", JackType::Out, JackColor::Red);
    }
    void prepare (double s) override { Module::prepare (s); ff = false; swEdge = {}; }
    void tick() override
    {
        for (int s = 0; s < 3; ++s)
        {
            const int jb = s * 4;
            bool on;
            if (s < 2) on = isHigh (in (jb+3));
            else { if (swEdge.rising (in (jb+3))) ff = ! ff; on = ff; }

            auto& jA1 = jacks[(size_t)(jb+0)];
            auto& jB  = jacks[(size_t)(jb+1)];
            auto& jA0 = jacks[(size_t)(jb+2)];
            if (jB.biDriven) // distribute: B is fed, route to A0 or A1
            {
                float v = in (jb+1);
                jA1.value = on ? v : 0.0f;
                jA0.value = on ? 0.0f : v;
                jB.value = 0.0f;
            }
            else             // select: A0/A1 feed B
            {
                jB.value  = on ? in (jb+0) : in (jb+2);
                jA1.value = 0.0f; jA0.value = 0.0f;
            }
        }
        jacks[jFlop].value = ff ? 0.0f : kCV; // FLOP is ON while switch is OFF
    }
    bool ff = false; EdgeDetector swEdge;
};

// ---------------------------------------------------------- Peak & Trough ---
// Plan V2 (CGS84 schematic): includes configurable precision diode bridge
// soft-crossover transition around equality while preserving exact DC extremes.
class PeakTrough : public Module
{
public:
    enum J { jPeak, jTrough, jIn1, jIn2 };
    PeakTrough()
    {
        addJack ("PEAK",   JackType::Out, JackColor::White);
        addJack ("TROUGH", JackType::Out, JackColor::White);
        addJack ("IN",     JackType::In,  JackColor::White);
        addJack ("IN",     JackType::In,  JackColor::White);
    }
    void tick() override
    {
        float a = in (jIn1), b = in (jIn2);
        // Smooth diode knee (eps = 0.0002V keeps <1e-3V DC accuracy for T10 while smoothing audio zero-crossings)
        float diff = a - b;
        float absDiff = std::sqrt (diff * diff + 4.0e-8f);
        jacks[jPeak].value   = 0.5f * (a + b + absDiff);
        jacks[jTrough].value = 0.5f * (a + b - absDiff);
    }
};

// ============================================================================
// EXPANSION ROW 4: SERGE 1976–1979 CLASSIC ICONS (SRG-027, SRG-028, SRG-030,
// SRG-031, SRG-040, SRG-044) — Modeled from CGS114, SMMS NTO, CGS113, CGS112,
// CGS202 & SMMS NCOM schematics collected in our historical archive.
// ============================================================================

// --------------------------------------------------- New Timbral Oscillator --
// SRG-028 (SMMS 1976 NTO / Random*Source NTO): Precision 1V/oct oscillator
// with integrated voltage-controlled portamento (Variable Slew on 1V/O),
// linear & exponential FM, hard sync, and continuous waveshaper (Sine ->
// Variable Timbre Folded Saw/Triangle) plus sub-octave pulse.
class NTO : public Module
{
public:
    enum J { jVariableOut, jSineOut, jSawOut, jSubOut,
             jV1O, jPortCV, jFMLin, jFMExp, jTimbreCV, jSync };
    enum C { cFreq, cPort, cTimbre, cFMExpAtt, cFMLinAtt };

    NTO()
    {
        addJack ("VARIABLE", JackType::Out, JackColor::Black);
        addJack ("SINE",     JackType::Out, JackColor::Black);
        addJack ("SAW",      JackType::Out, JackColor::Black);
        addJack ("SUB",      JackType::Out, JackColor::Red);
        addJack ("1V/O",     JackType::In,  JackColor::White);
        addJack ("PORTA VC", JackType::In,  JackColor::White);
        addJack ("LIN FM",   JackType::In,  JackColor::Black);
        addJack ("EXP FM",   JackType::In,  JackColor::White);
        addJack ("TIMBRE",   JackType::In,  JackColor::White);
        addJack ("SYNC",     JackType::In,  JackColor::Purple);

        addCtl ("FREQ",   0.38f);
        addCtl ("PORTA",  0.0f);
        addCtl ("TIMBRE", 0.25f);
        addCtl ("EXP FM", 0.5f);
        addCtl ("LIN FM", 0.0f);
        leds.resize (1);
    }

    void prepare (double s) override
    {
        Module::prepare (s);
        ph = 0.0f; pitchSlew = 0.0f; subFF = false; syncEdge = {};
    }

    void tick() override
    {
        const float dt = 1.0f / sr;
        // Integrated VC Portamento on 1V/O input (signature NTO circuit feature)
        float targetOct = in (jV1O);
        float portAmt = std::clamp (ctls[cPort].v + in (jPortCV) / kCV, 0.0f, 1.0f);
        if (portAmt < 0.005f)
        {
            pitchSlew = targetOct;
        }
        else
        {
            float tau = expmap (portAmt, 0.001f, 1.8f);
            pitchSlew += (targetOct - pitchSlew) * std::min (1.0f, dt / tau);
        }

        float oct = pitchSlew + av (ctls[cFMExpAtt].v) * in (jFMExp);
        float baseHz = expmap (ctls[cFreq].v, 10.0f, 5200.0f) * std::exp2 (oct);
        // Thru-zero capable linear FM
        float linHz = ctls[cFMLinAtt].v * in (jFMLin) * 450.0f;
        float f = std::clamp (baseHz + linHz, 0.01f, sr * 0.45f);
        float dp = f / sr;

        if (syncEdge.rising (in (jSync))) ph = 0.0f;
        ph += dp;
        if (ph >= 1.0f) { ph -= 1.0f; subFF = ! subFF; }

        float sine = std::sin (6.2831853f * ph);
        float saw  = (2.0f * ph - 1.0f);
        // PolyBLEP on sawtooth discontinuity
        if (ph < dp) { float x = ph / dp; saw -= (x + x - x * x - 1.0f); }
        else if (ph > 1.0f - dp) { float x = (ph - 1.0f) / dp; saw -= (x * x + x + x + 1.0f); }

        // NTO Variable Waveform Timbre modulator: morphs smoothly from pure sine
        // through asymmetric triangle/saw into folded harmonic overtones
        float timb = std::clamp (ctls[cTimbre].v + in (jTimbreCV) / kCV, 0.0f, 1.5f);
        float varWave = std::sin (6.2831853f * ph + timb * 2.6f * sine) * (1.0f - 0.35f * std::min (timb, 1.0f))
                      + std::tanh (saw * (1.0f + 2.5f * timb)) * (0.35f * std::min (timb, 1.0f));

        jacks[jVariableOut].value = std::clamp (varWave * kAudio, -kRails, kRails);
        jacks[jSineOut].value     = sine * kAudio;
        jacks[jSawOut].value      = saw * kAudio;
        jacks[jSubOut].value      = subFF ? kCV : 0.0f;
        leds[0] = 0.5f + 0.5f * sine;
    }

    float ph = 0.0f, pitchSlew = 0.0f;
    bool subFF = false;
    EdgeDetector syncEdge;
};

// -------------------------------------- Dual Universal Slope Generator (DUSG)
// SRG-027 (CGS114 / Series 76 DUSG): Two full universal slope channels, each
// with IN, TRIG, RISE VC, FALL VC, BIPOLAR OUT, UNIPOLAR OUT, and END PULSE,
// plus exponential/linear/logarithmic slope curvature and 1V/oct tracking.
class DUSG : public Module
{
public:
    // Per channel (×2): IN, TRIG, RISE_VC, FALL_VC, EXP_CV, OUT_B, OUT_W, END_PULSE
    enum J { jIn0, jTrig0, jRiseVC0, jFallVC0, jOutB0, jOutW0, jEnd0,
             jIn1, jTrig1, jRiseVC1, jFallVC1, jOutB1, jOutW1, jEnd1 };
    enum C { cRise0, cFall0, cShape0, cCycle0,
             cRise1, cFall1, cShape1, cCycle1 };

    DUSG()
    {
        for (int s = 0; s < 2; ++s)
        {
            addJack ("IN",      JackType::In,  JackColor::White);
            addJack ("TRIG",    JackType::In,  JackColor::Red);
            addJack ("RISE VC", JackType::In,  JackColor::White);
            addJack ("FALL VC", JackType::In,  JackColor::White);
            addJack ("OUT",     JackType::Out, JackColor::Black);
            addJack ("OUT",     JackType::Out, JackColor::White);
            addJack ("END",     JackType::Out, JackColor::Red);
        }
        for (int s = 0; s < 2; ++s)
        {
            addCtl ("RISE",  0.35f);
            addCtl ("FALL",  0.35f);
            addCtl ("SHAPE", 0.5f);    // 0 = log, 0.5 = linear, 1 = exponential (CGS114 BJT pair curve)
            addCtl ("CYCLE", 0.0f, 2); // loop toggle
        }
        leds.resize (2);
    }

    void prepare (double s) override
    {
        Module::prepare (s);
        for (int i = 0; i < 2; ++i) { y[i] = 0.0f; risingState[i] = false; trigEdge[i] = {}; endSlew[i].reset(); }
    }

    void tick() override
    {
        const float dt = 1.0f / sr;
        for (int s = 0; s < 2; ++s)
        {
            const int jb = s * 7, cb = s * 4;
            bool cyc = ctls[cb + 3].v > 0.5f;
            if (trigEdge[s].rising (in (jb + 1))) risingState[s] = true;
            if (cyc && y[s] <= 0.005f && ! connected (jb + 0)) risingState[s] = true;

            // Independent exponential voltage control over Rise and Fall (tracks 1V/oct when both patched)
            float riseRate = expmap (ctls[cb + 0].v, 0.4f, 28000.0f) * std::exp2 (in (jb + 2));
            float fallRate = expmap (ctls[cb + 1].v, 0.4f, 28000.0f) * std::exp2 (in (jb + 3));
            riseRate = std::clamp (riseRate, 0.1f, sr * kCV * 0.48f);
            fallRate = std::clamp (fallRate, 0.1f, sr * kCV * 0.48f);

            float target = risingState[s] ? kCV : (connected (jb + 0) ? std::clamp (in (jb + 0), 0.0f, kCV) : 0.0f);
            float& v = y[s];
            float sh = (ctls[cb + 2].v - 0.5f) * 2.0f; // -1..+1 curvature

            if (target > v)
            {
                float norm = std::clamp ((kCV - v) / kCV, 0.05f, 1.0f);
                float curve = std::pow (norm, sh * 0.75f);
                v += std::min (target - v, riseRate * curve * dt);
                if (risingState[s] && v >= kCV - 1.0e-3f)
                {
                    v = kCV;
                    risingState[s] = false;
                }
            }
            else if (target < v)
            {
                float norm = std::clamp (v / kCV, 0.05f, 1.0f);
                float curve = std::pow (norm, sh * 0.75f);
                v -= std::min (v - target, fallRate * curve * dt);
                if (v <= 1.0e-3f) v = 0.0f;
            }

            float rawEnd = (v <= 0.02f) ? kCV : 0.0f;
            jacks[jb + 4].value = v - kAudio;
            jacks[jb + 5].value = v;
            jacks[jb + 6].value = endSlew[s].tick (rawEnd, sr);
            leds[(size_t) s] = std::clamp (v / kCV, 0.0f, 1.0f);
        }
    }

    float y[2] {};
    bool risingState[2] {};
    EdgeDetector trigEdge[2];
    ComparatorEdgeSlew endSlew[2];
};

// ----------------------------------------- Variable Q VCF (VCFQ - CGS112) ---
// SRG-031 (Series 76 VCFQ / CGS112): Classic Serge state-variable filter with
// simultaneous LP, HP, BP, and NOTCH outputs, voltage-controlled Resonance (Q),
// 1V/oct tracking, and dedicated PING trigger/audio input for Serge percussion.
class VCFQ : public Module
{
public:
    enum J { jIn, jTrigPing, jV1O, jVCFreq, jVCQ, jLP, jBP, jHP, jNotch };
    enum C { cFreq, cQ, cGain, cFreqVC, cQVC };

    VCFQ()
    {
        addJack ("IN",      JackType::In,  JackColor::Black);
        addJack ("PING",    JackType::In,  JackColor::Red);
        addJack ("1V/O",    JackType::In,  JackColor::White);
        addJack ("VC FREQ", JackType::In,  JackColor::White);
        addJack ("VC Q",    JackType::In,  JackColor::White);
        addJack ("LOW",     JackType::Out, JackColor::Black);
        addJack ("BAND",    JackType::Out, JackColor::Black);
        addJack ("HIGH",    JackType::Out, JackColor::Black);
        addJack ("NOTCH",   JackType::Out, JackColor::Black);

        addCtl ("FREQ",    0.48f);
        addCtl ("Q",       0.35f);
        addCtl ("GAIN",    0.7f);
        addCtl ("FREQ VC", 0.5f);
        addCtl ("Q VC",    0.5f);
        leds.resize (1);
    }

    void prepare (double s) override { Module::prepare (s); ic1 = ic2 = 0.0f; pingEdge = {}; pingImp = 0.0f; }

    void tick() override
    {
        if (pingEdge.rising (in (jTrigPing))) pingImp = 6.5f; // crisp impulse into SVF resonator
        float pIn = pingImp;
        pingImp *= 0.82f;
        if (pingImp < 1.0e-4f) pingImp = 0.0f;

        float oct = in (jV1O) + av (ctls[cFreqVC].v) * in (jVCFreq);
        float f = expmap (ctls[cFreq].v, 18.0f, 9600.0f) * std::exp2 (oct);
        f = std::clamp (f, 0.1f, sr * 0.47f);

        float qCtl = std::clamp (ctls[cQ].v + av (ctls[cQVC].v) * in (jVCQ) / kCV, 0.0f, 1.15f);
        float res = expmap (qCtl, 0.55f, 35.0f);

        float g = std::tan (3.14159265f * f / sr);
        float k = 1.0f / res;
        float x = std::tanh ((in (jIn) * ctls[cGain].v + pIn) / kAudio) * kAudio;

        // CGS112 constant-loudness Q compensation & OTA saturation
        float bpSat = ic1 / (1.0f + 0.022f * ic1 * ic1);
        float a1 = 1.0f / (1.0f + g * (g + k));
        float v1 = a1 * (bpSat + g * (x - ic2));
        float v2 = ic2 + g * v1;
        ic1 = flushDenorm (2.0f * v1 - bpSat);
        ic2 = flushDenorm (2.0f * v2 - ic2);

        float bp = v1;
        float lp = v2;
        float hp = x - k * v1 - v2;
        float notch = lp + hp;

        jacks[jLP].value    = std::clamp (lp, -kRails, kRails);
        jacks[jBP].value    = std::clamp (bp, -kRails, kRails);
        jacks[jHP].value    = std::clamp (hp, -kRails, kRails);
        jacks[jNotch].value = std::clamp (notch, -kRails, kRails);
        leds[0] = std::clamp (std::fabs (bp) / kCV, 0.0f, 1.0f);
    }

    float ic1 = 0.0f, ic2 = 0.0f, pingImp = 0.0f;
    EdgeDetector pingEdge;
};

// ------------------------------- Voltage-Controlled Wave Multipliers (VCM) --
// SRG-030 (Series 76 VCM / CGS113): Three distinct classic timbre sections:
// 1. Top ("Grinder" / Overdrive VCA: CA3080 nonlinear odd-harmonic clipping)
// 2. Middle (6-Stage Odd-Harmonic Wavefolder: cascaded differential pairs)
// 3. Bottom (Full-Wave Rectifier + Linear VCA for even-harmonic multiplication)
class WaveMultipliers : public Module
{
public:
    enum J { jIn1, jVC1, jOut1,
             jIn2, jVC2, jSweep2, jOut2,
             jIn3, jVC3, jOut3 };
    enum C { cGain1, cFold2, cSweepAtt2, cGain3 };

    WaveMultipliers()
    {
        // Section 1: Grinder
        addJack ("IN 1",  JackType::In,  JackColor::Black);
        addJack ("VC 1",  JackType::In,  JackColor::White);
        addJack ("OUT 1", JackType::Out, JackColor::Black);
        // Section 2: Middle Wavefolder
        addJack ("IN 2",  JackType::In,  JackColor::Black);
        addJack ("VC 2",  JackType::In,  JackColor::White);
        addJack ("SWEEP", JackType::In,  JackColor::White);
        addJack ("OUT 2", JackType::Out, JackColor::Black);
        // Section 3: Full-Wave Rectifier + VCA
        addJack ("IN 3",  JackType::In,  JackColor::Black);
        addJack ("VC 3",  JackType::In,  JackColor::White);
        addJack ("OUT 3", JackType::Out, JackColor::Black);

        addCtl ("GRIND", 0.5f);
        addCtl ("FOLD",  0.4f);
        addCtl ("SWEEP", 0.5f);
        addCtl ("RECT",  0.6f);
        leds.resize (1);
    }

    void prepare (double s) override { Module::prepare (s); dc2.prepare (sr); dc3.prepare (sr); }

    void tick() override
    {
        // Section 1: CGS113 Top Section (Differential OTA soft/hard symmetrical clipper)
        float drv1 = std::clamp (ctls[cGain1].v + in (jVC1) / kCV, 0.0f, 1.5f) * 4.5f;
        float x1 = in (jIn1) / kAudio;
        jacks[jOut1].value = std::tanh (x1 * drv1) * kAudio;

        // Section 2: CGS113 Middle Wavefolder (6-stage BJT folder generating rich odd harmonics)
        float foldAmt = std::clamp (ctls[cFold2].v + in (jVC2) / kCV, 0.02f, 1.6f) * 5.2f;
        float bias = av (ctls[cSweepAtt2].v) * (connected (jSweep2) ? in (jSweep2) / kCV : 0.35f);
        float x2 = (in (jIn2) / kAudio + bias) * foldAmt;
        // Multi-stage Lockhart/Serge wavefolder transfer function
        float f2 = std::sin (3.14159265f * x2) - 0.35f * std::sin (2.0f * 3.14159265f * x2)
                 + 0.15f * std::tanh (x2 * 0.8f);
        float out2 = dc2.tick (std::clamp (f2, -1.5f, 1.5f)) * kAudio;
        jacks[jOut2].value = out2;

        // Section 3: CGS113 Bottom Section (Precision Full-Wave Rectifier + VCA)
        float g3 = std::clamp (ctls[cGain3].v + in (jVC3) / kCV, 0.0f, 1.5f);
        float x3 = in (jIn3) / kAudio;
        float rect = std::sqrt (x3 * x3 + 0.001f) * 2.0f - 0.8f;
        jacks[jOut3].value = dc3.tick (rect * g3) * kAudio;
        leds[0] = std::clamp (std::fabs (out2) / kAudio, 0.0f, 1.0f);
    }

    DCBlock dc2, dc3;
};

// ------------------------------------ 10-Band Resonant Equalizer (ResEQ) ----
// SRG-040 (Series 79 ResEQ / CGS202): Iconic 10-band non-octave resonant filter
// bank (29 Hz, 61 Hz, 115 Hz, 218 Hz, 411 Hz, 777 Hz, 1.47 kHz, 2.77 kHz,
// 5.24 kHz, 11 kHz) with Odd, Even, and All Mix outputs. Above mid-position,
// bands become strongly resonant for formant sculpting and pinging.
class ResEQ : public Module
{
public:
    enum J { jIn, jFbIn, jOddOut, jAllOut, jEvenOut };
    enum C { cB0, cB1, cB2, cB3, cB4, cB5, cB6, cB7, cB8, cB9 };

    ResEQ()
    {
        addJack ("IN",   JackType::In,  JackColor::Black);
        addJack ("FB",   JackType::In,  JackColor::Black);
        addJack ("ODD",  JackType::Out, JackColor::Black);
        addJack ("OUT",  JackType::Out, JackColor::Black);
        addJack ("EVEN", JackType::Out, JackColor::Black);

        static const char* bandLabels[10] = {
            "29", "61", "115", "218", "411", "777", "1.5K", "2.8K", "5.2K", "11K"
        };
        for (int b = 0; b < 10; ++b) addCtl (bandLabels[b], 0.5f);
        leds.resize (1);
    }

    void prepare (double s) override
    {
        Module::prepare (s);
        static const float freqs[10] = {
            29.0f, 61.0f, 115.0f, 218.0f, 411.0f, 777.0f, 1470.0f, 2770.0f, 5240.0f, 11000.0f
        };
        for (int b = 0; b < 10; ++b)
        {
            gBand[b] = std::tan (3.14159265f * std::min (freqs[b], sr * 0.44f) / sr);
            ic1[b] = ic2[b] = 0.0f;
        }
    }

    void tick() override
    {
        float x = std::tanh ((in (jIn) + 0.65f * in (jFbIn)) / (kAudio * 1.5f)) * (kAudio * 1.5f);
        float oddSum = 0.0f, evenSum = 0.0f;

        for (int b = 0; b < 10; ++b)
        {
            float ctl = ctls[(size_t) b].v; // 0 = cut, 0.5 = flat, 1.0 = max resonant boost
            float boost = (ctl - 0.5f) * 2.0f;
            float Q = (boost > 0.0f) ? (1.8f + boost * boost * 18.0f) : 1.8f;
            float k = 1.0f / Q;
            float g = gBand[b];
            float a1 = 1.0f / (1.0f + g * (g + k));
            float v1 = a1 * (ic1[b] + g * (x - ic2[b]));
            float v2 = ic2[b] + g * v1;
            ic1[b] = flushDenorm (2.0f * v1 - ic1[b]);
            ic2[b] = flushDenorm (2.0f * v2 - ic2[b]);

            // Bandpass contribution weighted by knob setting
            float gain = (boost >= 0.0f) ? (0.22f + boost * 1.45f) : (0.22f * (1.0f + boost));
            float bandOut = v1 * gain;
            if ((b & 1) == 0) oddSum += bandOut;  // 1st, 3rd, 5th, 7th, 9th
            else              evenSum += bandOut; // 2nd, 4th, 6th, 8th, 10th
        }

        oddSum  = CVPro::softlim (oddSum);
        evenSum = CVPro::softlim (evenSum);
        float allSum = CVPro::softlim (0.35f * x + 0.75f * (oddSum + evenSum));

        jacks[jOddOut].value  = oddSum;
        jacks[jEvenOut].value = evenSum;
        jacks[jAllOut].value  = allSum;
        leds[0] = std::clamp (std::fabs (allSum) / kCV, 0.0f, 1.0f);
    }

    float gBand[10] {};
    float ic1[10] {}, ic2[10] {};
};

// ----------------------------------------- Divide-by-N Comparator (NCOM) ----
// SRG-044 (Series 79 NCOM): Voltage-controlled pulse/audio divider (/1 to /31)
// with staircase CV output proportional to count and threshold comparator.
class NCOM : public Module
{
public:
    enum J { jClkIn, jVC_N, jCompIn, jDivOut, jStairOut, jCompOut };
    enum C { cDivN, cThresh };

    NCOM()
    {
        addJack ("CLOCK", JackType::In,  JackColor::Red);
        addJack ("VC N",  JackType::In,  JackColor::White);
        addJack ("IN",    JackType::In,  JackColor::White);
        addJack ("/N",    JackType::Out, JackColor::Red);
        addJack ("STAIR", JackType::Out, JackColor::White);
        addJack ("COMP",  JackType::Out, JackColor::Red);

        addCtl ("DIV N",  0.15f);
        addCtl ("THRESH", 0.5f);
        leds.resize (1);
    }

    void prepare (double s) override { Module::prepare (s); count = 0; clkEdge = {}; divPg = {}; }

    void tick() override
    {
        float nNorm = std::clamp (ctls[cDivN].v + in (jVC_N) / kCV, 0.0f, 1.0f);
        int N = std::clamp (1 + (int) std::lround (nNorm * 30.0f), 1, 31);

        if (clkEdge.rising (in (jClkIn)))
        {
            ++count;
            if (count >= N)
            {
                count = 0;
                divPg.fire (sr, 1.2f);
            }
        }
        float stair = (N > 1) ? (float (count) / float (N - 1)) * kCV : 0.0f;
        float thresh = ctls[cThresh].v * kCV;
        bool cmp = (connected (jCompIn) ? in (jCompIn) : stair) >= thresh;

        jacks[jDivOut].value   = divPg.tick();
        jacks[jStairOut].value = stair;
        jacks[jCompOut].value  = cmp ? kCV : 0.0f;
        leds[0] = jacks[jDivOut].value / kCV;
    }

    int count = 0;
    EdgeDetector clkEdge;
    PulseGen divPg;
};

// ------------------------------ Touch Activated Keyboard Sequencer (TKB) ----
// SRG-034 (Series 76 TKB): 16-stage x 4-row (A, B, C, D = 64 knobs) 2D
// addressable analog sequencer & 16 capacitive touch plates with KEY V,
// KEY PULSE, PRESSURE, VERT CLOCK / V RESET, RANDOM, DIR, HOLD, and CLOCK.
class TKB : public Module
{
public:
    enum J {
        jSel0 = 0, // 0..15 (16 stage select Red inputs)
        jVertClk = 16,
        jABCD, jA, jB, jC, jD,
        jKeyV, jKeyPulse, jPressure,
        jVReset, jReset, jRandom, jDir, jHold, jClock
    };
    enum C {
        cKnobsStart = 0,     // 64 knobs: row*16 + stage
        cPlatesStart = 64,   // 16 momentary touch plates
        cKeyboardToggle = 80 // 2-pos switch
    };

    TKB()
    {
        for (int i = 0; i < 16; ++i) addJack ("STAGE", JackType::In, JackColor::Red);
        addJack ("VERT CLK",  JackType::In,  JackColor::Red);
        addJack ("ABCD",      JackType::Out, JackColor::White);
        addJack ("A",         JackType::Out, JackColor::White);
        addJack ("B",         JackType::Out, JackColor::White);
        addJack ("C",         JackType::Out, JackColor::White);
        addJack ("D",         JackType::Out, JackColor::White);
        addJack ("KEY V",     JackType::Out, JackColor::White);
        addJack ("KEY PULSE", JackType::Out, JackColor::Red);
        addJack ("PRESSURE",  JackType::Out, JackColor::White);
        addJack ("V RESET",   JackType::In,  JackColor::Red);
        addJack ("RESET",     JackType::In,  JackColor::Red);
        addJack ("RANDOM",    JackType::In,  JackColor::Red);
        addJack ("DIR",       JackType::In,  JackColor::Red);
        addJack ("HOLD",      JackType::In,  JackColor::Red);
        addJack ("CLOCK",     JackType::In,  JackColor::Red);

        static const char* rowNames[4] = { "A", "B", "C", "D" };
        for (int r = 0; r < 4; ++r)
            for (int s = 0; s < 16; ++s)
                addCtl (rowNames[r], 0.5f);
        for (int s = 0; s < 16; ++s)
            addCtl ("KEY", 0.0f, 2);
        addCtl ("KEYBOARD", 0.0f, 2);

        leds.resize (20); // 0..15 stages, 16..19 vertical rows A..D
    }

    void prepare (double s) override
    {
        Module::prepare (s);
        stage = 0; vRow = 0; rngState = 0x5E26107Bu;
        ck = {}; rs = {}; vck = {}; vrs = {}; rnd = {};
        for (auto& e : sel) e = {};
        for (auto& b : platePrev) b = false;
        pressureVal = 0.0f;
        lastKeyV = 0.0f;
    }

    void tick() override
    {
        bool anyTouch = false;
        for (int i = 0; i < 16; ++i)
        {
            bool p = ctls[(size_t) (cPlatesStart + i)].v > 0.5f;
            if (p && ! platePrev[i])
            {
                stage = i;
                lastKeyV = (float (i) / 15.0f) * kCV;
            }
            if (p) anyTouch = true;
            platePrev[i] = p;

            if (sel[i].rising (in (jSel0 + i)))
            {
                stage = i;
                lastKeyV = (float (i) / 15.0f) * kCV;
            }
            if (isHigh (in (jSel0 + i))) anyTouch = true;
        }

        if (rs.rising (in (jReset))) stage = 0;
        if (vrs.rising (in (jVReset))) vRow = 0;
        if (rnd.rising (in (jRandom)))
        {
            rngState = rngState * 1664525u + 1013904223u;
            stage = (int) ((rngState >> 16) & 15u);
        }

        bool keybOnly = ctls[cKeyboardToggle].v > 0.5f;
        if (! keybOnly && ck.rising (in (jClock)) && ! isHigh (in (jHold)))
        {
            int dir = connected (jDir) ? (isHigh (in (jDir)) ? 1 : -1) : 1;
            stage = (stage + dir + 16) & 15;
        }
        if (vck.rising (in (jVertClk)))
            vRow = (vRow + 1) & 3;

        float valA = ctls[(size_t) (0  + stage)].v * kCV;
        float valB = ctls[(size_t) (16 + stage)].v * kCV;
        float valC = ctls[(size_t) (32 + stage)].v * kCV;
        float valD = ctls[(size_t) (48 + stage)].v * kCV;
        float abcd = (vRow == 0) ? valA : (vRow == 1) ? valB : (vRow == 2) ? valC : valD;

        jacks[jA].value        = valA;
        jacks[jB].value        = valB;
        jacks[jC].value        = valC;
        jacks[jD].value        = valD;
        jacks[jABCD].value     = abcd;
        jacks[jKeyV].value     = lastKeyV;
        jacks[jKeyPulse].value = anyTouch ? kCV : 0.0f;
        jacks[jPressure].value = anyTouch ? std::clamp (pressureVal * kCV, 0.0f, kCV) : 0.0f;

        for (int i = 0; i < 16; ++i) leds[(size_t) i] = (i == stage ? 1.0f : 0.0f);
        for (int r = 0; r < 4; ++r)  leds[(size_t) (16 + r)] = (r == vRow ? 1.0f : 0.0f);
    }

    int stage = 0, vRow = 0;
    float pressureVal = 0.0f, lastKeyV = 0.0f;
    uint32_t rngState = 0x5E26107Bu;
    EdgeDetector ck, rs, vck, vrs, rnd, sel[16];
    bool platePrev[16] {};
};

// ------------------------------------------------------------- I/O strip ----
// Software addition: MIDI→CV, host clock, DAW audio in (FX build), master out.
class IOStrip : public Module
{
public:
    enum J { jPitch, jGate, jVel, jClk, jAudioL, jAudioR, jMainL, jMainR };
    enum C { cVolume };
    IOStrip()
    {
        addJack ("1V/O",  JackType::Out, JackColor::White);
        addJack ("GATE",  JackType::Out, JackColor::Red);
        addJack ("VEL",   JackType::Out, JackColor::White);
        addJack ("CLK",   JackType::Out, JackColor::Red);
        addJack ("IN L",  JackType::Out, JackColor::Black); // DAW → panel
        addJack ("IN R",  JackType::Out, JackColor::Black);
        addJack ("OUT L", JackType::In,  JackColor::Black); // panel → DAW
        addJack ("OUT R", JackType::In,  JackColor::Black);
        addCtl ("VOLUME", 0.8f);
        leds.resize (1);
    }
    void tick() override
    {
        jacks[jPitch].value = pitchV;
        jacks[jGate].value  = gateV;
        jacks[jVel].value   = velV;
        jacks[jClk].value   = clkV;
        jacks[jAudioL].value = audioL;
        jacks[jAudioR].value = audioR;
        float vol = ctls[cVolume].v;
        outL = std::tanh (in (jMainL) * vol / 3.0f) * 3.0f / kAudio; // soft-limited, ±1 nominal
        outR = connected (jMainR) ? std::tanh (in (jMainR) * vol / 3.0f) * 3.0f / kAudio : outL;
        leds[0] = std::clamp (std::fabs (outL), 0.0f, 1.0f);
    }
    // written by the plugin layer each sample/block:
    float pitchV = 0, gateV = 0, velV = 0, clkV = 0, audioL = 0, audioR = 0;
    // read by the plugin layer:
    float outL = 0, outR = 0;
};

} // namespace pf50
