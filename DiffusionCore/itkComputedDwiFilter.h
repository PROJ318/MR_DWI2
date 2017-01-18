#ifndef _itkComputedDwiFilter_h_
#define _itkComputedDwiFilter_h_

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include "itkVariableLengthVector.h"
#include "itkImageRegionIterator.h"
#include "itkSmartPointer.h"


namespace itk
{
	template< typename TInputPixelType, typename TOutputPixelType >
	class ComputedDwiFilter :
		public ImageToImageFilter< VectorImage<TInputPixelType, 3>, VectorImage<TOutputPixelType, 3> >
	{
	public:
		/** Standard class typedefs. */
		typedef ComputedDwiFilter                           Self;
		typedef ImageToImageFilter< VectorImage<TInputPixelType, 3>, VectorImage<TOutputPixelType, 3> > Superclass;
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
		typedef VariableLengthVector <TOutputPixelType> VariableLengthVectorType;

		typedef itk::ImageRegionConstIterator <InputImageType> ConstInputIteratorType;
		typedef itk::ImageRegionIterator <OutputImageType> OutputIteratorType;
		//typedef typename NumericTraits< std::vector<float> >::RealType RealType;

		/** Run-time type information (and related methods). */
		itkTypeMacro(ComputedDwiFilter, ImageToImageFilter);

		/** Set/Get the amount to Shift each Pixel. The shift is followed by a Scale.
		*/

		itkSetMacro(NumOfDiffDirections, unsigned int);
		itkGetConstMacro(NumOfDiffDirections, unsigned int);

		itkSetMacro(ComputedBValue, unsigned int);
		itkGetConstMacro(ComputedBValue, unsigned int);

	protected:
		ComputedDwiFilter();
		~ComputedDwiFilter(){}

		void PrintSelf(std::ostream & os, Indent indent) const ITK_OVERRIDE;

		/** Initialize some accumulators before the threads run. */
		void BeforeThreadedGenerateData() ITK_OVERRIDE;

		/** Tally accumulated in threads. */
		//void AfterThreadedGenerateData() ITK_OVERRIDE;

		/** Multi-thread version GenerateData. */
		void  ThreadedGenerateData(const OutputImageRegionType &
			outputRegionForThread, ThreadIdType threadId) ITK_OVERRIDE;

		//virtual void GenerateOutputInformation() ITK_OVERRIDE;

	private:
		ComputedDwiFilter(const Self &); //ITK_DELETE_FUNCTION;
		void operator=(const Self &);//ITK_DELETE_FUNCTION;

		unsigned int m_NumOfDiffDirections;
		unsigned int m_ComputedBValue;
		//int m_NumOfComponents;
		const InputImageType * m_InputImage;
		OutputImageType * m_OutputImage;
	};
} 

#include "itkComputedDwiFilter.hxx"

#endif