#ifndef _itkTensor_hxx_
#define _itkTensor_hxx_

#include "itkTensor.h"
#include "vnl\algo\vnl_matrix_inverse.h"

namespace itk
{
	/**
	* Assigment from a plain array
	*/
	template< typename T >
	Tensor< T > &
		Tensor< T >
		::operator=(const ComponentType r[7])
	{
		BaseArray::operator=(r);
		return *this;
	}
	/**
	* Matrix notation access to elements
	*/
	template< typename T >
	const typename Tensor< T >::ValueType &
		Tensor< T >
		::operator()(unsigned int row, unsigned int col) const
	{
		unsigned int k;
		if (row == col)
		{
			k = row;
		}
		else
		{
			k = row + col + 2;
		}
		if (k >= 7)
		{
			k = 0;
		}
		return (*this)[k + 1];
	}

	/**
	* Matrix notation access to elements
	*/
	template< typename T >
	typename Tensor< T >::ValueType &
		Tensor< T >
		::operator()(unsigned int row, unsigned int col)
	{
		unsigned int k;
		if (row == col)
		{
			k = row;
		}
		else 
		{
			k = row + col + 2;
		}
		if (k >= 7)
		{
			k = 0;
		}
		return (*this)[k + 1 ];
	}

	/**
	* Compute Eigen analysis, it returns an array with eigen values
	* and a Matrix with eigen vectors
	*/
	template< typename T >
	void
		Tensor< T >
		::CalculateEigenvector()
	{
		SymmetricEigenAnalysisType symmetricEigenSystem = SymmetricEigenAnalysisType(3);

		MatrixType tensorMatrix;
		EigenValuesArrayType eigenValues;
		EigenVectorsMatrixType eigenVectors;
		for (unsigned int row = 0; row < Dimension; row++)
		{
			for (unsigned int col = 0; col < Dimension; col++)
			{
				tensorMatrix[row][col] = (*this)(row, col);
			}
		}

		symmetricEigenSystem.ComputeEigenValuesAndVectors(
			tensorMatrix, eigenValues, eigenVectors);
		this->EigenValues[0] = eigenValues[0];
		this->EigenValues[1] = eigenValues[1];
		this->EigenValues[2] = eigenValues[2];

		this->EigenVectors = eigenVectors;
	}

	/**
	* Compute Eigen Values
	*/
	//template< typename T >
	//void
	//	Tensor<T>
	//	::CalculateEigenValues( )
	//{
	//	SymmetricEigenAnalysisType symmetricEigenSystem = SymmetricEigenAnalysisType(3);

	//	MatrixType tensorMatrix;
	//	EigenValuesArrayType eigenValues;

	//	for (unsigned int row = 0; row < Dimension; row++)
	//	{
	//		for (unsigned int col = 0; col < Dimension; col++)
	//		{
	//			tensorMatrix[row][col] = (*this)(row, col);
	//		}
	//	}

	//	symmetricEigenSystem.ComputeEigenValues(tensorMatrix, eigenValues);
	//	this->EigenValues[0] = eigenValues[0];
	//	this->EigenValues[1] = eigenValues[1];
	//	this->EigenValues[2] = eigenValues[2];
	//}

	/**
	* Compute ADC value, it returns ADC value for the tensor
	*/
	template< typename T>
	typename Tensor< T >::ComponentType 
		Tensor< T >
		::ComputeAdcIso()
	{
		ComponentType referenceDwiValue = (*this)[0];
		if (referenceDwiValue < exp(-10))
		{
			return 0.0;
		}
		else
		{
			/*EigenValuesArrayType eigenValues;
			CalculateEigenValues(eigenValues);
			return ((eigenValues[0] + eigenValues[1] + eigenValues[2]) / 3);*/
			return ((this->EigenValues[0] + this->EigenValues[1] + this->EigenValues[2]) / 3);
		}

	}

	/**
	* Compute EADC value, it returns EADC value for the tensor
	*/
	template< typename T>
	typename Tensor< T >::ComponentType
		Tensor< T >
		::ComputeEAdcIso(float bValue)
	{
		ComponentType referenceDwiValue = (*this)[0];
		if (referenceDwiValue < exp(-10))
		{
			return 0.0;
		}
		else
		{
			//EigenValuesArrayType eigenValues;
			//CalculateEigenValues(eigenValues);
			//float Adc = (eigenValues[0] + eigenValues[1] + eigenValues[2]) / 3;
			//return (exp(-bValue*Adc));
			return(exp(-bValue*ComputeAdcIso()));
		}

	}

	/**
	* Compute fa value, it returns fa value for the tensor
	*/
	template< typename T>
	typename Tensor< T >::ComponentType 
		Tensor< T >
		::ComputeFa()
	{
		ComponentType referenceDwiValue = (*this)[0];
		if (referenceDwiValue < exp(-10))
		{
			return 0.0;
		}
		else
		{
			//EigenValuesArrayType eigenValues;
			//CalculateEigenValues(eigenValues);
				

			//set negative eigenvalues to Zero.
			ComponentType r1 = this->EigenValues[0] > 0.0F ? this->EigenValues[0] : 0.0;
			ComponentType r2 = this->EigenValues[1] > 0.0F ? this->EigenValues[1] : 0.0;
			ComponentType r3 = this->EigenValues[2] > 0.0F ? this->EigenValues[2] : 0.0;
			ComponentType meanEigen = (r1 + r2 + r3) / 3;
			ComponentType a = r1 - meanEigen;
			ComponentType b = r2 - meanEigen;
			ComponentType c = r3 - meanEigen;

			ComponentType denominator = sqrt(2 * (r1*r1 + r2*r2 + r3*r3));

			ComponentType rawFa = 0.0f;
			if (denominator > exp(-10))
			{
				rawFa = (float)(sqrt(3 * (a*a + b*b + c*c)) / denominator);
			}
			//cout << rawFa << endl;
			return rawFa;

		}
	}

	/**
	* Compute color fa value, it returns color fa value for the tensor
	*/
	template< typename T>
	typename Tensor< T >::colorFaPixelType 
		Tensor< T >
		::ComputeColorFa(vnl_matrix<double> m_slice2PatMatrix)
	{
		ComponentType referenceDwiValue = (*this)[0];
		colorFaPixelType colorFa;
		//if (referenceDwiValue < exp(-10))
		//{
		//	colorFa.SetRed(0.0);
		//	colorFa.SetGreen(0.0);
		//	colorFa.SetBlue(0.0);
		//}
		//else
		//{
			//EigenValuesArrayType eigenValues;
			//EigenVectorsMatrixType  eigenVectors;
			//CalculateEigenvector(eigenValues, eigenVectors);

			//set negative eigenvalues to Zero.
			//ComponentType r1 = eigenValues[0] > 0.0F ? eigenValues[0] : 0.0;
			//ComponentType r2 = eigenValues[1] > 0.0F ? eigenValues[1] : 0.0;
			//ComponentType r3 = eigenValues[2] > 0.0F ? eigenValues[2] : 0.0;
			//ComponentType meanEigen = (r1 + r2 + r3) / 3;
			//ComponentType a = r1 - meanEigen;
			//ComponentType b = r2 - meanEigen;
			//ComponentType c = r3 - meanEigen;

			//ComponentType denominator = sqrt(2 * (r1*r1 + r2*r2 + r3*r3));

			//ComponentType Fa = 0.0f;
			//if (denominator > exp(-10))
			//{
			//	Fa = (float)(sqrt(3 * (a*a + b*b + c*c)) / denominator);
			//}
			ComponentType Fa = ComputeFa();

			//cout << "fa " << endl;
			itk::Vector<ComponentType,3> rgb;
			rgb[0] = EigenVectors(2, 0)*m_slice2PatMatrix(0, 0) + EigenVectors(2, 1)*m_slice2PatMatrix(0, 1) + EigenVectors(2, 2)*m_slice2PatMatrix(0, 2);
			rgb[1] = EigenVectors(2, 0)*m_slice2PatMatrix(1, 0) + EigenVectors(2, 1)*m_slice2PatMatrix(1, 1) + EigenVectors(2, 2)*m_slice2PatMatrix(1, 2);
			rgb[2] = EigenVectors(2, 0)*m_slice2PatMatrix(2, 0) + EigenVectors(2, 1)*m_slice2PatMatrix(2, 1) + EigenVectors(2, 2)*m_slice2PatMatrix(2, 2);
			unsigned short red, green, blue;
			red = (unsigned short)(abs(rgb[0] * Fa) * 255);
			green = (unsigned short)(abs(rgb[1] * Fa) * 255);
			blue = (unsigned short)(abs(rgb[2] * Fa) * 255);

			colorFa.SetRed(red);
			colorFa.SetGreen(green);
			colorFa.SetBlue(blue);
		//}

		return colorFa;
	}

	template< typename T>
	typename Tensor< T >::EigenVectorType 
		Tensor<T>
		::GetPrimaryEigenVector()
	{
		ComponentType referenceDwiValue = (*this)[0];
		EigenVectorType primaryVector;
		if (referenceDwiValue < exp(-10))
		{
			primaryVector[0] = primaryVector[1] = primaryVector[2] = 0;;
		}
		else
		{
			EigenValuesArrayType eigenValues;
			EigenVectorsMatrixType eigenVectorMatrix;
			CalculateEigenvector(eigenValues, eigenVectorMatrix);

			primaryVector[0] = eigenVectorMatrix(2, 0);
			primaryVector[1] = eigenVectorMatrix(2, 1);
			primaryVector[2] = eigenVectorMatrix(2, 2);
		}
		return primaryVector;
	}

	template< typename T>
	void
		Tensor<T>
		::GetSignalFromTensor(SignalValueType & signal)
	{
		vnl_matrix<float> rigsInv(6, 6);
		rigsInv(0, 0) = 0.65395055F; rigsInv(0, 1) = -0.24983731F; rigsInv(0, 2) = 0.095448630F;
		rigsInv(0, 3) = 0.65395055F; rigsInv(0, 4) = -0.24983731F; rigsInv(0, 5) = 0.095448630F;

		rigsInv(1, 0) = 0.095448630F; rigsInv(1, 1) = 0.65395055F; rigsInv(1, 2) = -0.24983731F;
		rigsInv(1, 3) = 0.095448630F; rigsInv(1, 4) = 0.65395055F; rigsInv(1, 5) = -0.24983731F;

		rigsInv(2, 0) = -0.24983731F; rigsInv(2, 1) = 0.095448630F; rigsInv(2, 2) = 0.65395055F;
		rigsInv(2, 3) = -0.24983731F; rigsInv(2, 4) = 0.095448630F; rigsInv(2, 5) = 0.65395055F;

		rigsInv(3, 0) = 0.55850193F; rigsInv(3, 1) = 0.00000000F; rigsInv(3, 2) = 0.00000000F;
		rigsInv(3, 3) = -0.55850193F; rigsInv(3, 4) = 0.00000000F; rigsInv(3, 5) = 0.00000000F;

		rigsInv(4, 0) = 0.00000000F; rigsInv(4, 1) = 0.00000000F; rigsInv(4, 2) = 0.55850193F;
		rigsInv(4, 3) = 0.00000000F; rigsInv(4, 4) = 0.00000000F; rigsInv(4, 5) = -0.55850193F;

		rigsInv(5, 0) = 0.00000000F; rigsInv(5, 1) = 0.55850193F; rigsInv(5, 2) = 0.00000000F;
		rigsInv(5, 3) = 0.00000000F; rigsInv(5, 4) = -0.55850193F; rigsInv(5, 5) = -0.00000000F;

		vnl_matrix<float> rigsMatrix = vnl_matrix_inverse<float>(rigsInv);

		SignalValueType Adc;
		Adc.Fill(0.0);
		for (int row = 0; row < 6; row++)
		{
			for (int col = 0; col < 6; col++)
			{
				Adc[row] += rigsMatrix(row, col)*(*this)[col + 1];
			}
		}
		float referenceValue = (*this)[0];
		float interpolationBValue = 1000;
		signal[0] = (float)(referenceValue*exp(-interpolationBValue*Adc[0]));
		signal[1] = (float)(referenceValue*exp(-interpolationBValue*Adc[1]));
		signal[2] = (float)(referenceValue*exp(-interpolationBValue*Adc[2]));
		signal[3] = (float)(referenceValue*exp(-interpolationBValue*Adc[3]));
		signal[4] = (float)(referenceValue*exp(-interpolationBValue*Adc[4]));
		signal[5] = (float)(referenceValue*exp(-interpolationBValue*Adc[5]));
	}

	template< typename T >
	void
		Tensor< T >
		::SetTensorFromSignal(ComponentType referenceValue, SignalValueType & signal)
	{
		vnl_matrix<float> rigsInv(6, 6);
		rigsInv(0, 0) = 0.65395055F; rigsInv(0, 1) = -0.24983731F; rigsInv(0, 2) = 0.095448630F;
		rigsInv(0, 3) = 0.65395055F; rigsInv(0, 4) = -0.24983731F; rigsInv(0, 5) = 0.095448630F;

		rigsInv(1, 0) = 0.095448630F; rigsInv(1, 1) = 0.65395055F; rigsInv(1, 2) = -0.24983731F;
		rigsInv(1, 3) = 0.095448630F; rigsInv(1, 4) = 0.65395055F; rigsInv(1, 5) = -0.24983731F;

		rigsInv(2, 0) = -0.24983731F; rigsInv(2, 1) = 0.095448630F; rigsInv(2, 2) = 0.65395055F;
		rigsInv(2, 3) = -0.24983731F; rigsInv(2, 4) = 0.095448630F; rigsInv(2, 5) = 0.65395055F;

		rigsInv(3, 0) = 0.55850193F; rigsInv(3, 1) = 0.00000000F; rigsInv(3, 2) = 0.00000000F;
		rigsInv(3, 3) = -0.55850193F; rigsInv(3, 4) = 0.00000000F; rigsInv(3, 5) = 0.00000000F;

		rigsInv(4, 0) = 0.00000000F; rigsInv(4, 1) = 0.00000000F; rigsInv(4, 2) = 0.55850193F;
		rigsInv(4, 3) = 0.00000000F; rigsInv(4, 4) = 0.00000000F; rigsInv(4, 5) = -0.55850193F;

		rigsInv(5, 0) = 0.00000000F; rigsInv(5, 1) = 0.55850193F; rigsInv(5, 2) = 0.00000000F;
		rigsInv(5, 3) = 0.00000000F; rigsInv(5, 4) = -0.55850193F; rigsInv(5, 5) = -0.00000000F;

		float rec_b0 = 1.0F / referenceValue;
		float interpolationBValue = 1000;
		float negRecBValue = -1.0F / interpolationBValue;

		SignalValueType Adc;
		Adc[0] = (float)(log(signal[0] * rec_b0) * negRecBValue);
		Adc[1] = (float)(log(signal[1] * rec_b0) * negRecBValue);
		Adc[2] = (float)(log(signal[2] * rec_b0) * negRecBValue);
		Adc[3] = (float)(log(signal[3] * rec_b0) * negRecBValue);
		Adc[4] = (float)(log(signal[4] * rec_b0) * negRecBValue);
		Adc[5] = (float)(log(signal[5] * rec_b0) * negRecBValue);

		(*this)[0] = referenceValue;
		
		for (int row = 0; row < 6; row++)
		{
			float c=0.0F;
			for (int col = 0; col < 6; col++)
			{
				c += rigsInv(row, col)*Adc[col];
			}
			(*this)[row + 1] = c;
		}

	}


	/**
	* Print content to an ostream
	*/
	template< typename TComponent >
	std::ostream &
		operator<<(std::ostream & os, const Tensor< TComponent > & c)
	{
		for (unsigned int i = 0; i < c.GetNumberOfComponents(); i++)
		{
			os << c[i] << "  ";
		}
		return os;
	}

	/**
	* Read content from an istream
	*/
	template< typename T, unsigned int NDimension >
	std::istream &
		operator>>(std::istream & is, Tensor< T > & dt)
	{
		for (unsigned int i = 0; i < dt.GetNumberOfComponents(); i++)
		{
			is >> dt[i];
		}
		return is;
	}




	//template< typename TComponent = float>
	//Tensor<TComponent>
	//	::Tensor()
	//{
	//	vnl_matrix<float> rigsInv(6,6);
	//	rigsInv(0, 0) = 0.65395055F; rigsInv(0, 1) = -0.24983731F; rigsInv(0, 2) = 0.095448630F; 
	//	rigsInv(0, 3) = 0.65395055F; rigsInv(0, 4) = -0.24983731F; rigsInv(0, 5) = 0.095448630F;

	//	rigsInv(1, 0) = 0.095448630F; rigsInv(1, 1) = 0.65395055F; rigsInv(1, 2) = -0.24983731F;
	//	rigsInv(1, 3) = 0.095448630F; rigsInv(1, 4) = 0.65395055F; rigsInv(1, 5) = -0.24983731F;

	//	rigsInv(2, 0) = -0.24983731F; rigsInv(2, 1) = 0.095448630F; rigsInv(2, 2) = 0.65395055F;
	//	rigsInv(2, 3) = -0.24983731F; rigsInv(2, 4) = 0.095448630F; rigsInv(2, 5) = 0.65395055F;

	//	rigsInv(3, 0) = 0.55850193F; rigsInv(3, 1) = 0.00000000F; rigsInv(3, 2) = 0.00000000F;
	//	rigsInv(3, 3) = -0.55850193F; rigsInv(3, 4) = 0.00000000F; rigsInv(3, 5) = 0.00000000F;

	//	rigsInv(4, 0) = 0.00000000F; rigsInv(4, 1) = 0.00000000F; rigsInv(4, 2) = 0.55850193F;
	//	rigsInv(4, 3) = 0.00000000F; rigsInv(4, 4) = 0.00000000F; rigsInv(4, 5) = -0.55850193F;

	//	rigsInv(5, 0) = 0.00000000F; rigsInv(5, 1) = 0.55850193F; rigsInv(5, 2) = 0.00000000F;
	//	rigsInv(5, 3) = 0.00000000F; rigsInv(5, 4) = -0.55850193F; rigsInv(5, 5) = -0.00000000F;

	//	rigsMatrix = vnl_matrix_inverse<float>(rigsInv);
	//	rigsInvMatrix = rigsInv;
	//	//cout << "rigs Inv " << rigsInv(0,:) <<endl;
	//}

	//template< typename TComponent = float>
	//void
	//	Tensor<TComponent>
	//	::CalculateTensor(float& referenceValue, SignalValues& singnals)
	//{
	//	referenceDwiValue = referenceValue;

	//	float rec_b0 = 1.0F / referenceDwiValue;
	//	float negRecBValue = -1.0F / interpolationBValue;
	//	SignalValues singnalValue = singnals;
	//	//cout << "singal values1: " << singnalValue << endl;
	//	//cout << "reference value: " << referenceDwiValue << endl;
	//	singnalValue[0] = (float)(log(singnalValue[0] * rec_b0) * negRecBValue); 
	//	singnalValue[1] = (float)(log(singnalValue[1] * rec_b0) * negRecBValue);
	//	singnalValue[2] = (float)(log(singnalValue[2] * rec_b0) * negRecBValue);
	//	singnalValue[3] = (float)(log(singnalValue[3] * rec_b0) * negRecBValue);
	//	singnalValue[4] = (float)(log(singnalValue[4] * rec_b0) * negRecBValue);
	//	singnalValue[5] = (float)(log(singnalValue[5] * rec_b0) * negRecBValue);
	//	singnalValue[6] = (float)(log(singnalValue[6] * rec_b0) * negRecBValue);
	//	//cout << "signal value: " << singnalValue << endl;
	//	TransformVector(rigsInvMatrix, singnalValue, tensorMatrix);
	//	//cout << "tensor after : " << tensorMatrix << endl;
	//	CalculateEigenvector(tensorMatrix, eigenValues, eigenVectors);
	//	//cout << "calcluated eigenvector " << endl;
	//}

	//template< typename TComponent = float>
	//void
	//	Tensor<TComponent>
	//	::GetSignalFromTensor(vnl_vector<float>& tensorValules, float& referenceValue, vnl_vector<float>& ADC, SignalValues& singal)
	//{
	//	
	//	//vnl_matrix<float> rigsInv = vnl_matrix_inverse<float>(rigsMatrix);
	//	//cout << "rigsINv" << rigsInv << endl;
	//	//cout << "tensor before :" << tensorValules[3] << endl;
 //		vnl_vector<float> Adc = rigsMatrix*tensorValules;

	//	singal[0] = (float)(referenceValue*exp(-interpolationBValue*Adc[0]));
	//	singal[1] = (float)(referenceValue*exp(-interpolationBValue*Adc[1]));
	//	singal[2] = (float)(referenceValue*exp(-interpolationBValue*Adc[2]));
	//	singal[3] = (float)(referenceValue*exp(-interpolationBValue*Adc[3]));
	//	singal[4] = (float)(referenceValue*exp(-interpolationBValue*Adc[4])); 
	//	singal[5] = (float)(referenceValue*exp(-interpolationBValue*Adc[5]));
	//}

	//template< typename TComponent = float>
	//void
	//	Tensor<TComponent>
	//	::TransformVector(vnl_matrix<float>& HMatrix, SignalValues& singal, TensorType& tensorValules)
	//{
	//	/*vnl_matrix<float> combinationMatrix = vnl_matrix_inverse<float>(HMatrix.transpose()*HMatrix)
	//		*HMatrix.transpose();*/
	//	vnl_vector<float> adcVector(numberOfDirection);
	//	vnl_vector<float> result(numberOfDirection);
	//	for (int row = 0; row < numberOfDirection; row++)
	//	{
	//		adcVector[row] = singal[row];
	//	}
	//	//	float v = HMatrix(row, 0)* singal[0] + HMatrix(0, 1)*singal[1]
	//	//		+ HMatrix(row, 2)*singal[2] + HMatrix(row, 3)*singal[3]
	//	//		+ HMatrix(row, 4)*singal[4] + HMatrix(row, 5)*singal[5];
	//	//	if (row < 3)
	//	//		tensorValules(row, row) = v;
	//	//	else if (row < 5)
	//	//		tensorValules(0, row - 2) = v;
	//	//	else
	//	//		tensorValules(1, 2) = v;
	//	//}
	//	//tensorValules(1, 0) = tensorValules(0, 1);
	//	//tensorValules(2, 0) = tensorValules(0, 2);
	//	//tensorValules(2, 1) = tensorValules(1, 2);
	//
	//	result = HMatrix*adcVector;
	//	//cout << "tensor after: " << result[3] << endl;
	//	tensorValules(0, 0) = (double)result[0]; tensorValules(0, 1) = (double)result[3]; tensorValules(0, 2) = (double)result[4];
	//	tensorValules(1, 0) = (double)result[3]; tensorValules(1, 1) = (double)result[1]; tensorValules(1, 2) = (double)result[5];
	//	tensorValules(2, 0) = (double)result[4]; tensorValules(2, 1) = (double)result[5]; tensorValules(2, 2) = (double)result[2];
	//	//cout << "transformVector" << tensorValules << endl;

	//}

} // end namespace itk
#endif