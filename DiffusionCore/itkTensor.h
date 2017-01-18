#ifndef _itkTensor_h_
#define _itkTensor_h_


#include <itkSymmetricEigenAnalysis.h>
#include <itkRGBPixel.h>
#include <itkMatrix.h>
#include <itkFixedArray.h>
#include <itkIndent.h>

namespace itk
{
	/*\class Tensor
	\brief A templated class holding a diffusion Tensor
	\the first element is the b0 value, which is referenceDwiValue
	\the other six elements are the symmetric tensor value, they are arranged in the Matrix
	\as follow:
	\ 0  3  4
	\ 3  1  5
	\ 4  5  2
	\\\
	*/
	template< typename TComponent = float>
	class Tensor:public itk::FixedArray< TComponent, 7>
	{
	public:
		/** Standard class typedefs. */
		typedef Tensor                           Self;
		typedef FixedArray< TComponent, 7 > Superclass;
		typedef FixedArray< TComponent, 7 > BaseArray;

		itkStaticConstMacro(Dimension, unsigned int, 3);
		typedef TComponent                          ComponentType;
		typedef typename Superclass::ValueType                ValueType;


		typedef FixedArray< TComponent, 3 > EigenValuesArrayType;
		typedef Matrix< TComponent, 3, 3 > MatrixType;
		typedef Matrix< TComponent, 3, 3 > EigenVectorsMatrixType;
		typedef Vector< TComponent, 3 > EigenVectorType;
		typedef SymmetricEigenAnalysis< MatrixType, EigenValuesArrayType, EigenVectorsMatrixType >  SymmetricEigenAnalysisType;
		typedef RGBPixel< unsigned char >    colorFaPixelType;
		typedef FixedArray<TComponent, 6>  SignalValueType;

		Tensor() {}
		Tensor(const ComponentType & r) { this->Fill(r); }

		template< typename TTensorlValueType >
		Self & operator=(const Tensor< TTensorlValueType > & r)
		{
			BaseArray::operator=(r);
			return *this;
		}

		Self & operator=(const ComponentType r[7]);


		static unsigned int GetNumberOfComponents(){ return 7; }
		
		ComponentType GetNthComponent(int c) const { return this->operator[](c); }
		void SetNthComponent(int c, const ComponentType & v) { this->operator[](c) = v; }
		void Set(ComponentType c1, ComponentType c2, ComponentType c3, ComponentType c4,
			ComponentType c5, ComponentType c6, ComponentType c7)
		{
			this->operator[](0) = c1;
			this->operator[](1) = c2;
			this->operator[](2) = c3;
			this->operator[](3) = c4;
			this->operator[](4) = c5;
			this->operator[](5) = c6;
			this->operator[](6) = c7;
			//this->EigenValues.Fill(0.0);
			//this->EigenVectors.Fill(0.0);
			this->CalculateEigenvector();
		}

		ValueType & operator()(unsigned int row, unsigned int col);
		const ValueType & operator()(unsigned int row, unsigned int col) const;
		
		//void  CalculateEigenValues();
		void  CalculateEigenvector() ;
		ComponentType ComputeAdcIso();
		ComponentType ComputeEAdcIso(float bValue);
		ComponentType ComputeFa();
		colorFaPixelType ComputeColorFa(vnl_matrix<double> m_slice2PatMatrix);
		EigenVectorType GetPrimaryEigenVector();
		void GetSignalFromTensor(SignalValueType & signal);
		void SetTensorFromSignal(ComponentType referenceValue, SignalValueType & signal);
	private:
		EigenValuesArrayType EigenValues;
		EigenVectorsMatrixType EigenVectors;

	};

	typedef std::ostream OutputStreamType;
	typedef std::istream InputStreamType;

	template< typename TComponent >
	OutputStreamType & operator<<(OutputStreamType & os,
		                             const Tensor< TComponent > & c);

	template< typename TComponent >
	InputStreamType & operator>>(InputStreamType & is,
		                       Tensor< TComponent > & c);

} // end namespace itk

#include "itkTensor.hxx"

#endif