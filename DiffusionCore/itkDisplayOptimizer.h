#ifndef _itkDisplayOptimizer_h_
#define _itkDisplayOptimizer_h_

#include "itkImageToImageFilter.h"
#include "itkImage.h"
#include "itkImageRegionIterator.h"
#include "itkSmartPointer.h"
#include "itkNumericTraits.h"

namespace itk
{
	template< typename TInputImage, typename TOutputImage >
	class DisplayOptimizer :
		public ImageToImageFilter< TInputImage, TOutputImage >
	{
	public:
		/** Standard class typedefs. */
		typedef DisplayOptimizer                           Self;
		typedef ImageToImageFilter< TInputImage, TOutputImage > Superclass;
		typedef SmartPointer< Self >                            Pointer;
		typedef SmartPointer< const Self >                      ConstPointer;

		/** Method for creation through the object factory. */
		itkNewMacro(Self);

		/** Typedef to describe the output and input image region types. */
		//typedef typename TInputImage::RegionType  InputImageRegionType;
		//typedef typename TOutputImage::RegionType OutputImageRegionType;

		/** Typedef to describe the pointer to the input/output. */
		//typedef typename TInputImage::Pointer  InputImagePointer;
		//typedef typename TOutputImage::Pointer OutputImagePointer;

		/** Typedef to describe the type of pixel. */
		typedef typename Superclass::InputImageType						 InputImageType;
		typedef typename Superclass::OutputImageType					 OutputImageType;
		typedef typename TOutputImage::PixelType                   OutputPixelType;
		typedef typename TInputImage::PixelType                    InputPixelType;
		typedef typename NumericTraits< InputPixelType >::RealType RealType;

		typedef itk::ImageRegionConstIterator <InputImageType> ConstInputIteratorType;
		typedef itk::ImageRegionIterator <OutputImageType> OutputIteratorType;

		//itkSetMacro(OutputMinimum, OutputPixelType);
		//itkSetMacro(OutputMaximum, OutputPixelType);
		//itkGetConstReferenceMacro(OutputMinimum, OutputPixelType);
		//itkGetConstReferenceMacro(OutputMaximum, OutputPixelType);

		/** Get the Scale and Shift used for the linear transformation
		of gray level values.
		\warning These Values are only valid after the filter has been updated */
		//itkGetConstReferenceMacro(Scale, RealType);
		//itkGetConstReferenceMacro(Shift, RealType);

		/** Get the Minimum and Maximum values of the input image.
		\warning These Values are only valid after the filter has been updated */
		itkGetConstReferenceMacro(InputMinimum, InputPixelType);
		itkGetConstReferenceMacro(InputMaximum, InputPixelType);

		/** Run-time type information (and related methods). */
		itkTypeMacro(DisplayOptimizer, ImageToImageFilter);

		/** Set/Get the amount to Shift each Pixel. The shift is followed by a Scale.
		*/
		itkSetMacro(CoveragePercent, InputPixelType);
		itkGetConstMacro(CoveragePercent, InputPixelType);
		//itkGetConstMacro(NumOfDiffBValues, unsigned int);
		//itkGetConstMacro(NumOfComponents, unsigned int);

	protected:
		DisplayOptimizer();
		~DisplayOptimizer(){}

		void PrintSelf(std::ostream & os, Indent indent) const ITK_OVERRIDE;

		/** Initialize some accumulators before the threads run. */
		void GenerateData() ITK_OVERRIDE;

	private:
		DisplayOptimizer(const Self &); //ITK_DELETE_FUNCTION;
		void operator=(const Self &);//ITK_DELETE_FUNCTION;

		//RealType m_Scale;
		//RealType m_Shift;

		InputPixelType m_InputMinimum;
		InputPixelType m_CoveragePercent;
		InputPixelType m_InputMaximum;
		unsigned int m_HistogramSize;

		//OutputPixelType m_OutputMinimum;
		//OutputPixelType m_OutputMaximum;
		const InputImageType * m_InputImage;
		OutputImageType * m_OutputImage;
	};
}

#include "itkDisplayOptimizer.hxx"

#endif