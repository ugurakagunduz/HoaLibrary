/*
// Copyright (c) 2012-2014 Eliott Paris, Julien Colafrancesco & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#include "Meter_3D.h"

#define METER_ROW_NUMBER 181
#define METER_COLUMN_NUMBER 360

namespace Hoa3D
{
    Meter::Meter(unsigned int numberOfChannels) : Planewaves(numberOfChannels)
    {
        m_ramp                  = 0;
        m_vector_size           = 256;
        m_channels_peaks		= new double[m_number_of_channels];

		m_channels_number_of_points	= new unsigned int[m_number_of_channels];
		m_channels_points_azimuth	= new double*[m_number_of_channels];
		m_channels_points_elevation	= new double*[m_number_of_channels];

		m_channels_number_of_points_top	= new unsigned int[m_number_of_channels];
		m_channels_points_azimuth_top	= new double*[m_number_of_channels];
		m_channels_points_elevation_top	= new double*[m_number_of_channels];

		m_channels_number_of_points_bottom	= new unsigned int[m_number_of_channels];
		m_channels_points_azimuth_bottom	= new double*[m_number_of_channels];
		m_channels_points_elevation_bottom	= new double*[m_number_of_channels];

		m_channels_neighbors				= new int*[m_number_of_channels];
		for(unsigned int i = 0; i < m_number_of_channels; i++)
        {
            m_channels_peaks[i] = 0.;
			m_channels_neighbors[i]			= new int[m_number_of_channels];

			m_channels_number_of_points[i]	= 0;
			m_channels_points_azimuth[i]	= new double[m_number_of_channels];
			m_channels_points_elevation[i]	= new double[m_number_of_channels];

			m_channels_number_of_points_top[i]	= 0;
			m_channels_points_azimuth_top[i]	= new double[m_number_of_channels * 2];
			m_channels_points_elevation_top[i]	= new double[m_number_of_channels * 2];

			m_channels_number_of_points_bottom[i]	= 0;
			m_channels_points_azimuth_bottom[i]		= new double[m_number_of_channels * 2];
			m_channels_points_elevation_bottom[i]	= new double[m_number_of_channels * 2];
			
        }
		setChannelPosition(0, m_channels_azimuth[0], m_channels_elevation[0]);
    }
    
    void Meter::setVectorSize(unsigned int vectorSize)
    {
        m_vector_size   = vectorSize;
        m_ramp          = 0;
    }
    
	void Meter::setChannelsPosition(double* azimuths, double* elevations)
	{
		Planewaves::setChannelsPosition(azimuths, elevations);
		setChannelPosition(0, m_channels_azimuth[0], m_channels_elevation[0]);
	}

	void Meter::find_neighbor_channels()
	{
		int indices[8];
		double dist1, dist2, azi, ele;
		unsigned int numberOfRows = METER_ROW_NUMBER;
		unsigned int numberOfColumns = METER_COLUMN_NUMBER;
		int* sphere = new int[numberOfRows * numberOfColumns];
		unsigned int numberOfChannels = m_number_of_channels;
		double* azimuths = m_channels_azimuth;
		double* elevations = m_channels_elevation;
		int** channels_neighbours = m_channels_neighbors;
		
		// Fill a matrix that discretize a sphere with the indices of the closest loudspeakers
		for(unsigned int i = 0; i < numberOfRows; i++)
        {
            for(unsigned int j = 0; j < numberOfColumns; j++)
            {
                azi = (double)j / (double)numberOfColumns * HOA_2PI;
                ele = (double)(i + 0.5) * HOA_PI / (double)(numberOfRows) - HOA_PI2;
				dist1 = distance_spherical(azimuths[0], elevations[0], azi, ele);
                sphere[i * numberOfColumns + j] = 0;
                for(unsigned int k = 1; k < numberOfChannels; k++)
                {
                    dist2 = distance_spherical(azimuths[k], elevations[k], azi, ele);
                    if(dist2 <= dist1)
                    {
                        dist1 = dist2;
                        sphere[i * numberOfColumns + j] = k;
                    }
                }
            }
		}

		// Find the neighbor channels
		for(unsigned int i = 0; i < numberOfChannels; i++)
		{
			for(unsigned int j = 0; j < numberOfChannels; j++)
				channels_neighbours[i][j] = -1;
		}

		for(unsigned int i = 0; i < numberOfRows; i++)
        {
            for(unsigned int j = 0; j < numberOfColumns; j++)
            {
				int index_central = sphere[i * numberOfColumns + j];
				int j0 = j-1, j1 = j - 1, j2 = j -1, j3 = j, j4 = j + 1, j5 = j + 1, j6 = j + 1, j7 = j;
				int i0 = i + 1, i1 = i, i2 = i - 1, i3 = i - 1, i4 = i - 1, i5 = i, i6 = i + 1, i7 = i + 1;

				if(j == 0)
					j0 = j1 = j2 = numberOfColumns-1;
				else if(j == numberOfColumns - 1)
					j4 = j5 = j6 = 0;
				if(i == 0)
				{
					i2 = 0; i3 = 0; i4 = 0;
					j2 += numberOfColumns * 0.5;
					j3 += numberOfColumns * 0.5;
					j4 += numberOfColumns * 0.5;
					if(j2 > numberOfColumns - 1)
						j2 -= numberOfColumns;
					if(j3 > numberOfColumns - 1)
						j3 -= numberOfColumns;
					if(j4 > numberOfColumns - 1)
						j4 -= numberOfColumns;
					//post("j2 %i, j3 %i , j4 %i", j2, j3, j4);
				}
				else if(i == numberOfRows - 1)
				{
					i0 = i; i6 = i; i7 = i;
					j0 += numberOfColumns * 0.5;
					j6 += numberOfColumns * 0.5;
					j7 += numberOfColumns * 0.5;
					if(j0 > numberOfColumns - 1)
						j0 -= numberOfColumns;
					if(j6 > numberOfColumns - 1)
						j6 -= numberOfColumns;
					if(j7 > numberOfColumns - 1)
						j7 -= numberOfColumns;
					//post("j1 %i, j6 %i , j7 %i", j1, j6, j7);
				}
				
				indices[0] = sphere[i0 * numberOfColumns + j0];
				indices[1] = sphere[i1 * numberOfColumns + j1];
				indices[2] = sphere[i2 * numberOfColumns + j2];
				indices[3] = sphere[i3 * numberOfColumns + j3];
				indices[4] = sphere[i4 * numberOfColumns + j4];
				indices[5] = sphere[i5 * numberOfColumns + j5];
				indices[6] = sphere[i6 * numberOfColumns + j6];
				indices[7] = sphere[i7 * numberOfColumns + j7];
				
				for(int k = 0; k < 8; k++)
				{
					if(index_central != indices[k])
					{
						for(int l = 0; l < numberOfChannels && channels_neighbours[index_central][l] != indices[k]; l++)
						{
							if(channels_neighbours[index_central][l] == -1)
							{
								channels_neighbours[index_central][l] = indices[k];
								break;
							}
						}

						for(int l = 0; l < numberOfChannels && channels_neighbours[indices[k]][l] != index_central; l++)
						{
							if(channels_neighbours[indices[k]][l] == -1)
							{
								channels_neighbours[indices[k]][l] = index_central;
								break;
							}
						}
					}
				}
			}
		}

		for(unsigned int i = 0; i < m_number_of_channels; i++)
		{
			m_channels_number_of_points[i] = 0;
			//post("channel %i :", i);
			for(int j = 0; j < m_number_of_channels && m_channels_neighbors[i][j] != -1; j++, m_channels_number_of_points[i]++)
			{
				//post("neighbors %i :", m_channels_neighbors[i][j]);
			}
		}

		delete [] sphere;
	}

	void vector_sort_coordinates(unsigned int size, double* azimuths, double* elevations, double azimuth)
	{
        double* temp1_a	= new double[size];
		double* temp2_a	= new double[size];
		double* temp1_e	= new double[size];
		double* temp2_e	= new double[size];
		int inc1 = 0;
		int inc2 = 0;
		for(unsigned int i = 0; i < size; i++)
		{
			if(wrap_twopi(azimuths[i] - azimuth) <= HOA_PI)
			{
				temp1_a[inc1] = azimuths[i];
				temp1_e[inc1] = elevations[i];
				inc1++;
			}
			else
			{
				temp2_a[inc2] = azimuths[i];
				temp2_e[inc2] = elevations[i];
				inc2++;
			}
		}
		vector_sort_byone(inc1, temp1_e, temp1_a);
		vector_sort_byone(inc2, temp2_e, temp2_a);

		for(unsigned int i = 0; i < size; i++)
		{
			if(i < inc1)
			{
				azimuths[i] = temp1_a[i];
				elevations[i] = temp1_e[i];
			}
			else
			{
				azimuths[i] = temp2_a[inc2-(i-inc1)-1];
				elevations[i] = temp2_e[inc2-(i-inc1)-1];
			}
		}

		delete [] temp1_a;
		delete [] temp1_e;
		delete [] temp2_a;
		delete [] temp2_e;
	}

    void Meter::setChannelPosition(unsigned int index, double azimuth, double elevation)
	{
        Planewaves::setChannelPosition(index, azimuth, elevation);
		find_neighbor_channels();

		for(unsigned int i = 0; i < m_number_of_channels; i++)
		{
			int npts = m_channels_number_of_points[i];
			for(int j = 0; j < npts; j++)
			{
				int index = m_channels_neighbors[i][j];
				m_channels_points_azimuth[i][j] = spherical_azimuth_interpolation(m_channels_azimuth[i], m_channels_elevation[i], 
					m_channels_azimuth[index], m_channels_elevation[index], 0.5);
				m_channels_points_elevation[i][j] = spherical_elevation_interpolation(m_channels_azimuth[i], m_channels_elevation[i], 
					m_channels_azimuth[index], m_channels_elevation[index], 0.5);
			}
			vector_sort_coordinates(m_channels_number_of_points[i], m_channels_points_azimuth[i], m_channels_points_elevation[i], m_channels_azimuth[i]);

			// Find the top and bottom pvue points
			bool top = 0, bottom = 0;
			for(int j = 0; j < npts; j++)
			{
				if(m_channels_points_elevation[i][j] > 0)
					top = 1;
				else if(m_channels_points_elevation[i][j] < 0)
					bottom = 1;
			}
			m_channels_number_of_points_top[i] = 0;
			m_channels_number_of_points_bottom[i] = 0;
			if(top)
			{
				for(int j = 0; j < npts; j++)
				{
					if(m_channels_points_elevation[i][j] >= 0)
					{
						m_channels_points_azimuth_top[i][m_channels_number_of_points_top[i]] = m_channels_points_azimuth[i][j];
						m_channels_points_elevation_top[i][m_channels_number_of_points_top[i]] = m_channels_points_elevation[i][j];
						m_channels_number_of_points_top[i]++;
						int next = j + 1;
						if(j == npts - 1)
							next = 0;
						if(m_channels_points_elevation[i][next] < 0)
						{
							double mu =  m_channels_points_elevation[i][j] / ( m_channels_points_elevation[i][j] - m_channels_points_elevation[i][next]);
							m_channels_points_azimuth_top[i][m_channels_number_of_points_top[i]] = spherical_azimuth_interpolation(
								m_channels_points_azimuth[i][j], m_channels_points_elevation[i][j], 
								m_channels_points_azimuth[i][next], m_channels_points_elevation[i][next], mu);
							m_channels_points_elevation_top[i][m_channels_number_of_points_top[i]] = spherical_elevation_interpolation(
								m_channels_points_azimuth[i][j], m_channels_points_elevation[i][j], 
								m_channels_points_azimuth[i][next], m_channels_points_elevation[i][next], mu);
							m_channels_number_of_points_top[i]++;
						}
					}
					else
					{
						int next = j + 1;
						if(j == npts - 1)
							next = 0;
						if(m_channels_points_elevation[i][next] > 0)
						{
							double mu =  m_channels_points_elevation[i][next] / ( m_channels_points_elevation[i][next] - m_channels_points_elevation[i][j]);
							m_channels_points_azimuth_top[i][m_channels_number_of_points_top[i]] = spherical_azimuth_interpolation(
								m_channels_points_azimuth[i][next], m_channels_points_elevation[i][next], 
								m_channels_points_azimuth[i][j], m_channels_points_elevation[i][j], mu);
							m_channels_points_elevation_top[i][m_channels_number_of_points_top[i]] = spherical_elevation_interpolation(
								m_channels_points_azimuth[i][next], m_channels_points_elevation[i][next], 
								m_channels_points_azimuth[i][j], m_channels_points_elevation[i][j], mu);
							m_channels_number_of_points_top[i]++;
						}
					}
				}
			}
			if(bottom)
			{
				/*
				m_channels_number_of_points_bottom[i] = 0;
				for(int j = 0; j < npts; j++)
				{
					if(m_channels_points_elevation[i][j] <= 0)
					{
						m_channels_points_azimuth_bottom[i][m_channels_number_of_points_bottom[i]] = m_channels_points_azimuth[i][j];
						m_channels_points_elevation_bottom[i][m_channels_number_of_points_bottom[i]] = m_channels_points_elevation[i][j];
						m_channels_number_of_points_bottom[i]++;
						int next = j + 1;
						if(j == npts - 1)
							next = 0;
						if(m_channels_points_elevation[i][next] > 0)
						{
							double mu =  m_channels_points_elevation[i][j] / ( m_channels_points_elevation[i][j] - m_channels_points_elevation[i][next]);
							m_channels_points_azimuth_bottom[i][m_channels_number_of_points_top[i]] = spherical_azimuth_interpolation(
								m_channels_points_azimuth[i][j], m_channels_points_elevation[i][j], 
								m_channels_points_azimuth[i][next], m_channels_points_elevation[i][next], mu);
							m_channels_points_elevation_bottom[i][m_channels_number_of_points_top[i]] = spherical_elevation_interpolation(
								m_channels_points_azimuth[i][j], m_channels_points_elevation[i][j], 
								m_channels_points_azimuth[i][next], m_channels_points_elevation[i][next], mu);
							m_channels_number_of_points_top[i]++;
						}
					}
					else
					{
						int next = j + 1;
						if(j == npts - 1)
							next = 0;
						if(m_channels_points_elevation[i][next] > 0)
						{
							double mu =  m_channels_points_elevation[i][next] / ( m_channels_points_elevation[i][next] - m_channels_points_elevation[i][j]);
							m_channels_points_azimuth_bottom[i][m_channels_number_of_points_top[i]] = spherical_azimuth_interpolation(
								m_channels_points_azimuth[i][next], m_channels_points_elevation[i][next], 
								m_channels_points_azimuth[i][j], m_channels_points_elevation[i][j], mu);
							m_channels_points_elevation_top[i][m_channels_number_of_points_top[i]] = spherical_elevation_interpolation(
								m_channels_points_azimuth[i][next], m_channels_points_elevation[i][next], 
								m_channels_points_azimuth[i][j], m_channels_points_elevation[i][j], mu);
							m_channels_number_of_points_bottom[i]++;
						}
					}
				}*/
			}

		}
	}
    
    void Meter::process(const float* inputs)
    {
        if(m_ramp++ == m_vector_size)
        {
            m_ramp = 0;
            for(unsigned int i = 0; i < m_number_of_channels; i++)
            {
                m_channels_peaks[i] = fabs(inputs[i]);
            }
        }
        else
        {
            for(unsigned int i = 0; i < m_number_of_channels; i++)
            {
                if(fabs(inputs[i]) > m_channels_peaks[i])
                    m_channels_peaks[i] = fabs(inputs[i]);
            }
        }
    }
    
    void Meter::process(const double* inputs)
    {
        if(m_ramp++ == m_vector_size)
        {
            m_ramp = 0;
            for(unsigned int i = 0; i < m_number_of_channels; i++)
            {
                m_channels_peaks[i] = fabs(inputs[i]);
            }
        }
        else
        {
            for(unsigned int i = 0; i < m_number_of_channels; i++)
            {
                if(fabs(inputs[i]) > m_channels_peaks[i])
                    m_channels_peaks[i] = fabs(inputs[i]);
            }
        }
    }
    
    Meter::~Meter()
    {
		for(unsigned int i = 0; i < m_number_of_channels; i++)
		{
			delete [] m_channels_points_azimuth[i];
			delete [] m_channels_points_elevation[i];
			delete [] m_channels_neighbors[i];
			delete [] m_channels_points_azimuth_top[i];
			delete [] m_channels_points_elevation_top[i];
			delete [] m_channels_points_azimuth_bottom[i];
			delete [] m_channels_points_elevation_bottom[i];
		}
		delete [] m_channels_neighbors;

		delete [] m_channels_number_of_points;
		delete [] m_channels_points_azimuth;
		delete [] m_channels_points_elevation;

		delete [] m_channels_number_of_points_top;
		delete [] m_channels_points_azimuth_top;
		delete [] m_channels_points_elevation_top;

		delete [] m_channels_number_of_points_bottom;
		delete [] m_channels_points_azimuth_bottom;
		delete [] m_channels_points_elevation_bottom;
    }
}

