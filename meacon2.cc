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
  if (argc != 6)
  {
    std::cerr << "Usage: " << argv[0]
      << " input_file sample_rate start_time max_delay max_power\n"
      << "\tEx: " << argv[0] << " gpssim.bin 2600000 180 0.5 1.2"
      << std::endl;

    exit(1);
  }

  std::string in_name(argv[1]);
  std::string out_name = "meacon_" + in_name;

  // Sample rate of the binary
  //   Note: we assume the sample is complex two byte
  int sample_rate = std::atoi(argv[2]);

  // Start time in number of samples, multiplied by two to account for I and Q
  //   values associated with each sample
  int start_time = std::stoi(argv[3]) * sample_rate * 2;

  double delay = 0;
  // Maxiumum delay in number of samples
  int max_delay = std::atoi(argv[4]) * sample_rate * 2;

  double power = 0;
  double max_power = std::stof(argv[5]);

  // Buffer to hold the samples while they are processed
  std::deque<int16_t> sample_buffer;

  // Open the simulated Binary file
  std::ifstream input(in_name, std::ios::binary);

  // Open the output Binary file
  std::ofstream output(out_name, std::ios::binary);

  // Read the binary samples until done
  unsigned long long int sample_index = 0;

  int16_t sample = 0x00;

  //const char* working_char = "|/-\\";
  //std::cout << "Working \\\b";

  while (input.good())
  {
    //if (sample_index % 10000000 == 0)
    //  std::cout << working_char[(sample_index >> 7) % 4] << "\b" << std::flush;

    // Read a sample (I or Q)
    input.read(reinterpret_cast<char*>(&sample), sizeof sample);

    sample_index++;

    sample_buffer.push_back(sample);

    // If sample_index is greater than the start time we need to start adding
    //   previously recorded samples, and increasing the power
    // If the power is equal to the max power we need to start adding delay
    // If delay is equal to max delay, just continue at max_power and max_delay

    if (delay == max_delay) // Full on attack
      sample_buffer.back() += (power * sample_buffer[1]);
    else if (power == max_power) // Power at max, max delay not yet reached
    {
      sample_buffer.back() = sample_buffer.back()
        + (power * sample_buffer[max_delay - delay]);

      // Increase delay linearly a 0.2 s/min
      delay += 1 / (2 * 60.0 * 5);

      if (delay >= max_delay)
      {
        //std::cout << "\b\b\b\b\b\b\b\bFull on attack starts at "
        std::cout << "Full on attack starts at "
          << (sample_index / 2.0) / sample_rate << "s" << std::endl;
          //<< "Working -\b" << std::flush;

        // Catch any portential weirdness
        delay = max_delay;
      }
    }
    else if (sample_index > start_time) // Attack started, max power not reached
    {
      sample_buffer.back() *= (1 + power);

      // Increase delay
      //power = std::log((sample_index - start_time) / 100000.0 + 1) / 10.0;
      // Reach maximum power in 10 minutes
      power += max_power / (sample_rate * 2 * 60.0 * 10);

      // Cap max power
      if (power >= max_power)
      {
        //std::cout << "\b\b\b\b\b\b\b\bMaximum power reached at "
        std::cout << "Maximum power reached at "
          << (sample_index / 2.0) / sample_rate << "s, adding delay"
          << std::endl;// << "Working -\b" << std::flush;

        power = max_power;
      }
    }
    else if (sample_index == start_time)
      //std::cout << "\b\b\b\b\b\b\b\bBeginning power ramp up at "
      std::cout << "Beginning power ramp up at "
        << (sample_index / 2.0) / sample_rate << "s" << std::endl;
        //<< "Working -\b" << std::flush;
    else if (sample_index < start_time)
      ; // Don't do anything
    else
      std::cerr << "Error, didn't fall into anything" << std::endl;

    // If buffer is max length, pop and write first sample
    if (sample_buffer.size() > max_delay)
    {
      output.write(reinterpret_cast<char*>(&sample_buffer.front()), 2);

      sample_buffer.pop_front();
    }
  }

  //std::cout << "\b\b\b\b\b\b\b\bFinished processing "
  std::cout << "Finished processing "
    << sample_index / 2 << " samples" << std::endl;

  if (delay < max_delay)
    std::cout << "(Only reached " << (delay / 2.0) / sample_rate << "s delay)"
      << std::endl;

  if (output.is_open())
    output.close();
  else
    std::cerr << "Some serious error, outfile not opened" << std::endl;

  if (input.is_open())
    input.close();
  else
    std::cerr << "Some serious error, infile not opened" << std::endl;
}
