#ifndef _itkDwiIVIMFilter_h_
#define _itkDwiIVIMFilter_h_

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include "itkVariableLengthVector.h"
#include "itkImageRegionIterator.h"
#include "itkSmartPointer.h"
#include "itkLevenbergMarquardtOptimizer.h"
#include "itkDwiIVIMCostFunction.h"

namespace itk
{
	template< typename TInputPixelType, typename TOutputPixelType >
	class DwiIVIMFilter:
		public ImageToImageFilter< VectorImage<TInputPixelType, 3>, VectorImage<TOutputPixelType, 3> >
	{
	public:
		/** Standard class typedefs. */
		typedef DwiIVIMFilter                           Self;
		typedef ImageToImageFilter< VectorImage<TInputPixelType, 3>, VectorImage<TOutputPixelType, 3> > Superclass;
		typedef SmartPointer< Self >                            Pointer;
		typedef SmartPointer< const Self >                      ConstPointer;

		/** Method for creation through the object factory. */
		itkNewMacro(Self);

		/** Typedef to describe the output and input image region types. */
		//typedef typename TInputImage::RegionType  InputImageRegionType;
		//typedef typename TOutputImage::RegionType OutputImageRegionType;

		/** Typedef to describe the image type. */
		typedef typename Superclass::InputImageType						 InputImageType;
		typedef typename Superclass::OutputImageType					 OutputImageType;
		typedef VariableLengthVector <TInputPixelType>					 VariableLengthVectorType;

		typedef itk::ImageRegionConstIterator <InputImageType> ConstInputIteratorType;
		typedef itk::ImageRegionIterator <OutputImageType> OutputIteratorType;

		typedef itk::LevenbergMarquardtOptimizer OptimizerType;
		typedef itk::DwiIVIMCostFunction <TInputPixelType>		CostType;
		//typedef typename NumericTraits< std::vector<float> >::RealType RealType;

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
		itkTypeMacro(DwiIVIMFilter, ImageToImageFilter);

		// User defined input 
		void SetBValueList(std::vector<TInputPixelType> bValueList){ m_BValueList = bValueList; }
		std::vector<TInputPixelType> GetBValueList(){ return m_BValueList; }

		itkSetMacro(NumOfIterations, unsigned int);
		itkGetConstMacro(NumOfIterations, unsigned int);

	protected:
		DwiIVIMFilter();
		~DwiIVIMFilter(){}

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
		DwiIVIMFilter(const Self &); //ITK_DELETE_FUNCTION;
		void operator=(const Self &);//ITK_DELETE_FUNCTION;

		unsigned int m_NumOfIterations;
		double m_GradientTolerence;
		double m_ValueTolerence;
		double m_EpsilonFunction;
		std::vector<TInputPixelType> m_BValueList;
		const InputImageType * m_InputImage;
		OutputImageType * m_OutputImage;
	};
} 

#include "itkDwiIVIMFilter.hxx"

#endif