#ifndef _itkDwiIVIMCostFunction2_h_
#define _itkDwiIVIMCostFunction2_h_

#include "itkMultipleValuedCostFunction.h"
#include "itkVariableLengthVector.h"
#include <vector>

namespace itk
{
	template< typename TPixelType >
	class DwiIVIMCostFunction2 :
		public MultipleValuedCostFunction
	{
	public:
		/** Standard class typedefs. */
		typedef DwiIVIMCostFunction2        Self;
		typedef MultipleValuedCostFunction Superclass;
		typedef SmartPointer<Self>         Pointer;
		typedef SmartPointer<const Self>   ConstPointer;
		typedef itk::VariableLengthVector <TPixelType>		VariableLengthVectorType;
		/** Method for creation through the object factory. */
		itkNewMacro(Self);

		/** Run-time type information (and related methods). */
		itkTypeMacro(DwiIVIMCostFunction2, MultipleValuedCostFunction);

		// User Set
		void SetBValueList(std::vector<TPixelType> bValueList){
			m_BValueList = bValueList;
			m_NumOfDiffBValues = m_BValueList.size();
		}
		std::vector<TPixelType> GetBValueList(){ return this->m_BValueList; }

		void SetPixelArray(VariableLengthVectorType pixelArray){
			m_Y = pixelArray;
		}

		void SetPixelS0(TPixelType S0){
			m_S0 = double (S0);
		}

		void SetPixelD(TPixelType D){
			m_D = double(D);
		}




		// The equation we're fitting is Sb = S0*( (1-f)*exp(-bD) + f*exp(-b(D+D*)) )
		// Where, D is diffusion, D* is perfusion. 
		// D* = ADC ( using b = 0 & b > 200 s/mm2);
		unsigned int GetNumberOfParameters() const { return 2; }

		// !!!!!!!!!!!!!!!!!!!!!!
		// Only compute for 1 diffusion direction, make sure to regard other directions of the input src image
		// There are BValueList.size() values.
		unsigned int GetNumberOfValues() const { return this->m_NumOfDiffBValues; }

		// Calculate the residual array, given a set of parameters.
		MeasureType GetValue(const ParametersType &parameters) const
		{
			MeasureType residual(m_NumOfDiffBValues);
			//double predictedS0 = m_S0;
			double predictedF = parameters[0];
			//double predictedD = parameters[2];
			double predictedDstar = parameters[1];
			for (unsigned int i = 0; i < m_NumOfDiffBValues; ++i)
			{
				double bValue = double(m_BValueList.at(i));
				double prediction = m_S0*((1. - predictedF)*exp(-bValue * m_D) + predictedF*exp(-bValue*(m_D + predictedDstar)));
				residual[i] = prediction - m_Y[i];
			}
			return residual;
		}

		// The "derivative" is the Jacobian, which takes the derivative
		// of each residual with respect to each parameter.  Since this
		// class does not provide a derivative, any optimizer using this
		// cost function must be told explicitly not to ask for derivative,
		// otherwise an exception will the thrown.
		// Define later to compare its accuracy and speed with default gradoff
		void GetDerivative(const ParametersType &parameters, DerivativeType & derivative) const
		{
			std::cout << "Derivative not implemented yet, use LM without gradient J" << std::endl;
			throw ExceptionObject(__FILE__, __LINE__, "No derivative available.");
		}

	protected:
		DwiIVIMCostFunction2() {};
		~DwiIVIMCostFunction2() {};

	private:
		DwiIVIMCostFunction2(const Self &); //purposely not implemented
		void operator = (const Self &); //purposely not implemented

		std::vector<TPixelType> m_BValueList;
		unsigned int m_NumOfDiffBValues;// Equals to NumOfDiffComponents, because only input image with direction = 1 is allowed
		VariableLengthVectorType m_Y;
		double m_S0;
		double m_D;		
	};
} // end namespace itk
#endif