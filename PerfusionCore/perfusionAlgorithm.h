#pragma once

#include <vector>
#include "itkVariableLengthVector.h"

class perfusionAlgorithm
{
public:
	perfusionAlgorithm(int numberOfDynamics);
	virtual ~perfusionAlgorithm();

	std::vector<float> calcPixels;
	int m_numberOfDynamics;
	itk::VariableLengthVector<float> perfusionMaps;

	void GenerateMaps();
	void SetDynamicTime(std::vector<float> dynamicTime)
	{
		imageTimer = dynamicTime;
	}
	void SetDynamicPixels(std::vector<float> dynamicPixels)
	{
		calcPixels = dynamicPixels;
	}

	//itk::VariableLengthVector<float> perfusionMaps;

protected:
	void CalculateT0S0(int& _toIndex, float& _s0);

private:
	const float s0Threshold = 0.2f;
	std::vector <float> imageTimer;
    
};

