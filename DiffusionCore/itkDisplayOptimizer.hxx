#ifndef _itkDisplayOptimizer_hxx_
#define _itkDisplayOptimizer_hxx_

#include "itkDisplayOptimizer.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkImageToHistogramFilter.h"

namespace itk
{
	template< typename TInputImage, typename TOutputImage >
	DisplayOptimizer< TInputImage, TOutputImage >
		::DisplayOptimizer()
	{
		//m_OutputMaximum = NumericTraits< OutputPixelType >::max();
		//m_OutputMinimum = NumericTraits< OutputPixelType >::NonpositiveMin();

		m_InputMaximum = NumericTraits< InputPixelType >::ZeroValue();
		m_InputMinimum = NumericTraits< InputPixelType >::max();
		m_HistogramSize = 500;//default as 500

		//m_Scale = 1.0;
		//m_Shift = 0.0;
		m_CoveragePercent = static_cast <InputPixelType> (0.99);

		m_InputImage = ITK_NULLPTR;
		m_OutputImage = ITK_NULLPTR;
	}

	template< typename TInputImage, typename TOutputImage >
	void
	DisplayOptimizer< TInputImage, TOutputImage >
	::GenerateData()
	{
		//if (m_OutputMinimum > m_OutputMaximum)
		//{
			//itkExceptionMacro(<< "Minimum output value cannot be greater than Maximum output value.");
			//return;
		//}

		m_InputImage = this->GetInput();
		m_OutputImage = this->GetOutput();

		typedef MinimumMaximumImageCalculator< TInputImage > CalculatorType;
		typename CalculatorType::Pointer calculator = CalculatorType::New();

		calculator->SetImage(this->GetInput());
		calculator->Compute();
		m_InputMinimum = calculator->GetMinimum();
		m_InputMaximum = calculator->GetMaximum();

		//Get histogram: Grayscale image type, extend to vector images if needed
		typedef itk::Statistics::ImageToHistogramFilter< TInputImage > ImageToHistogramFilterType;
		ImageToHistogramFilterType::HistogramType::MeasurementVectorType lowerBound(1);
		lowerBound.Fill(m_InputMinimum);
		ImageToHistogramFilterType::HistogramType::MeasurementVectorType upperBound(1);
		upperBound.Fill(m_InputMaximum);

		ImageToHistogramFilterType::HistogramType::SizeType size(1);
		size.Fill(m_HistogramSize);

		ImageToHistogramFilterType::Pointer imageToHistogramFilter = ImageToHistogramFilterType::New();
		imageToHistogramFilter->SetInput(m_InputImage);
		imageToHistogramFilter->SetHistogramBinMinimum(lowerBound);
		imageToHistogramFilter->SetHistogramBinMaximum(upperBound);
		imageToHistogramFilter->SetHistogramSize(size);
		imageToHistogramFilter->Update();

		ImageToHistogramFilterType::HistogramType::Pointer histogram = imageToHistogramFilter->GetOutput();

		//Discard edge points
		unsigned int pixelsInHistogram = 0;//do not count min values
		double upperValue = 0.0;
		double lowerValue = 0.0;
		for (unsigned int i = 1; i < m_HistogramSize; i++)
		{
			pixelsInHistogram += histogram->GetFrequency(i);
		}

		if (pixelsInHistogram != 0)
		{
			unsigned int nrPixelsToIgnore = (unsigned int)((1.0 - m_CoveragePercent)*pixelsInHistogram);
			unsigned int upperIndex = m_HistogramSize - 1;
			unsigned int lowerIndex = 0;
			unsigned int nrPixelsIgnored = 0;

			while ((nrPixelsIgnored < nrPixelsToIgnore) && (upperIndex - lowerIndex > 2))
			{
				if (histogram->GetFrequency(lowerIndex) < histogram->GetFrequency(upperIndex))
				{
					nrPixelsIgnored += histogram->GetFrequency(lowerIndex);
					if (nrPixelsIgnored < nrPixelsToIgnore)
					{
						lowerIndex++;
					}
				}
				else
				{
					nrPixelsIgnored += histogram->GetFrequency(upperIndex);
					if (nrPixelsIgnored < nrPixelsToIgnore)
					{
						upperIndex--;
					}
				}
			}

			//Now compute the new extremes.
			double range = (double) (m_InputMaximum - m_InputMinimum);
			double cutOff = (double)(lowerIndex) / m_HistogramSize;
			lowerValue = cutOff*range + m_InputMinimum;
			cutOff = (double)(upperIndex + 1) / m_HistogramSize;
			upperValue = cutOff*range + m_InputMinimum;
		}

		//std::cout << "input min = " << m_InputMinimum << std::endl;
		//std::cout << "input max = " << m_InputMaximum << std::endl;
		//std::cout << "lowerValue = " << lowerValue << std::endl;
		//std::cout << "upperValue = " << upperValue << std::endl;

		//cutoff the input image pixle intensity to [lowerValue, upperValue]
		//Maybe we should rescale it to [0, 4095] here rather than use another filter outside, benefit is some sort of speed enhancement
		m_OutputImage->SetSpacing(m_InputImage->GetSpacing());
		m_OutputImage->SetOrigin(m_InputImage->GetOrigin());
		m_OutputImage->SetDirection(m_InputImage->GetDirection());
		m_OutputImage->SetRegions(m_InputImage->GetLargestPossibleRegion());
		m_OutputImage->Allocate();
		ConstInputIteratorType inputIt(m_InputImage, m_InputImage->GetLargestPossibleRegion());
		OutputIteratorType outputIt(m_OutputImage, m_InputImage->GetLargestPossibleRegion());
		inputIt.GoToBegin();
		outputIt.GoToBegin();
		while (!inputIt.IsAtEnd())
		{
			if (inputIt.Get() < static_cast <InputPixelType> (lowerValue))
			{
				outputIt.Set(static_cast <OutputPixelType> (lowerValue));
			}
			else if (inputIt.Get() > static_cast <InputPixelType> (upperValue))
			{
				outputIt.Set(static_cast <OutputPixelType> (upperValue));
			}
			else
			{
				outputIt.Set(static_cast <OutputPixelType> (inputIt.Get()));
			}
			++outputIt;
			++inputIt;
		}
	}

	template< typename TInputImage, typename TOutputImage >
	void
	DisplayOptimizer< TInputImage, TOutputImage >
		::PrintSelf(std::ostream& os, Indent indent) const
	{
		Superclass::PrintSelf(os, indent);
		//os << indent << "NumOfComponents: " << m_NumOfComponents << std::endl;
	}
} //end namespace itk

#endif
