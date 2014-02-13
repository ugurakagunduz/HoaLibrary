/*
// Copyright (c) 2012-2014 Eliott Paris & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#include "Optim.h"

namespace Hoa3D
{
    Optim::Optim(unsigned int order, Mode mode) : Ambisonic(order)
    {
        m_harmonics = new double[m_number_of_harmonics];
        setMode(mode);
    }
    
    void Optim::setMode(Mode mode)
    {
        m_mode = mode;
        if(m_mode == Basic)
        {
            for(int i = 0; i < m_number_of_harmonics; i++)
            {
                m_harmonics[i] = 1.;
            }
        }
        else if (m_mode == MaxRe)
        {
            for(int i = 0; i < m_number_of_harmonics; i++)
            {
                m_harmonics[i] = cos(abs(getHarmonicArgument(i)) * CICM_PI / (2 * m_order + 2));;
            }
        }
        else
        {
            long double gain = ((m_order + 1) * (m_order + 1)) / (2 * m_order + 1);
            for(int i = 0; i < m_number_of_harmonics; i++)
            {
                m_harmonics[i] = (long double)((long double)Factorial(m_order)*(long double)Factorial(m_order)) / (long double)((long double)Factorial(m_order + getHarmonicBand(i)) * (long double)Factorial(m_order - fabs(getHarmonicArgument(i)))) * gain;
                //m_harmonics[i] = (long double)((long double)Factorial(m_order)*(long double)Factorial(m_order + 1)) / (long double)((long double)Factorial(m_order + getHarmonicBand(i) + 1) * (long double)Factorial(m_order - fabs(getHarmonicArgument(i)))) * gain; // Daniel mais pas notre 2D
            }
        }
    }
    
    void Optim::process(const float* inputs, float* outputs)
    {
        for(int i = 0; i < m_number_of_harmonics; i++)
            outputs[i] = inputs[i] * m_harmonics[i];
    }
    
    void Optim::process(const double* inputs, double* outputs)
    {
        for(int i = 0; i < m_number_of_harmonics; i++)
            outputs[i] = inputs[i] * m_harmonics[i];
    }
    
    Optim::~Optim()
    {
        delete [] m_harmonics;
    }
}

