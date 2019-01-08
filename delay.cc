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

#include <cmath>
#include <deque>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
  if (argc != 8)
  {
    std::cerr << "Usage: " << argv[0]
      << " input_file sample_rate start_time max_time time_onset max_power "
      << "power_onset\n\tEx: " << argv[0]
      << " gpssim.bin 2600000 180 0.5 10 1.2 10"
      << "\n\t\t   max_time: maximum time offset (seconds)"
      << "\n\t\t time_onset: time (minutes) to reach maximum time offset"
      << "\n\t\t  max_power: maximum power ratio to clean signal"
      << "\n\t\tpower_onset: time (minutes) to reach maximum power"
      << std::endl;

    exit(1);
  }

  std::string in_name(argv[1]);
  std::string out_name = "spoof_" + in_name;

  // Sample rate of the binary
  //   Note: we assume the sample is complex two byte
  int sample_rate = std::atoi(argv[2]);

  // Start time in number of samples, multiplied by two to account for I and Q
  //   values associated with each sample
  int start_time = std::stoi(argv[3]) * sample_rate * 2;

  double time_offset = 0;
  int max_time_offset = std::atof(argv[4]) * sample_rate * 2;
  double time_onset = std::atof(argv[5]);

  double power = 0;
  double max_power = std::stof(argv[6]);
  double power_onset = std::atof(argv[7]);

  // Buffer to hold the samples while they are processed
  std::deque<int16_t> clean_buffer;

  // Open the simulated Binary file
  std::ifstream input(in_name, std::ios::binary);

  // Open the output Binary file
  std::ofstream output(out_name, std::ios::binary);

  // Read the binary samples until done
  unsigned long long int sample_index = 0;

  int16_t sample = 0x00;

  while (input.good())
  {
    input.read(reinterpret_cast<char*>(&sample), sizeof sample);

    sample_index++;

    if (power == max_power)
    {
      clean_buffer.push_back(sample);

      sample += (power * clean_buffer.front());

      if (time_offset < max_time_offset)
      {
        time_offset += 1 / (2 * 60.0 * time_onset);

        if (time_offset >= max_time_offset)
          std::cout << "Full on attack starts at "
            << (sample_index / 2.0) / sample_rate << "s" << std::endl;
      }
      else if (time_offset > max_time_offset) // Catch any portential weirdness
        time_offset = max_time_offset;
    }
    else if (sample_index > start_time)
    {
      sample *= (1 + power);

      power += max_power / (sample_rate * 2 * 60.0 * power_onset);

      if (power >= max_power)
      {
        std::cout << "Maximum power reached at "
          << (sample_index / 2.0) / sample_rate << "s, adding time_offset"
          << std::endl;

        power = max_power;
      }
    }
    else if (sample_index == start_time)
      std::cout << "Beginning power ramp up at "
        << (sample_index / 2.0) / sample_rate << "s" << std::endl;
    else if (sample_index < start_time)
      ; // Don't do anything
    else
      std::cerr << "Error, didn't fall into anything" << std::endl;

    if (clean_buffer.size() > time_offset)
      clean_buffer.pop_front();

    output.write(reinterpret_cast<char*>(&sample), 2);
  }

  std::cout << "Finished processing "
    << sample_index / 2 << " samples" << std::endl;

  if (time_offset < max_time_offset)
    std::cout << "(Only reached " << (time_offset / 2.0) / sample_rate
      << "s time offset)" << std::endl;

  if (output.is_open())
    output.close();
  else
    std::cerr << "Some serious error, outfile not opened" << std::endl;

  if (input.is_open())
    input.close();
  else
    std::cerr << "Some serious error, infile not opened" << std::endl;
}
