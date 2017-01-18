#ifndef _itkDwiIVIMCostFunction_h_
#define _itkDwiIVIMCostFunction_h_

#include "itkMultipleValuedCostFunction.h"
#include "itkVariableLengthVector.h"
#include <vector>

namespace itk
{
	template< typename TPixelType >
	class DwiIVIMCostFunction :
		public MultipleValuedCostFunction
	{
	public:
		/** Standard class typedefs. */
		typedef DwiIVIMCostFunction        Self;
		typedef MultipleValuedCostFunction Superclass;
		typedef SmartPointer<Self>         Pointer;
		typedef SmartPointer<const Self>   ConstPointer;
		typedef itk::VariableLengthVector <TPixelType>		VariableLengthVectorType;
		/** Method for creation through the object factory. */
		itkNewMacro(Self);

		/** Run-time type information (and related methods). */
		itkTypeMacro(DwiIVIMCostFunction, MultipleValuedCostFunction);

		// User Set
		void SetBValueList(std::vector<TPixelType> bValueList){
			m_BValueList = bValueList;
			m_NumOfDiffBValues = m_BValueList.size();
		}
		std::vector<TPixelType> GetBValueList(){ return this->m_BValueList; }

		void SetPixelArray(VariableLengthVectorType pixelArray){
			m_Y = pixelArray;
		}


		// The equation we're fitting is Sb = S0*( (1-f)*exp(-bD) + f*exp(-b(D+D*)) )
		// Where, D is diffusion, D* is perfusion
		// Or Should we use another model with D* as constant? Eg. D* = ADC ( if b > 200 s/mm2);
		unsigned int GetNumberOfParameters() const { return 4; }

		// !!!!!!!!!!!!!!!!!!!!!!
		// Only compute for 1 diffusion direction, make sure to regard other directions of the input src image
		// There are BValueList.size() values.
		unsigned int GetNumberOfValues() const { return this->m_NumOfDiffBValues; }

		// Calculate the residual array, given a set of parameters.
		MeasureType GetValue(const ParametersType &parameters) const
		{
			MeasureType residual(m_NumOfDiffBValues);
			double predictedS0 = parameters[0];
			double predictedF = parameters[1];
			double predictedD = parameters[2];
			double predictedDstar = parameters[3];

			for (unsigned int i = 0; i < m_NumOfDiffBValues; ++i)
			{
				double bValue = double(m_BValueList.at(i));
				double prediction = predictedS0*((1. - predictedF)*exp(-bValue * predictedD) + predictedF*exp(-bValue*(predictedD + predictedDstar)));
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
		DwiIVIMCostFunction() {};
		~DwiIVIMCostFunction() {};

	private:
		DwiIVIMCostFunction(const Self &); //purposely not implemented
		void operator = (const Self &); //purposely not implemented

		std::vector<TPixelType> m_BValueList;
		unsigned int m_NumOfDiffBValues;// Equals to NumOfDiffComponents, because only input image with direction = 1 is allowed
		VariableLengthVectorType m_Y;
	};
} // end namespace itk
#endif