#include "perfusionAlgorithm.h"

#include <math.h>
#include <iostream>


perfusionAlgorithm::perfusionAlgorithm(int numberOfDynamics)
{
	m_numberOfDynamics = numberOfDynamics;
	//calcPixels = dynamicSignals;
	perfusionMaps.SetSize(9);
}


perfusionAlgorithm::~perfusionAlgorithm()
{
}

void perfusionAlgorithm::GenerateMaps()
{
	//othervalues
	float RE = 0.0f;
	float ME = 0.0f;
	float MRE = 0.0f;
	float T0 = 0.0f;
	float TTP = 0.0f;
	float WIR = 0.0f;
	float WOR = 0.0f;
	float BOE = 0.0f;
	float Area = 0.0f;

	bool validPixel = false;
	float previousPixelValue = 0.0f;
	for (int j = 0; j < m_numberOfDynamics - 1 && !validPixel; j++)
	{
		if (calcPixels[j] != previousPixelValue)
		{
			validPixel = true;
		}
		previousPixelValue = calcPixels[j];
	}
	if (validPixel)
	{
		/*calculate Type Maps*/
		//Calculate S0 and T0
		int toIndex;
		float s0;
		CalculateT0S0(toIndex, s0);
		//std::cout << "to" << toIndex << std::endl;
		//Calculate PeakValue and Curve Area;
		int peakInex = toIndex;
		float peakValue = s0;
		float cureveArea = 0.0f;
		int k;

		for (k = toIndex; k < m_numberOfDynamics - 1; ++k)
		{
			if (calcPixels[k] > peakValue)
			{
				peakValue = calcPixels[k];
				peakInex = k;
			}
			if (k < (m_numberOfDynamics - 1))
			{
				float avgValue = (calcPixels[k] + calcPixels[k + 1]) / 2;
				float time = imageTimer.at(k + 1) - imageTimer.at(k);
				cureveArea += (avgValue - s0)*time;	
			}
		}

		//find max slope;
		float maxSlope = 0.0f;
		float slope = -3.40282e+038f; //(minValue)
		int maxSlopeIndex = -1;
		for (k = toIndex; k < peakInex; ++k)
		{
			slope =(float) (calcPixels[k + 1] - calcPixels[k]) / (imageTimer.at(k + 1) - imageTimer.at(k));
			if (slope > maxSlope)
			{
				maxSlope = slope;
				maxSlopeIndex = k;
			}
		}
		// validate max slope and index;
		bool validMaxSlope = (maxSlopeIndex >= 0) && (maxSlope >= 0);
		if (!validMaxSlope) maxSlope = 0;

		//Find min slope;
		float minSlope = 0.0f;
		slope = +3.40282e+038f; //(maxValue)
		int minSlopeIndex = -1;
		for (k = peakInex; k < m_numberOfDynamics - 1; ++k)
		{
			slope = (float)(calcPixels[k + 1] - calcPixels[k]) / (imageTimer.at(k + 1) - imageTimer.at(k));
			if (slope < minSlope)
			{
				minSlope = slope;
				minSlopeIndex = k;
			}
		}
		bool validMinSlope = (minSlope < 0) && (minSlopeIndex >= 0) && (minSlopeIndex >= maxSlopeIndex);
		if (!validMinSlope) minSlope = 0;

		//othervalues
		ME = peakValue - s0;
		//std::cout << " ME value: " << ME << std::endl;
		MRE = (!(fabs(s0) < 1e-10)) ? (100 * (peakValue - s0) / fabs(s0)) : 0;	
		T0 = toIndex;	
		TTP = imageTimer.at(peakInex) - imageTimer.at(0);		
	    WIR = maxSlope;		
		WOR = -minSlope;
		if (validMinSlope&&validMaxSlope)
		{
			BOE = imageTimer.at(minSlopeIndex) - imageTimer.at(maxSlopeIndex);
		}
		
		//make area always to be greater than 0;
		Area = std::fmax(0.0f, cureveArea);
		//std::cout << "area:" << Area << std::endl;
	}
	perfusionMaps[0] = RE; perfusionMaps[1] = MRE; perfusionMaps[2] = ME;
	perfusionMaps[3] = WOR; perfusionMaps[4] = WIR; perfusionMaps[5] = Area;
	perfusionMaps[6] = T0; perfusionMaps[7] = TTP; perfusionMaps[8] = BOE;
	//perfusionMaps.Fill(ME); //0 Max Enhance
	//perfusionMaps.Fill(MRE); // 1 max relative Enhance
	//perfusionMaps.Fill(T0); //2 T0;
	//perfusionMaps.Fill(TTP); //3 time to peak;s
	//perfusionMaps.Fill(WIR); //4 wash in rate';
	//perfusionMaps.Fill(WOR); //5 wash out rate;
	//perfusionMaps.Fill(BOE); //6 brevity of enhance
	//perfusionMaps.Fill(Area); //7 area under the curve;	
	//std::cout << "ME" << perfusionMaps[0] << std::endl;

}

void perfusionAlgorithm::CalculateT0S0(int& _toIndex, float& _s0)
{
	float total = calcPixels[0];
	float s0 = total;
	int toIndex;
	int k;
	for ( k = 1; k < m_numberOfDynamics; k++)
	{
		float diff = fabs(calcPixels[k] - s0);
		float threshold = fabs(s0Threshold*s0);
		if ((diff > threshold) && (calcPixels[k] > s0))
		{
			break;
		}
		total += calcPixels[k];
		s0 = total / (k + 1);
	}
	toIndex = k - 1;	
	if (toIndex >= m_numberOfDynamics - 1)
	{
		toIndex = 0;
	}
	_toIndex = toIndex;
	_s0 = s0;
}
