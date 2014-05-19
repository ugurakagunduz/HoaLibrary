/*
// Copyright (c) 2012-2014 Eliott Paris, Julien Colafrancesco & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#ifndef __HOA_DECODER_PLUGIN__
#define __HOA_DECODER_PLUGIN__

#include <JuceHeader.h>
#include "../../../Sources/Hoa2D/Hoa2D.h"
#include "..\HoaQuadra\Source\PluginProcessor.h"

class HoaMeterComponent : public Component, public Timer
{
    
private:
	HoaToolsAudioProcessor* m_processor;
	AudioProcessorEditor*	m_master;
    Meter*                  m_meter;
    
    double                  m_center;
    double                  m_radius;
    double                  m_radius_center;
    
    void draw_background(Graphics& g);
    void draw_leds(Graphics& g);
    
public:
    HoaMeterComponent(AudioProcessorEditor* master, HoaToolsAudioProcessor* processor);
    ~HoaMeterComponent();
    
    void paint (Graphics&);
    void timerCallback();
};


#endif