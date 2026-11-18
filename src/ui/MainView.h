// SERGIO Modular — main panel view (2x2 landscape boat grid + I/O strip),
// Calabi Yau Cosmic Neon aesthetic, banana-cable interaction, and zoomable editor.
#pragma once
#include "ModulePanels.h"
#include "../plugin/PluginProcessor.h"
#include "BinaryData.h"

namespace pf50ui {

inline juce::Image loadCroppedAsset (const void* data, int size, bool stripWhiteBg = true)
{
    auto src = juce::ImageFileFormat::loadFrom (data, (size_t) size);
    if (! src.isValid()) return {};
    auto img = src.convertedToFormat (juce::Image::ARGB);
    if (stripWhiteBg)
    {
        for (int y = 0; y < img.getHeight(); ++y)
            for (int x = 0; x < img.getWidth(); ++x)
            {
                auto p = img.getPixelAt (x, y);
                // Key out pure white / near-white studio background while keeping pastel logo fills
                if (p.getRed() > 246 && p.getGreen() > 246 && p.getBlue() > 246)
                    img.setPixelAt (x, y, juce::Colours::transparentBlack);
            }
    }
    int minX = img.getWidth(), minY = img.getHeight(), maxX = -1, maxY = -1;
    for (int y = 0; y < img.getHeight(); ++y)
        for (int x = 0; x < img.getWidth(); ++x)
            if (img.getPixelAt (x, y).getAlpha() > 16)
            {
                if (x < minX) minX = x;
                if (y < minY) minY = y;
                if (x > maxX) maxX = x;
                if (y > maxY) maxY = y;
            }
    if (maxX < minX || maxY < minY) return img;
    return img.getClippedImage ({ minX, minY, maxX - minX + 1, maxY - minY + 1 });
}

constexpr float RAIL_IN  = 0.24f;  // side rails, inches
constexpr int   HEADER_H = 38;
constexpr int   BOAT_GAP_X = 22;
constexpr int   BOAT_GAP_Y = 22;
constexpr float IO_H_IN  = 1.24f;

inline juce::RangedAudioParameter& ModuleComp::ctlParam (int ctlIdx)
{
    auto* p = proc.apvts.getParameter (PF50Processor::paramID (mod, ctlIdx));
    jassert (p != nullptr);
    return *p;
}

inline ButtonW* ModuleComp::addButton (int ctlIdx, float x, float y, float r)
{
    auto* b = new ButtonW();
    auto* modPtr = &mod;
    auto* procPtr = &proc;
    b->onPress = [procPtr, modPtr, ctlIdx] (bool down) { procPtr->setMomentary (modPtr, ctlIdx, down); };
    buttons.add (b);
    addAndMakeVisible (b);
    b->setBounds (pxr (sq (x, y, r)));
    return b;
}

inline TouchPlateW* ModuleComp::addTouchPlate (int ctlIdx, int stageNum, float x, float y, float w, float h)
{
    auto* tp = new TouchPlateW();
    tp->stageNum = stageNum;
    auto* modPtr = &mod;
    auto* procPtr = &proc;
    tp->onTouch = [procPtr, modPtr, ctlIdx] (bool down, float pressure)
    {
        procPtr->setMomentary (modPtr, ctlIdx, down);
        if (auto* tkb = dynamic_cast<pf50::TKB*> (modPtr))
            tkb->pressureVal = pressure;
    };
    touchPlates.add (tp);
    addAndMakeVisible (tp);
    tp->setBounds (pxr ({ x - w * 0.5f, y - h * 0.5f, w, h }));
    return tp;
}

inline void buildIOStrip (ModuleComp& c, pf50::IOStrip& io, float wIn)
{
    juce::ignoreUnused (io);
    using IO = pf50::IOStrip;
    c.darkFace = true;
    const float y = 0.42f;
    float x = 0.65f;
    auto put = [&] (int jack, const juce::String& lab) { c.addJack (jack, x, y, lab); x += 0.85f; };
    put (IO::jPitch, "1V/O"); put (IO::jGate, "GATE"); put (IO::jVel, "VEL"); put (IO::jClk, "CLK");
    x += 0.45f;
    put (IO::jAudioL, "IN L"); put (IO::jAudioR, "IN R");
    x += 0.45f;
    put (IO::jMainL, "OUT L"); put (IO::jMainR, "OUT R");
    x += 0.35f;
    c.addKnob (IO::cVolume, x, y, true, false, "VOLUME");
    x += 0.85f;
    c.addLed (0, x, y);
    c.addLabel ("SERGIO  MODULAR  SYNTHESIZER   -   CALABI YAU", wIn - 4.4f, y - 0.08f, 8.0f, 0.16f);
}

// --------------------------------------------------------------------------
class PanelView : public juce::Component, public CableHost, private juce::Timer
{
public:
    explicit PanelView (PF50Processor& p) : proc (p)
    {
        hostPtr = this;

        auto widthOf = [] (pf50::Module* m) -> float
        {
            auto id = m->id;
            if (id == "vco") return 2; if (id == "vcoP") return 3;
            if (id == "ssgH" || id == "ssgP") return 3;
            if (id == "cvproH" || id == "cvproP") return 3;
            if (id == "tws") return 3; if (id == "vcfH" || id == "vcfP") return 2;
            if (id == "pos") return 3; if (id == "neg") return 2;
            if (id == "env") return 2; if (id == "gates") return 3;
            if (id == "gseq") return 2; if (id == "seq4a" || id == "seq4b") return 5;
            if (id == "clock") return 1; if (id == "router") return 2;
            if (id == "pt") return 1;
            if (id == "nto") return 2; if (id == "dusg") return 4;
            if (id == "vcfq") return 2; if (id == "vcm") return 3;
            if (id == "reseq") return 4; if (id == "ncom") return 2;
            if (id == "tkb") return 17;
            return 2;
        };

        const std::vector<std::vector<pf50::Module*>> boats = {
            // Row 0 Left: HELIOS (16")
            { proc.sys.vco, proc.sys.vcoP, proc.sys.ssgH, proc.sys.cvproH, proc.sys.tws, proc.sys.vcfH },
            // Row 0 Right: PARIS-FLASH (18")
            { proc.sys.pos, proc.sys.neg, proc.sys.ssgP, proc.sys.env, proc.sys.cvproP, proc.sys.gates, proc.sys.vcfP },
            // Row 1 Left: TEMPO DI ROMA (17")
            { proc.sys.gseq, proc.sys.seq4a, proc.sys.clock, proc.sys.router, proc.sys.seq4b, proc.sys.pt },
            // Row 1 Right: SERIES '76 / '79 EXPANSION (17")
            { proc.sys.nto, proc.sys.dusg, proc.sys.vcfq, proc.sys.vcm, proc.sys.reseq, proc.sys.ncom },
            // Row 2 Left: TOUCH ACTIVATED KEYBOARD SEQUENCER (17")
            { proc.sys.tkb }
        };
        rowNames = {
            "HELIOS",
            "PARIS-FLASH",
            "TEMPO DI ROMA",
            "SERIES '76 / '79 EXPANSION",
            "TKB  //  TOUCH ACTIVATED KEYBOARD SEQUENCER"
        };

        auto boatWidthPx = [&] (const std::vector<pf50::Module*>& row) -> int
        {
            float w = 2 * RAIL_IN;
            for (auto* m : row) w += widthOf (m);
            return (int) std::ceil (w * PX);
        };

        const int leftColW  = std::max ({ boatWidthPx (boats[0]), boatWidthPx (boats[2]), boatWidthPx (boats[4]) });
        const int rightColW = std::max (boatWidthPx (boats[1]), boatWidthPx (boats[3]));
        const int padX = 16;
        const int padY = 14;
        const int modHpx = (int) (MOD_H * PX);

        logicalW = padX * 2 + leftColW + BOAT_GAP_X + rightColW;
        const int colX[2] = { padX, padX + leftColW + BOAT_GAP_X };
        const int rowY[3] = { padY, padY + modHpx + BOAT_GAP_Y, padY + 2 * (modHpx + BOAT_GAP_Y) };

        for (size_t bi = 0; bi < boats.size(); ++bi)
        {
            int col = (int) (bi % 2);
            int row = (int) (bi / 2);
            int bw = boatWidthPx (boats[bi]);
            int colW = (col == 0) ? leftColW : rightColW;
            int bx = colX[col] + (colW - bw) / 2;
            int by = rowY[row];

            rowRects.push_back ({ bx, by, bw, modHpx });
            int mx = bx + (int) (RAIL_IN * PX);
            for (auto* m : boats[bi])
            {
                auto* mc = new ModuleComp (*m, widthOf (m), hostPtr, proc);
                buildModule (*mc);
                mods.add (mc);
                addAndMakeVisible (mc);
                mc->setTopLeftPosition (mx, by);
                mx += mc->getWidth();
            }
        }

        // Row 2 Right: I/O Strip + Calabi Yau Logo Showcase
        {
            float wIn = (float) rightColW / PX;
            auto* mc = new ModuleComp (*proc.sys.io, wIn, hostPtr, proc);
            buildIOStrip (*mc, *proc.sys.io, wIn);
            mc->setSize (rightColW, (int) (IO_H_IN * PX));
            mods.add (mc);
            addAndMakeVisible (mc);
            mc->setTopLeftPosition (colX[1], rowY[2] + modHpx - mc->getHeight());
            ioRect = mc->getBounds();
            logoArea = { colX[1], rowY[2], rightColW, modHpx - mc->getHeight() - 12 };
            logicalH = rowY[2] + modHpx + 22;
        }

        calabiIcon     = loadCroppedAsset (BinaryData::calabi_icon_png, BinaryData::calabi_icon_pngSize);
        calabiWordmark = loadCroppedAsset (BinaryData::calabi_wordmark_png, BinaryData::calabi_wordmark_pngSize);

        // Generate deterministic cosmic starfield
        uint32_t rng = 0x5E2610u;
        for (int i = 0; i < 160; ++i)
        {
            rng = rng * 1664525u + 1013904223u;
            float sx = (float) (rng % (uint32_t) logicalW);
            rng = rng * 1664525u + 1013904223u;
            float sy = (float) (rng % (uint32_t) logicalH);
            float r = (i % 7 == 0) ? 1.8f : 1.0f;
            stars.push_back ({ sx, sy, r, (i % 3 == 0) });
        }

        setSize (logicalW, logicalH);

        for (auto* mc : mods)
            for (auto* j : mc->jacks)
                allJacks.push_back (j);

        startTimerHz (30);
        setOpaque (true);
    }

    void paint (juce::Graphics& g) override
    {
        // Deep cosmic space gradient
        g.setGradientFill (juce::ColourGradient (
            juce::Colour (0xff070811), 0.0f, 0.0f,
            juce::Colour (0xff101326), 0.0f, (float) logicalH, false));
        g.fillAll();

        // Subtle stars
        for (auto& st : stars)
        {
            g.setColour ((st.cyan ? col::neonCyan : col::neonPink).withAlpha (0.28f));
            g.fillEllipse (st.x, st.y, st.r, st.r);
        }

        // Boat chassis + cosmic plum rails + neon cyan nameplates
        for (size_t i = 0; i < rowRects.size(); ++i)
        {
            auto rr = rowRects[i].toFloat();
            g.setColour (juce::Colour (0xff090a14));
            g.fillRect (rr.expanded (3.0f));
            g.setColour (col::neonCyan.withAlpha (0.38f));
            g.drawRect (rr.expanded (3.0f), 1.0f);

            g.setGradientFill (juce::ColourGradient (col::wood, rr.getX(), rr.getY(), col::woodDark, rr.getX(), rr.getBottom(), false));
            g.fillRect (juce::Rectangle<float> (rr.getX(), rr.getY(), RAIL_IN * PX, rr.getHeight()));
            g.fillRect (juce::Rectangle<float> (rr.getRight() - RAIL_IN * PX, rr.getY(), RAIL_IN * PX, rr.getHeight()));

            // boat nameplate on the lower rail edge
            g.setColour (juce::Colour (0xff090b16));
            g.fillRect (juce::Rectangle<float> (rr.getX() - 3, rr.getBottom() - 1, rr.getWidth() + 6, 16.0f));
            g.setColour (col::neonCyan.withAlpha (0.88f));
            g.setFont (panelFont (10.5f, true));
            g.drawText (rowNames[(int) i], (int) rr.getX() + 10, (int) rr.getBottom() - 1, 420, 15, juce::Justification::centredLeft);
        }

        // Calabi Yau Logo Showcase Panel (Row 2 Right, above I/O Strip)
        if (! logoArea.isEmpty())
        {
            auto la = logoArea.toFloat();
            g.setColour (juce::Colour (0xff0a0c18));
            g.fillRoundedRectangle (la, 6.0f);
            g.setColour (col::neonCyan.withAlpha (0.45f));
            g.drawRoundedRectangle (la, 6.0f, 1.2f);
            g.setColour (col::neonPink.withAlpha (0.25f));
            g.drawRoundedRectangle (la.reduced (5.0f), 4.0f, 0.8f);

            if (calabiIcon.isValid() && calabiWordmark.isValid())
            {
                auto iconBox = juce::Rectangle<float> (la.getX() + 34.0f, la.getCentreY() - 95.0f, 190.0f, 190.0f);
                g.drawImageWithin (calabiIcon, (int) iconBox.getX(), (int) iconBox.getY(),
                                   (int) iconBox.getWidth(), (int) iconBox.getHeight(),
                                   juce::RectanglePlacement::centred);

                auto wordBox = juce::Rectangle<float> (iconBox.getRight() + 28.0f, la.getCentreY() - 92.0f,
                                                       la.getRight() - iconBox.getRight() - 56.0f, 145.0f);
                g.drawImageWithin (calabiWordmark, (int) wordBox.getX(), (int) wordBox.getY(),
                                   (int) wordBox.getWidth(), (int) wordBox.getHeight(),
                                   juce::RectanglePlacement::centred);

                g.setColour (col::neonCyan.withAlpha (0.85f));
                g.setFont (panelFont (14.0f, true));
                g.drawText ("SERGIO  //  WEST COAST MODULAR SYNTHESIZER",
                            (int) wordBox.getX(), (int) wordBox.getBottom() + 4, (int) wordBox.getWidth(), 22,
                            juce::Justification::centred);
            }
        }
    }

    void paintOverChildren (juce::Graphics& g) override
    {
        auto cables = proc.cableSnapshot();
        int idx = 0;
        for (auto& c : cables)
        {
            auto* a = findJack (c.modA, c.jackA);
            auto* b = findJack (c.modB, c.jackB);
            if (a && b) drawCable (g, centreOf (a), centreOf (b), cableColour (idx));
            ++idx;
        }
        if (dragSrc != nullptr)
            drawCable (g, centreOf (dragSrc), dragPos, col::neonCyan.withAlpha (0.85f));
    }

    // ---- CableHost ----
    void jackDragStart (JackW* j) override { dragSrc = j; dragPos = centreOf (j); repaint(); }
    void jackDragMove (juce::Point<float> screen) override
    {
        dragPos = getLocalPoint (nullptr, screen);
        auto* over = jackAt (dragPos);
        for (auto* j : allJacks) j->dropTarget = (j == over && j != dragSrc);
        repaint();
    }
    void jackDragEnd (juce::Point<float> screen) override
    {
        auto pos = getLocalPoint (nullptr, screen);
        auto* dst = jackAt (pos);
        if (dragSrc && dst && dst != dragSrc)
            proc.addCable (proc.sys.engine.indexOf (&dragSrc->mod), dragSrc->jack,
                           proc.sys.engine.indexOf (&dst->mod), dst->jack);
        for (auto* j : allJacks) j->dropTarget = false;
        dragSrc = nullptr;
        repaint();
    }
    void jackRightClick (JackW* j) override
    {
        juce::PopupMenu menu;
        auto cables = proc.cableSnapshot();
        int mIdx = proc.sys.engine.indexOf (&j->mod);
        std::vector<pf50::Cable> mine;
        for (auto& c : cables)
            if ((c.modA == mIdx && c.jackA == j->jack) || (c.modB == mIdx && c.jackB == j->jack))
                mine.push_back (c);
        if (mine.empty()) return;
        int id = 1;
        for (auto& c : mine)
        {
            bool aIsThis = (c.modA == mIdx && c.jackA == j->jack);
            auto& other = aIsThis ? *proc.sys.engine.mods[(size_t) c.modB] : *proc.sys.engine.mods[(size_t) c.modA];
            int oj = aIsThis ? c.jackB : c.jackA;
            menu.addItem (id++, "Remove -> " + juce::String (other.id) + " " + other.jacks[(size_t) oj].name);
        }
        menu.addSeparator();
        menu.addItem (999, "Remove all cables here");
        menu.showMenuAsync (juce::PopupMenu::Options(), [this, mine, mIdx, j] (int result)
        {
            if (result == 999) proc.removeCablesAt (mIdx, j->jack);
            else if (result > 0 && result <= (int) mine.size()) proc.removeCable (mine[(size_t) result - 1]);
            repaint();
        });
    }

    int logicalW = 1860, logicalH = 850;

private:
    struct Star { float x, y, r; bool cyan; };
    std::vector<Star> stars;

    void timerCallback() override
    {
        for (auto* mc : mods)
            for (auto* l : mc->leds) l->refresh();
        int cv = proc.cableVersion();
        if (cv != lastCableVer) { lastCableVer = cv; repaint(); }
    }

    JackW* findJack (int modIdx, int jackIdx)
    {
        auto* m = proc.sys.engine.mods[(size_t) modIdx].get();
        for (auto* j : allJacks)
            if (&j->mod == m && j->jack == jackIdx) return j;
        return nullptr;
    }
    juce::Point<float> centreOf (JackW* j)
    {
        return getLocalPoint (j, j->getLocalBounds().getCentre().toFloat());
    }
    JackW* jackAt (juce::Point<float> p)
    {
        for (auto* j : allJacks)
        {
            auto b = getLocalArea (j, j->getLocalBounds().toFloat()).expanded (5.0f);
            if (b.contains (p)) return j;
        }
        return nullptr;
    }
    static juce::Colour cableColour (int i)
    {
        static const juce::Colour cols[] = {
            juce::Colour (0xffff2ea6), // neon magenta
            juce::Colour (0xff2ce6ff), // electric cyan
            juce::Colour (0xffffbe3b), // solar gold
            juce::Colour (0xffb967ff), // neon violet
            juce::Colour (0xff2effa8), // cosmic mint
            juce::Colour (0xffff6b6b)  // neon coral
        };
        return cols[i % 6];
    }
    static void drawCable (juce::Graphics& g, juce::Point<float> a, juce::Point<float> b, juce::Colour col)
    {
        float dist = a.getDistanceFrom (b);
        float sag = juce::jmin (140.0f, 20.0f + dist * 0.14f);
        juce::Point<float> m ((a.x + b.x) * 0.5f, juce::jmax (a.y, b.y) + sag);
        juce::Path p;
        p.startNewSubPath (a);
        p.quadraticTo (m, b);
        // neon outer glow
        g.setColour (col.withAlpha (0.22f));
        g.strokePath (p, juce::PathStrokeType (9.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (col.withAlpha (0.92f));
        g.strokePath (p, juce::PathStrokeType (4.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (juce::Colours::white.withAlpha (0.38f));
        g.strokePath (p, juce::PathStrokeType (1.3f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        for (auto pt : { a, b })
        {
            g.setColour (col.darker (0.4f));
            g.fillEllipse (juce::Rectangle<float> (13, 13).withCentre (pt));
            g.setColour (col);
            g.fillEllipse (juce::Rectangle<float> (8, 8).withCentre (pt));
        }
    }

    PF50Processor& proc;
    CableHost* hostPtr = nullptr;
    juce::OwnedArray<ModuleComp> mods;
    std::vector<JackW*> allJacks;
    std::vector<juce::Rectangle<int>> rowRects;
    juce::Rectangle<int> ioRect, logoArea;
    juce::Image calabiIcon, calabiWordmark;
    juce::StringArray rowNames;
    JackW* dragSrc = nullptr;
    juce::Point<float> dragPos;
    int lastCableVer = -1;
};

// --------------------------------------------------------------------------
class PF50Editor : public juce::AudioProcessorEditor
{
public:
    explicit PF50Editor (PF50Processor& p) : AudioProcessorEditor (p), proc (p), view (p)
    {
        zoomContainer.addAndMakeVisible (view);
        viewport.setViewedComponent (&zoomContainer, false);
        viewport.setScrollBarsShown (true, true);
        addAndMakeVisible (viewport);

        presets.addItemList (PF50Processor::factoryPatchNames(), 1);
        presets.setTextWhenNothingSelected ("PATCHES");
        presets.onChange = [this]
        {
            int i = presets.getSelectedItemIndex();
            if (i >= 0) proc.loadFactoryPatch (i);
        };
        addAndMakeVisible (presets);

        clearBtn.setButtonText ("CLEAR");
        clearBtn.onClick = [this] { proc.clearAllCables(); };
        addAndMakeVisible (clearBtn);

        zoomOutBtn.setButtonText ("-");
        zoomOutBtn.onClick = [this] { setZoom (zoom - 0.12f, false); };
        addAndMakeVisible (zoomOutBtn);

        zoomInBtn.setButtonText ("+");
        zoomInBtn.onClick = [this] { setZoom (zoom + 0.12f, false); };
        addAndMakeVisible (zoomInBtn);

        zoomFitBtn.setButtonText ("FIT");
        zoomFitBtn.onClick = [this] { fitToWindow(); };
        addAndMakeVisible (zoomFitBtn);

        zoom100Btn.setButtonText ("100%");
        zoom100Btn.onClick = [this] { setZoom (1.0f, false); };
        addAndMakeVisible (zoom100Btn);

        zoomLabel.setJustificationType (juce::Justification::centred);
        zoomLabel.setColour (juce::Label::textColourId, col::neonCyan);
        addAndMakeVisible (zoomLabel);

        headerIcon = loadCroppedAsset (BinaryData::calabi_icon_png, BinaryData::calabi_icon_pngSize);

        setResizable (true, true);
        setResizeLimits (760, 420, 3200, 1800);
        // Landscape default window size (wider than tall)
        setSize (1440, 820);
        fitToWindow();
    }

    void setZoom (float newZoom, bool fromFit)
    {
        fitMode = fromFit;
        zoom = juce::jlimit (0.40f, 2.00f, newZoom);
        view.setBounds (0, 0, view.logicalW, view.logicalH);
        view.setTransform (juce::AffineTransform::scale (zoom));
        zoomContainer.setSize ((int) std::ceil (view.logicalW * zoom), (int) std::ceil (view.logicalH * zoom));
        zoomLabel.setText (juce::String ((int) std::lround (zoom * 100.0f)) + "%", juce::dontSendNotification);
    }

    void fitToWindow()
    {
        float availW = (float) juce::jmax (100, getWidth() - 12);
        float availH = (float) juce::jmax (100, getHeight() - HEADER_H - 12);
        float s = juce::jmin (availW / (float) view.logicalW, availH / (float) view.logicalH);
        setZoom (s, true);
    }

    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& w) override
    {
        if (e.mods.isCommandDown() || e.mods.isCtrlDown())
        {
            setZoom (zoom + w.deltaY * 0.25f, false);
            return;
        }
        AudioProcessorEditor::mouseWheelMove (e, w);
    }

    void resized() override
    {
        const int w = getWidth();
        viewport.setBounds (0, HEADER_H, w, getHeight() - HEADER_H);

        int rx = w - 12;
        clearBtn.setBounds (rx - 74, 6, 74, HEADER_H - 12); rx -= 82;
        presets.setBounds  (rx - 220, 6, 220, HEADER_H - 12); rx -= 236;

        // Zoom toolbar
        zoomInBtn.setBounds  (rx - 28, 6, 28, HEADER_H - 12); rx -= 30;
        zoomLabel.setBounds  (rx - 46, 6, 46, HEADER_H - 12); rx -= 48;
        zoomOutBtn.setBounds (rx - 28, 6, 28, HEADER_H - 12); rx -= 34;
        zoom100Btn.setBounds (rx - 46, 6, 46, HEADER_H - 12); rx -= 50;
        zoomFitBtn.setBounds (rx - 40, 6, 40, HEADER_H - 12);

        if (fitMode) fitToWindow();
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (col::headerBg);
        auto hdr = juce::Rectangle<float> (0, 0, (float) getWidth(), (float) HEADER_H);
        g.setGradientFill (juce::ColourGradient (
            juce::Colour (0xff0c0e1c), 0, 0,
            juce::Colour (0xff15122b), (float) getWidth(), 0, false));
        g.fillRect (hdr);
        g.setColour (col::neonCyan.withAlpha (0.45f));
        g.drawLine (0.0f, (float) HEADER_H - 0.5f, (float) getWidth(), (float) HEADER_H - 0.5f, 1.0f);

        // Retro-futuristic gradient SERGIO title + CALABI YAU icon & badge
        if (headerIcon.isValid())
            g.drawImageWithin (headerIcon, 10, 4, HEADER_H - 8, HEADER_H - 8, juce::RectanglePlacement::centred);

        g.setGradientFill (juce::ColourGradient (
            juce::Colour (0xffffe2a8), 46.0f, 6.0f,
            col::neonPink, 46.0f, 32.0f, false));
        g.setFont (panelFont (22.0f, true));
        g.drawText ("SERGIO", 48, 2, 130, HEADER_H - 4, juce::Justification::centredLeft);

        g.setColour (col::neonCyan.withAlpha (0.9f));
        g.setFont (panelFont (11.0f, false));
        g.drawText ("MODULAR SYNTHESIZER  //  CALABI YAU", 160, 4, 340, HEADER_H - 6, juce::Justification::centredLeft);
    }

private:
    PF50Processor& proc;
    juce::Viewport viewport;
    juce::Component zoomContainer;
    PanelView view;
    juce::Image headerIcon;
    juce::ComboBox presets;
    juce::TextButton clearBtn, zoomInBtn, zoomOutBtn, zoomFitBtn, zoom100Btn;
    juce::Label zoomLabel;
    float zoom = 0.72f;
    bool fitMode = true;
};

} // namespace pf50ui

using PF50Editor = pf50ui::PF50Editor;
