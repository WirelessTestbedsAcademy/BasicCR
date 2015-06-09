/**
 * \file FileUtility.h
 * \version 1.0
 *
 * \section COPYRIGHT
 *
 * Copyright 2012-2013 The Iris Project Developers. See the
 * COPYRIGHT file at the top-level directory of this distribution
 * and at http://www.softwareradiosystems.com/iris/copyright.html.
 *
 * \section LICENSE
 *
 * This file is part of the Iris Project.
 *
 * Iris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Iris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * A copy of the GNU Lesser General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 * \section DESCRIPTION
 *
 * Class providing static functions for reading/writing data to/from files.
 */

#ifndef FILEUTILITY_H
#define FILEUTILITY_H

#include <fstream>
#include "tml/tml.h"

using namespace std;

/*! \class FileUtility Class
* \brief This class provides generic functionality for reading and writing signals to and from files.
*/
class FileUtility
{
public:

	/*! Function Description.
	* \brief Write arrays of complex samples to file
	* \param input An array of complex samples to be written to file
	* \param size Number of complex samples in input
	* \param filename Name of file to write data to
	* \param scalefactor Factor by which to scale the data
	*/
	static bool write_complex_float(SCplx* input, int size, string filename, float scalefactor)
	{
		string realFileName(filename);
		string imagFileName(filename);
		realFileName += "_real.bin";
		imagFileName += "_imag.bin";

		ofstream hOutFile1(realFileName.c_str(), ios::binary | ios::trunc);
		ofstream hOutFile2(imagFileName.c_str(), ios::binary | ios::trunc);

		float real = 0;
		float imag = 0;

		if(hOutFile1.is_open() && hOutFile2.is_open())
		{
			for(int i=0;i<size;i++)
			{
				real = input[i].re * scalefactor;
				imag = input[i].im * scalefactor;
				hOutFile1.write((char*)&real, sizeof(float));
				hOutFile2.write((char*)&imag, sizeof(float));
			}
			hOutFile1.close();
			hOutFile2.close();
			return true;
		}
		return false;
	};

	/*! Function Description.
	* \brief Write arrays of floats to file
	* \param input An array of floats to be written to file
	* \param size Number of floats in input
	* \param filename Name of file to write data to
	*/
	static bool write_float(float* input, int size, string filename, float scalefactor)
	{
		string fileName(filename);
		fileName += ".bin";

		ofstream hOutFile(fileName.c_str(), ios::binary | ios::trunc);
		float output = 0;

		if(hOutFile.is_open())
		{
			for(int i=0;i<size;i++)
			{
				output = input[i] * scalefactor;
				hOutFile.write((char*)&output, sizeof(float));
			}
			hOutFile.close();
			return true;
		}

		return false;
	};

	/*! Function Description.
	* \brief Write arrays of ints to file
	* \param input An array of ints to be written to file
	* \param size Number of ints in input
	* \param filename Name of file to write data to
	*/
	static bool write_int(int* input, int size, string filename)
	{
		string fileName(filename);
		fileName += ".bin";

		ofstream hOutFile(fileName.c_str(), ios::binary | ios::trunc);

		if(hOutFile.is_open())
		{
			for(int i=0;i<size;i++)
			{
				hOutFile.write((char*)&input[i], sizeof(int));
			}
			hOutFile.close();
			return true;
		}

		return false;
	};

	/*! Function Description.
	* \brief Read arrays of complex samples from file
	* \param output An array of to hold the complex samples
	* \param size Number of complex samples to read
	* \param filename Name of file to read data from
	*/
	static bool read_complex_float(SCplx* output, int size, string filename)
	{
		string realFileName(filename);
		string imagFileName(filename);
		realFileName += "_real.bin";
		imagFileName += "_imag.bin";

		ifstream hInFile1(realFileName.c_str(), ios::binary);
		ifstream hInFile2(imagFileName.c_str(), ios::binary);

		float real = 0;
		float imag = 0;

		if(hInFile1.is_open() && hInFile2.is_open())
		{
			for(int i=0;i<size;i++)
			{
				hInFile1.read((char*)&real, sizeof(float));
				hInFile2.read((char*)&imag, sizeof(float));
				output[i].re = real;
				output[i].im = imag;
			}
			hInFile1.close();
			hInFile2.close();
			return true;
		}
		return false;
	};

	/*! Function Description.
	* \brief Read arrays of floats from file
	* \param output An array of to hold the floats
	* \param size Number of floats to read
	* \param filename Name of file to read data from
	*/
	static bool read_float(float* output, int size, string filename)
	{
		string fileName(filename);
		fileName += ".bin";

		ifstream hInFile(fileName.c_str(), ios::binary);

		if(hInFile.is_open()){
			hInFile.read((char*)output, sizeof(float)*size);
			if(hInFile.good()){
				hInFile.close();
				return true;
			}
		}
		return false;
	};
};

#endif

