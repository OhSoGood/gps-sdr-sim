// Copyright (C) 2018 Colton Riedel
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see https://www.gnu.org/licenses/
//
// If you are interested in obtaining a copy of this program under a
// different license, or have other questions or comments, contact me at
//
//   coltonriedel at protonmail dot ch

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " input_file " << std::endl;

    exit(1);
  }

  std::string in_name(argv[1]);
  std::string out_name = "demo_" + in_name;

  std::srand(std::time(nullptr));

  // Sample rate of the binary
  //   Note: we assume the sample is complex two byte
  int sample_rate = 2600000; //std::atoi(argv[2]);

  // Open the simulated Binary file
  std::ifstream input(in_name, std::ios::binary);

  // Open the output Binary file
  std::ofstream output(out_name, std::ios::binary);

  // Read the binary samples until done
  unsigned long long int sample_index = 0;

  int16_t sample = 0x00;

  double offset = 0;

  while (input.good())
  {
    input.read(reinterpret_cast<char*>(&sample), sizeof sample);

    sample_index++;

    // Write the first 4 minutes out directly
    if (sample_index < (2600000 * 2 * 4 * 60))
      output.write(reinterpret_cast<char*>(&sample), 2);
    // Write minutes 4-5 with and increasing jam
    else if (sample_index < (2600000 * 2 * 5 * 60))
    {
      offset += 3000.0 / (2600000 * 2 * 60);

      sample += offset + (std::rand() % 500);

      output.write(reinterpret_cast<char*>(&sample), 2);
    }
    // Maintain jamming level
    else if (sample_index < (2600000 * 2 * 6 * 60))
    {
      sample += offset + (std::rand() % 500);

      output.write(reinterpret_cast<char*>(&sample), 2);
    }
    // Drop 0.2583 of a second (15.5 cycles)
    else if (sample_index == (2600000 * 2 * 6 * 60))
      std::cout << "Offset value peaked at " << offset << std::endl;
    else if (sample_index < (2600000 * 2 * 6.004305 * 60));
      // Do nothing
    // Wind down jamming power
    else if (sample_index < (2600000 * 2 * 7.004305 * 60))
    {
      offset -= 3000.0 / (2600000 * 2);

      sample += offset + (std::rand() % 500);

      output.write(reinterpret_cast<char*>(&sample), 2);
    }
    else // Write out directly (at 0.2583s total offset)
      output.write(reinterpret_cast<char*>(&sample), 2);
  }

  std::cout << "Finished processing "
    << sample_index / 2 << " samples" << std::endl;

  if (output.is_open())
    output.close();
  else
    std::cerr << "Some serious error, outfile not opened" << std::endl;

  if (input.is_open())
    input.close();
  else
    std::cerr << "Some serious error, infile not opened" << std::endl;
}
