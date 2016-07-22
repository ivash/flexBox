#ifndef flexGradientOperator_H
#define flexGradientOperator_H

#include "vector"
#include "flexLinearOperator.h"

template < typename T, typename Tvector >
class flexGradientOperator : public flexLinearOperator<T, Tvector>
{
private:
#if __CUDACC__
	thrust::device_vector<int> inputDimension;
#else
	std::vector<int> inputDimension;
#endif
	
	int* inputDimensionPtr;
	int gradDirection;
	int type;
	bool transposed;
	int numberDimensions;

public:

	//type:
	// 0 = forward
	// 1 = backward
	flexGradientOperator(std::vector<int> _inputDimension, int _gradDirection, int _type) : flexLinearOperator<T, Tvector>(vectorProduct(_inputDimension), vectorProduct(_inputDimension),gradientOp)
	{
		
		this->gradDirection = _gradDirection;
		this->type = _type;
		this->transposed = false;
		this->numberDimensions = _inputDimension.size();

#if __CUDACC__
		this->inputDimension.resize(_inputDimension.size());
		thrust::copy(_inputDimension.begin(), _inputDimension.end(), this->inputDimension.begin());
		inputDimensionPtr = thrust::raw_pointer_cast(this->inputDimension.data());
#else
		this->inputDimension = _inputDimension;
		this->inputDimensionPtr = this->inputDimension.data();
#endif
	};

	flexGradientOperator<T, Tvector>* copy()
	{
		flexGradientOperator<T, Tvector>* A = new flexGradientOperator<T, Tvector>(this->inputDimension, this->gradDirection, this->type);

		return A;
	}

	//apply linear operator to vector
	void times(const Tvector &input, Tvector &output)
	{

	}



	void dxp2d(const Tvector &input, Tvector &output, sign s)
	{
		#pragma omp parallel for
		for (int j = 0; j < this->inputDimension[1]; ++j)
		{
			for (int i = 0; i < this->inputDimension[0] - 1; ++i)
			{
				const int tmpIndex = this->index2DtoLinear(i, j);

				switch (s)
				{
					case PLUS:
					{
						output[tmpIndex] += input[tmpIndex + 1] - input[tmpIndex];
						break;
					}
					case MINUS:
					{
						output[tmpIndex] -= input[tmpIndex + 1] - input[tmpIndex];
						break;
					}
					case EQUALS:
					{
						output[tmpIndex] = input[tmpIndex + 1] - input[tmpIndex];
						break;
					}
				}
			}
		}
	}

	void dyp2d(const Tvector &input, Tvector &output, sign s)
	{
		#pragma omp parallel for
		for (int j = 0; j < this->inputDimension[1] - 1; ++j)
		{
			for (int i = 0; i < this->inputDimension[0]; ++i)
			{
				const int tmpIndex = this->index2DtoLinear(i, j);

				switch (s)
				{
					case PLUS:
					{
						output[tmpIndex] += input[tmpIndex + this->inputDimension[0]] - input[tmpIndex];
						break;
					}
					case MINUS:
					{
						output[tmpIndex] -= input[tmpIndex + this->inputDimension[0]] - input[tmpIndex];
						break;
					}
					case EQUALS:
					{
						output[tmpIndex] = input[tmpIndex + this->inputDimension[0]] - input[tmpIndex];
						break;
					}
				}
			}
		}
	}

	void dxp2dTransposed(const Tvector &input, Tvector &output, sign s)
	{
		#pragma omp parallel for
		for (int j = 0; j < this->inputDimension[1]; ++j)
		{
			for (int i = 1; i < this->inputDimension[0]-1; ++i)
			{
				int tmpIndex = this->index2DtoLinear(i, j);

				switch (s)
				{
					case PLUS:
					{
						output[tmpIndex] += -(input[tmpIndex] - input[tmpIndex - 1]);
						break;
					}
					case MINUS:
					{
						output[tmpIndex] -= -(input[tmpIndex] - input[tmpIndex - 1]);
						break;
					}
					case EQUALS:
					{
						output[tmpIndex] = -(input[tmpIndex] - input[tmpIndex - 1]);
						break;
					}
				}
			}
		}

		for (int j = 0; j < this->inputDimension[1]; ++j)
		{
			switch (s)
			{
				case PLUS:
				{
					output[this->index2DtoLinear(0, j)] += -input[this->index2DtoLinear(0, j)];
					output[this->index2DtoLinear(this->inputDimension[0] - 1, j)] += input[this->index2DtoLinear(this->inputDimension[0] - 2, j)];
					break;
				}
				case MINUS:
				{
					output[this->index2DtoLinear(0, j)] -= -input[this->index2DtoLinear(0, j)];
					output[this->index2DtoLinear(this->inputDimension[0] - 1, j)] -= input[this->index2DtoLinear(this->inputDimension[0] - 2, j)];
					break;
				}
				case EQUALS:
				{
					output[this->index2DtoLinear(0, j)] = -input[this->index2DtoLinear(0, j)];
					output[this->index2DtoLinear(this->inputDimension[0] - 1, j)] = input[this->index2DtoLinear(this->inputDimension[0] - 2, j)];
					break;
				}
			}
		}
	}

	void dyp2dTransposed(const Tvector &input, Tvector &output, sign s)
	{
		#pragma omp parallel for
		for (int j = 1; j < this->inputDimension[1] - 1; ++j)
		{
			for (int i = 0; i < this->inputDimension[0]; ++i)
			{
				int tmpIndex = this->index2DtoLinear(i, j);

				switch (s)
				{
					case PLUS:
					{
						output[tmpIndex] += -(input[tmpIndex] - input[tmpIndex - this->inputDimension[0]]);
						break;
					}
					case MINUS:
					{
						output[tmpIndex] -= -(input[tmpIndex] - input[tmpIndex - this->inputDimension[0]]);
						break;
					}
					case EQUALS:
					{
						output[tmpIndex] = -(input[tmpIndex] - input[tmpIndex - this->inputDimension[0]]);
						break;
					}
				}
			}
		}

		for (int i = 0; i < this->inputDimension[0]; ++i)
		{
			switch (s)
			{
			case PLUS:
			{
				output[this->index2DtoLinear(i, 0)] += -input[this->index2DtoLinear(i, 0)];
				output[this->index2DtoLinear(i, this->inputDimension[1] - 1)] += input[this->index2DtoLinear(i, this->inputDimension[1] - 2)];
				break;
			}
			case MINUS:
			{
				output[this->index2DtoLinear(i, 0)] -= -input[this->index2DtoLinear(i, 0)];
				output[this->index2DtoLinear(i, this->inputDimension[1] - 1)] -= input[this->index2DtoLinear(i, this->inputDimension[1] - 2)];
				break;
			}
			case EQUALS:
			{
				output[this->index2DtoLinear(i, 0)] = -input[this->index2DtoLinear(i, 0)];
				output[this->index2DtoLinear(i, this->inputDimension[1] - 1)] = input[this->index2DtoLinear(i, this->inputDimension[1] - 2)];
				break;
			}
			}
		}
	}


	void timesPlus(const Tvector &input, Tvector &output)
	{
		if (this->inputDimension.size() == 2)
		{
			if (this->gradDirection == 0)
			{
				if (this->transposed == false)
				{
                    this->dxp2d(input,output,PLUS);
				}
				else
				{
                    this->dxp2dTransposed(input, output, PLUS);
				}
			}
			else if (this->gradDirection == 1)
			{
				if (this->transposed == false)
				{
					this->dyp2d(input, output, PLUS);
				}
				else
				{
					this->dyp2dTransposed(input, output, PLUS);
				}
			}
		}
		else if (this->inputDimension.size() == 3)
		{
			//todo
		}
	}

	void timesMinus(const Tvector &input, Tvector &output)
	{
		if (this->inputDimension.size() == 2)
		{
			if (this->gradDirection == 0)
			{
				if (this->transposed == false)
				{
					this->dxp2d(input, output, MINUS);
				}
				else
				{
					this->dxp2dTransposed(input, output, MINUS);
				}
			}
			else if (this->gradDirection == 1)
			{
				if (this->transposed == false)
				{
					this->dyp2d(input, output, MINUS);
				}
				else
				{
					this->dyp2dTransposed(input, output, MINUS);
				}
			}
		}
		else if (this->inputDimension.size() == 3)
		{
			//todo
		}
	}

	T getMaxRowSumAbs() const
	{
		//row sum of absolute values is at maximum 2
		return static_cast<T>(2);
	}

	T getMaxRowSumAbs()
	{
		//row sum of absolute values is at maximum 2
		return static_cast<T>(2);
	}

	std::vector<T> getAbsRowSum()
	{
		std::vector<T> result(this->getNumRows(),(T)2);

		return result;
	}

	//transposing the identity does nothing
	void transpose()
	{
		//swith type from forward to backward and vice versa
		if (this->transposed == false)
		{
			this->transposed = true;
		}
		else
		{
			this->transposed = false;
		}
	}

	int index2DtoLinear(int i, int j)
	{
		return (i + j*this->inputDimension[0]);
	}

	T timesElement_xp(int index, const T* input, int i)
	{
		if (i < this->inputDimensionPtr[0] - 1)
		{
			return input[index + 1] - input[index];
		}
		else
		{
			return (T)0;
		}
	}

	T timesElement_xm(int index, const T* input, int i)
	{
		if (i > 0 && i < this->inputDimensionPtr[0] - 1)
		{
			return -(input[index] - input[index - 1]);
		}
		else if (i == 0)
		{
			return -input[index];
		}
		else
		{
			return input[index - 1];
		}
	}

	T timesElement_yp(int index, const T* input, int j)
	{
		if (j < this->inputDimensionPtr[1] - 1)
		{
			return input[index + this->inputDimensionPtr[0]] - input[index];
		}
		else
		{
			return (T)0;
		}
	}

	T timesElement_ym(int index, const T* input, int j)
	{
		if (j > 0 && j < this->inputDimensionPtr[1] - 1)
		{
			return -(input[index] - input[index - this->inputDimensionPtr[0]]);
		}
		else if (j == 0)
		{
			return -input[index];
		}
		else
		{
			return input[index - this->inputDimensionPtr[0]];
		}
	}

	T timesElement_zp(int index, const T* input, int k)
	{
		if (k < this->inputDimensionPtr[2] - 1)
		{
			return input[index + this->inputDimensionPtr[0] * this->inputDimensionPtr[1]] - input[index];
		}
		else
		{
			return (T)0;
		}
	}

	T timesElement_zm(int index, const T* input, int k)
	{
		if (k > 0 && k < this->inputDimensionPtr[2] - 1)
		{
			return -(input[index] - input[index - this->inputDimensionPtr[0] * this->inputDimensionPtr[1]]);
		}
		else if (k == 0)
		{
			return -input[index];
		}
		else
		{
			return input[index - this->inputDimensionPtr[0] * this->inputDimensionPtr[1]];
		}
	}

	int indexI(int index)
	{
		return index % this->inputDimensionPtr[0];
	}

	int indexJ(int index)
	{
		return (index / this->inputDimensionPtr[0]) % this->inputDimensionPtr[1];
	}

	int indexK(int index)
	{
		return ((index / this->inputDimensionPtr[0]) / this->inputDimensionPtr[1]) % this->inputDimensionPtr[2];
	}

	T timesElement(const int index, const T* input)
	{
		switch (this->gradDirection)
		{
		case 0:
		{
			switch (this->transposed)
			{
			case false:
			{
				return timesElement_xp(index, input, indexI(index));
				break;
			}
			case true:
			{
				return timesElement_xm(index, input, indexI(index));
				break;
			}
			}
			break;
		}
		case 1:
		{
			switch (this->transposed)
			{
			case false:
			{
				return timesElement_yp(index, input, indexJ(index));
				break;
			}
			case true:
			{
				return timesElement_ym(index, input, indexJ(index));
				break;
			}
			}
			break;
		}
		case 2:
		{
			switch (this->transposed)
			{
			case false:
			{
				return timesElement_zp(index, input, indexK(index));
				break;
			}
			case true:
			{
				return timesElement_zm(index, input, indexK(index));
				break;
			}
			}
			break;
		}
		}
	}


    
	#if __CUDACC__
	__device__ T timesElement_xp(int index, const T* input, int i)
	{
		if (i < this->inputDimensionPtr[0] - 1)
		{
			return input[index + 1] - input[index];
		}
		else
		{
			return (T)0;
		}
	}

	__device__ T timesElement_xm(int index, const T* input, int i)
	{
		if (i > 0 && i < this->inputDimensionPtr[0] - 1)
		{
			return -(input[index] - input[index - 1]);
		}
		else if (i == 0)
		{
			return -input[index];
		}
		else
		{
			return input[index - 1];
		}
	}

	__device__ T timesElement_yp(int index, const T* input, int j)
	{
		if (j < this->inputDimensionPtr[1] - 1)
		{
			return input[index + this->inputDimensionPtr[0]] - input[index];
		}
		else
		{
			return (T)0;
		}
	}

	__device__ T timesElement_ym(int index, const T* input, int j)
	{
		if (j > 0 && j < this->inputDimensionPtr[1] - 1)
		{
			return -(input[index] - input[index - this->inputDimensionPtr[0]]);
		}
		else if (j == 0)
		{
			return -input[index];
		}
		else
		{
			return input[index - this->inputDimensionPtr[0]];
		}
	}

	__device__ T timesElement_zp(int index, const T* input, int k)
	{
		if (k < this->inputDimensionPtr[2] - 1)
		{
			return input[index + this->inputDimensionPtr[0] * this->inputDimensionPtr[1]] - input[index];
		}
		else
		{
			return (T)0;
		}
	}

	__device__ T timesElement_zm(int index, const T* input, int k)
	{
		if (k > 0 && k < this->inputDimensionPtr[2] - 1)
		{
			return -(input[index] - input[index - this->inputDimensionPtr[0]*this->inputDimensionPtr[1]]);
		}
		else if (k == 0)
		{
			return -input[index];
		}
		else
		{
			return input[index - this->inputDimensionPtr[0] * this->inputDimensionPtr[1]];
		}
	}

	__inline__ __device__ int indexI(int index)
	{
		return index % this->inputDimensionPtr[0];
	}

	__inline__ __device__ int indexJ(int index)
	{
		return (index / this->inputDimensionPtr[0]) % this->inputDimensionPtr[1];
	}

	__inline__ __device__ int indexK(int index)
	{
		return ((index / this->inputDimensionPtr[0]) / this->inputDimensionPtr[1]) % this->inputDimensionPtr[2];
	}

	__device__ T timesElementCUDA(const int index, const T* input)
	{
		switch (this->gradDirection)
		{
			case 0:
			{
				switch (this->transposed)
				{
					case false:
					{
						return timesElement_xp(index, input, indexI(index));
						break;
					}
					case true:
					{
						return timesElement_xm(index, input, indexI(index));
						break;
					}
				}
				break;
			}
			case 1:
			{
				switch (this->transposed)
				{
					case false:
					{
						return timesElement_yp(index, input, indexJ(index));
						break;
					}
					case true:
					{
						return timesElement_ym(index, input, indexJ(index));
						break;
					}
				}
				break;
			}
			case 2:
			{
				switch (this->transposed)
				{
					case false:
					{
						return timesElement_zp(index, input, indexK(index));
						break;
					}
					case true:
					{
						return timesElement_zm(index, input, indexK(index));
						break;
					}
				}
				break;
			}
		}
	}

	__device__ T getRowsumElementCUDA(int index)
	{
		int i, j, k;

		i = index % this->inputDimensionPtr[0];

		if (this->numberDimensions > 1)
		{
			int indexTmp = index / this->inputDimensionPtr[0];

			j = indexTmp % this->inputDimensionPtr[1];

			if (this->numberDimensions > 2)
			{
				indexTmp = indexTmp / this->inputDimensionPtr[1];

				k = indexTmp % this->inputDimensionPtr[2];
			}
		}

		switch (this->gradDirection)
		{
			case 0:
			{
				switch (this->transposed)
				{
					case false:
					{
						if (i < this->inputDimensionPtr[0] - 1)
						{
							return (T)2;
						}
						else
						{
							return (T)0;
						}
						break;
					}
					case true:
					{
						if (i > 0 && i < this->inputDimensionPtr[0] - 1)
						{
							return (T)2;
						}
						else
						{
							return (T)1;
						}
						break;
					}
				}
				break;
			}
			case 1:
			{
				switch (this->transposed)
				{
					case false:
					{
						if (j < this->inputDimensionPtr[1] - 1)
						{
							return (T)2;
						}
						else
						{
							return (T)0;
						}
						break;
					}
					case true:
					{
						if (j > 0 && j < this->inputDimensionPtr[1] - 1)
						{
							return (T)2;
						}
						else
						{
							return (T)1;
						}
						break;
					}
				}
				break;
			}
			case 2:
			{
				switch (this->transposed)
				{
					case false:
					{
						if (k < this->inputDimensionPtr[2] - 1)
						{
							return (T)2;
						}
						else
						{
							return (T)0;
						}
						break;
					}
					case true:
					{
						if (k > 0 && k < this->inputDimensionPtr[2] - 1)
						{
							return (T)2;
						}
						else
						{
							return (T)1;
						}
						break;
					}
				}
				break;
			}
		}

		return (T)2;
	}
	#endif
};

#endif
