#ifndef _itkGetDiffusionImageFilter_h_
#define _itkGetDiffusionImageFilter_h_

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include "itkVector.h"
#include "itkImageRegionIterator.h"
#include "itkSmartPointer.h"
#include <itkSymmetricEigenAnalysis.h>
#include <itkRGBPixel.h>
#include <itkTensor.h>

#include <vector>
#include <vnl\vnl_matrix.h>
#include <vnl\vnl_vector.h>

namespace itk
{
	template< typename TInputPixelType, typename TOutputPixelType >
	class GetDiffusionImageFilter :
		public ImageToImageFilter< VectorImage<TInputPixelType, 3>, VectorImage<TOutputPixelType, 3> >
	{
	public:
		/** Standard class typedefs. */
		typedef GetDiffusionImageFilter                           Self;
		typedef ImageToImageFilter< VectorImage<TInputPixelType, 3>, VectorImage<TOutputPixelType, 3> > Superclass;
		typedef SmartPointer< Self >                            Pointer;
		typedef SmartPointer< const Self >                      ConstPointer;

		/** Method for creation through the object factory. */
		itkNewMacro(Self);

		/** Typedef to describe the type of pixel. */
		typedef typename Superclass::InputImageType						 InputImageType;
		typedef typename Superclass::OutputImageType					 OutputImageType;

		/** Image related typedefs. */
		itkStaticConstMacro(ImageDimension, unsigned int,
			InputImageType::ImageDimension);

		/** Run-time type information (and related methods). */
		itkTypeMacro(GetDiffusionImageFilter, ImageToImageFilter);

		typedef RGBPixel< unsigned char >    colorFaPixelType;
		typedef Image<colorFaPixelType, 3>   colorFaImageType;
		typedef Tensor<float> tensorType;
		typedef Image<tensorType, 3>   tensorImageType;

		OutputImageType*  GetOutput1();
		colorFaImageType* GetOutput2();
		tensorImageType*  GetOutput3();
		

		typedef VariableLengthVector <TOutputPixelType> TensorVectorType;
		typedef ImageRegionConstIterator <InputImageType> ConstInputIteratorType;
		typedef ImageRegionIterator <OutputImageType> OutputIteratorType;
		typedef ImageRegionIterator <colorFaImageType> OutputIteratorTypeColorFa;
		typedef ImageRegionIterator <tensorImageType> OutputIteratorTypeTensor;

		void SetBValueList(std::vector<float> bValueList){ m_BValueList = bValueList; }
		void SetHMatrix(vnl_matrix<float> finalH){ m_finalHMatrix = finalH; }
		void SetSlice2PatMatrix(vnl_matrix<double> slice2PatMatrix){ m_slice2PatMatrix = slice2PatMatrix; }
		void SetComputedBValue(float bValue){ m_computedBValue = bValue;  }

	protected:
		GetDiffusionImageFilter();
		~GetDiffusionImageFilter(){}

		void PrintSelf(std::ostream & os, Indent indent) const ITK_OVERRIDE;

		/** Initialize some accumulators before the threads run. */
		void BeforeThreadedGenerateData() ITK_OVERRIDE;

		/** Multi-thread version GenerateData. */
		void  ThreadedGenerateData(const OutputImageRegionType &
			outputRegionForThread, ThreadIdType threadId) ITK_OVERRIDE;

		DataObject::Pointer MakeOutput(unsigned int idx);

		//virtual void GenerateOutputInformation() ITK_OVERRIDE;

	private:
		GetDiffusionImageFilter(const Self &); //ITK_DELETE_FUNCTION;
		void operator=(const Self &);//ITK_DELETE_FUNCTION;

		vnl_matrix<float> m_finalHMatrix;
		std::vector<float> m_BValueList;
		vnl_matrix<double> m_slice2PatMatrix;
		float m_computedBValue;
		const InputImageType * m_InputImage;
		OutputImageType * m_OutputImage; //two elements, 0 Adc, 1 EADC, 2CDWI, 3FA
		colorFaImageType * m_OutputImageColorFa;
	};
} 

#include "itkGetDiffusionImageFilter.hxx"

#endif