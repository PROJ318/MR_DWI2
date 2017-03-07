#ifndef _itkPerfusionMapFilter_h_
#define _itkPerfusionMapFilter_h_

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include "itkVariableLengthVector.h"
#include "itkImageRegionIterator.h"
#include "itkSmartPointer.h"

#include <vector>

namespace itk
{
	template< typename TInputPixelType, typename TOutputPixelType >
	class PerfusionMapFilter :
		public ImageToImageFilter< VectorImage<TInputPixelType, 3>, VectorImage<TOutputPixelType, 3> >
	{
	public:
		/** Standard class typedefs. */
		typedef PerfusionMapFilter                           Self;
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

		//typedef typename TOutputImage::PixelType OutputImagePixelType;

		/** Typedef to describe the output and input image index and size types. */
		//typedef typename TInputImage::IndexType   InputImageIndexType;
		//typedef typename TInputImage::SizeType    InputImageSizeType;
		//typedef typename TInputImage::OffsetType  InputImageOffsetType;
		//typedef typename TOutputImage::IndexType  OutputImageIndexType;
		//typedef typename TOutputImage::SizeType   OutputImageSizeType;
		//typedef typename TOutputImage::OffsetType OutputImageOffsetType;

		/** Type to use form computations. */
		//typedef typename NumericTraits< OutputImagePixelType >::RealType RealType;


		/** Image related typedefs. */
		//itkStaticConstMacro(ImageDimension, unsigned int,
			//InputImageType::ImageDimension);

		/** Run-time type information (and related methods). */
		itkTypeMacro(PerfusionMapFilter, ImageToImageFilter);

		/** Set/Get the amount to Shift each Pixel. The shift is followed by a Scale.
		*/
		//itkSetMacro(Shift, RealType);
		itkGetConstMacro(NumOfDynamics, unsigned int);
		void setimageTimer(std::vector<float> timeList)
		{
			m_imageTimer = timeList;
		}


	protected:
		PerfusionMapFilter();
		~PerfusionMapFilter(){}

		void PrintSelf(std::ostream & os, Indent indent) const ITK_OVERRIDE;

		/** Initialize some accumulators before the threads run. */
		void BeforeThreadedGenerateData() ITK_OVERRIDE;

		/** Tally accumulated in threads. */
		//void AfterThreadedGenerateData() ITK_OVERRIDE;

		/** Multi-thread version GenerateData. */
		void  ThreadedGenerateData(const OutputImageRegionType &
			outputRegionForThread, ThreadIdType threadId) ITK_OVERRIDE;

	private:
		PerfusionMapFilter(const Self &); //ITK_DELETE_FUNCTION;
		void operator=(const Self &);//ITK_DELETE_FUNCTION;

		unsigned int m_NumOfDynamics;
		std::vector<float> m_imageTimer;

		const InputImageType * m_InputImage;
		OutputImageType * m_OutputImage;
	};
}

#include "itkPerfusionMapFilter.hxx"

#endif